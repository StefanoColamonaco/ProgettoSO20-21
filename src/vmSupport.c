#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

#include "pandos_types.h"
#include "pandos_const.h"
#include "init.h"
#include "asl.h"
#include "pcb.h"
#include "scheduler.h"
#include "vmSupport.h"
#include "interrupts.h"
#include "exceptions.c"
#include "sysSupport.h"

#define READBLK 2 
#define WRITEBLK 3


swap_t swapTable[POOLSIZE];
unsigned int swapFloor;

int swapSem = 1;
semd_t swapSemStruct;


unsigned int freeAsidBitmap = 0b11111111; 
static int frameIndexToReplace = 0;


static int getVPNAddress(int index);

//static void initSwapSem();

static void initSwapTable();

static inline int frameIsOccupied(swap_t* frame);

static int getFrameIndexToReplace();

static void markPTEntryNotValid(pteEntry_t* entry);

static void markTLBEntryNotValid(pteEntry_t* entry);

static void writeFromPoolToDev(swap_t* frame, int frameIndex);

static void writeFromDevToPool(support_t* supp, int missingPageNum, int frameIndex);

static void updateSwapTableEntry(swap_t *poolFrame, int asid, pteEntry_t* pageToAdd);

static void markEntryPresentAtIndex(pteEntry_t *missingPage, int newIndex);

static void markValidAndUpdateTLB(pteEntry_t *page, int frameIndex);

static void updateEntryInTLB(pteEntry_t* entry);

static void handleTLBInvalid();

static void markInvalidAndSave(swap_t* frameToReplace, int frameIndexToReplace);

int PTEentryIsValid(pteEntry_t *entry);


void initUprocPageTable(pcb_t *uproc) {
    int asid = uproc->p_supportStruct->sup_asid;
    int vpn;
    for (int i = 0; i < USERPGTBLSIZE; i++) {
        vpn = getVPNAddress(i);
        unsigned int entryHI = (vpn << VPNSHIFT) + (asid << ASIDSHIFT);
        unsigned int entryLO = DIRTYON;
        uproc->p_supportStruct->sup_privatePgTbl[i].pte_entryHI = entryHI;
        uproc->p_supportStruct->sup_privatePgTbl[i].pte_entryLO = entryLO;
    }
}


void initSwapStructs() {
    initSwapTable();
    swapSem = 1;
}


// static void initSwapSem() {
//     swapSem = 1;
//     swapSemStruct.s_semAdd = &swapSem;
//     swapSemStruct.s_procQ = mkEmptyProcQ();
//     swapSemStruct.s_next = NULL;
// }


static void initSwapTable() {
    for (int i = 0; i < POOLSIZE; i++) {
        swapTable->sw_asid = -1;
    }
}


/*check bitmap for free Asid. Returns -1 if none is free*/
int getFreeAsid() {
    for (int i = 1; i <= 8; i++) {
        if ( ((freeAsidBitmap >> i) & 1U) == ON )
            return i;
    }
    return -1;
}


int getVPNAddress(int index) {
    if(index < 0 || index > 31) {
        //TODO handle exception
    }
    if (index == 31) {
        return 0xBFFFF; 
    }
    return 0x80000 + index;
}



/*pager*/
void handlePageFault() {
    support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0 ,0);
    unsigned int cause = supp->sup_exceptState[0].cause;
    if (cause == EXC_MOD) {     //trying to write on read-only
        //treat as program trap
    } else {                    //page fault on load or store. at this point the TLB has already been refilled
        SYSCALL(PASSEREN, (int)&swapSem, 0, 0);
        handleTLBInvalid(supp);
        SYSCALL(VERHOGEN, (int)&swapSem, 0, 0);    
        contextSwitch(currentProcess);   
    }
}


static void handleTLBInvalid(support_t *supp) {
    int* missingPageNumber = getMissingPageNumber();
    int frameIndexToReplace = getFrameIndexToReplace();
    pteEntry_t *missingPage = &supp->sup_privatePgTbl[*missingPageNumber];
    swap_t *frameToReplace = &swapTable[frameIndexToReplace];

    if(frameIsOccupied(frameToReplace)) {
        markInvalidAndSave(frameToReplace, frameIndexToReplace);
    }

    writeFromDevToPool(supp, *missingPageNumber, frameIndexToReplace);
    updateSwapTableEntry(frameToReplace, supp->sup_asid, missingPage);
    
    markValidAndUpdateTLB(missingPage, frameIndexToReplace);
}



static void markInvalidAndSave(swap_t *frameToReplace, int frameIndexToReplace) {
    unsigned int oldStatus = getSTATUS();
    setSTATUS(oldStatus | DISABLEINTS);
    markPTEntryNotValid(frameToReplace->sw_pte);
    markTLBEntryNotValid(frameToReplace->sw_pte);
    setSTATUS(oldStatus);
    writeFromPoolToDev(frameToReplace, frameIndexToReplace);
}


static void markValidAndUpdateTLB(pteEntry_t *page, int frameIndex) {
    unsigned int oldStatus = getSTATUS();
    setSTATUS(oldStatus | DISABLEINTS);     //TODO possibly wrong. check correct mask
    markEntryPresentAtIndex(page, frameIndex);
    updateEntryInTLB(page);
    setSTATUS(oldStatus);
}



static int getFrameIndexToReplace() {
    for (int i = 0; i < POOLSIZE; i++) {
        if (swapTable[i].sw_asid == -1)
            return i;
    }
    int result = frameIndexToReplace;
    frameIndexToReplace = (frameIndexToReplace + 1)%POOLSIZE;
    return result;
}


static inline int frameIsOccupied(swap_t* frame) {
    return (frame->sw_asid != -1);
}


static inline void markPTEntryNotValid(pteEntry_t* entry) {
    entry->pte_entryLO &= ~VALIDON;
}


static void markTLBEntryNotValid(pteEntry_t* entry) {
    setENTRYHI(entry->pte_entryHI);
    TLBP();
    unsigned int indexReg = getINDEX();
    if (indexReg >> 31 == OFF) {  //check p bit for valid index
        //unsigned int index = (indexReg & 0xff00) >> 8;
        TLBR();
        unsigned int entryLO = getENTRYLO();
        entryLO &= ~VALIDON;
        setENTRYLO(entryLO);
        TLBWI();
    }
}


static inline int frameAddrInPool(int frameIndex) {
    return swapFloor + frameIndex * 4096;
}


static void writeFromPoolToDev(swap_t* frame, int frameIndex) {
    int devNo = frame->sw_asid - 1;
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(IL_FLASH, devNo);
    int blockNum = frame->sw_pageNo;
    dev->dtp.data0 = ENTRYLO_GET_PFN(frame->sw_pte->pte_entryLO);   //this entry is marked invalid but the pfn is still correct due to mutex
    //dev->dtp.data0 = frameAddrInPool(frameIndex);
    dev->dtp.command = blockNum << 8 | WRITEBLK;
    SYSCALL(IOWAIT, deviceSemaphores[getSemIndex(IL_FLASH, devNo, 0)], 0 ,0);
}


static void writeFromDevToPool(support_t* supp, int missingPageNum, int frameIndex) {
    pteEntry_t *missingPage = &supp->sup_privatePgTbl[missingPageNum];                             //unused missingPage 
    int devNo = supp->sup_asid - 1;
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(IL_FLASH, devNo);
    int blockNum = missingPageNum;
    //dev->dtp.data0 = ENTRYLO_GET_PFN(missingPage->pte_entryLO);
    dev->dtp.data0 = frameAddrInPool(frameIndex);
    dev->dtp.command = blockNum << 8 | READBLK;
    SYSCALL(IOWAIT, deviceSemaphores[getSemIndex(IL_FLASH, devNo, 0)], 0 ,0);
}


static void updateSwapTableEntry(swap_t *poolFrame, int asid, pteEntry_t* pageToAdd) {
    poolFrame->sw_asid = asid;
    poolFrame->sw_pte = pageToAdd;
    poolFrame->sw_pageNo = (pageToAdd->pte_entryHI & GETPAGENO) >> 12;
}


static void markEntryPresentAtIndex(pteEntry_t *missingPage, int newIndex) {
    missingPage->pte_entryLO |= VALIDON;        //TODO check
    missingPage->pte_entryLO &= ~ENTRYLO_PFN_MASK | newIndex << ENTRYLO_PFN_BIT;
}



void updateEntryInTLB(pteEntry_t* entry) {
    setENTRYHI(entry->pte_entryHI);
    TLBP();
    unsigned int indexReg = getINDEX();
    if (indexReg >> 31 == OFF) {  //check p bit for valid index
        TLBR();
        setENTRYLO(entry->pte_entryLO);
        TLBWI();
    }
}


void removePagesFromTLB(int asid) {
    support_t* supp = (support_t*)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    for (int i = 0; i < MAXPAGES; i++) {
        if (PTEentryIsValid(&supp->sup_privatePgTbl[i])) {
            markTLBEntryNotValid(&supp->sup_privatePgTbl[i]);
        }
    }
}



int PTEentryIsValid(pteEntry_t *entry) {
    return (entry->pte_entryLO & ENTRYLO_VALID) >> ENTRYLO_VALID_BIT;
}



void markReadOnlyPages(support_t *supp) {
    unsigned int *text_mem_start = (unsigned int *)0x0008;    
    unsigned int *text_mem_size = (unsigned int *)0x000C
;

    for (int i = 0; i < MAXPAGES; i++){
        pteEntry_t *page = &supp->sup_privatePgTbl[i];
        memaddr vpn = ENTRYHI_GET_VPN(page->pte_entryHI);
        if (vpn >= *text_mem_start && vpn < *text_mem_start + *text_mem_size) {
            page->pte_entryLO &= ~DIRTYON;
        }
    }
}


void setSwapFloor() {
    unsigned int *data_mem_start = (unsigned int *)0x0018;
    unsigned int *data_mem_size = (unsigned int *)0x001C;
    swapFloor = *data_mem_start + *data_mem_size;
}




//#define DEBUG
#ifdef DEBUG
#include "tests/vmSupportTest.h"
#include "tests/vmSupportTest.c"
#endif // DEBUG

