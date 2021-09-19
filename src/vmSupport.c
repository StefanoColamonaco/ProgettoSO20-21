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
#include "sysSupport.h"

#define READBLK 2
#define WRITEBLK 3






swap_t swapTable[POOLSIZE];
unsigned int swapFloor;

int swapSem = 1;
semd_t swapSemStruct;


unsigned int freeAsidBitmap = 0b11111111;
static int frameIndexToReplace = 0;


int getVPNAddress(int index);

//static void initSwapSem();

static void initSwapTable();

static inline int frameIsOccupied(swap_t* frame);

static void updateFrameIndexToReplace();

static void markPTEntryNotValid(pteEntry_t* entry);

static void markTLBEntryNotValid(pteEntry_t* entry);

static void writeFromPoolToDev(swap_t* frame);

static void writeFromDevToPool(pteEntry_t *page);

static void markEntryPresentAtIndex(pteEntry_t *missingPage, int newIndex);

static void markValidAndUpdateTLB(pteEntry_t *page);

static void updateEntryInTLB(pteEntry_t* entry);

static void handleTLBInvalid();

static void markInvalidAndSave(swap_t* frameToReplace);

int PTEentryIsValid(pteEntry_t *entry);

static void updateSwapTableEntry(swap_t *swapPage, pteEntry_t* pageToAdd);




void initSwapStructs()
{
    initSwapTable();
    swapSem = 1;
}


static void initSwapTable()
{
    for (int i = 0; i < POOLSIZE; i++) {
        swapTable[i].sw_asid = -1;
    }
}


/*check bitmap for free Asid. Returns -1 if none is free*/
int getFreeAsid()
{
    for (int i = 1; i <= 8; i++) {
        if ( ((freeAsidBitmap >> i) & 1U) == ON ) {
            return i;
        }
    }

    return -1;
}


int getVPNAddress(int index)
{
    if (index < 0 || index > 31) {
        //TODO handle exception
    }

    if (index == 31) {
        return 0xBFFFF;
    }

    return 0x80000 + index;
}


void init_uproc_pagetable(support_t * supp)
{
    pteEntry_t *page_table = supp->sup_privatePgTbl;

    for (int i = 0; i < USERPGTBLSIZE; i++) {
        unsigned int vpn = getVPNAddress(i);
        page_table[i].pte_entryHI = (vpn << VPNSHIFT) | (supp->sup_asid << ASIDSHIFT);
        page_table[i].pte_entryLO = DIRTYON;
    }
}


/*pager*/
void handlePageFault()
{
    support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int cause = supp->sup_exceptState[0].cause;

    if (cause == EXC_MOD) {     //trying to write on read-only
        programTrapHandler(supp);
    } else {           //page fault on load or store. at this point the TLB has already been refilled
        SYSCALL(PASSEREN, (int)&swapSem, 0, 0);
        handleTLBInvalid(supp);
        SYSCALL(VERHOGEN, (int)&swapSem, 0, 0);

        //supp->sup_exceptState[0].pc_epc +=4;
        LDST(&(supp->sup_exceptState[0]));
    }
}

static void handleTLBInvalid(support_t *supp)
{
    pteEntry_t *missingPage = getMissingPageFromSupp(supp);
    updateFrameIndexToReplace();
    swap_t *frameToReplace = &swapTable[frameIndexToReplace];

    if (frameIsOccupied(frameToReplace)) {
        markInvalidAndSave(frameToReplace);
    }

    writeFromDevToPool(missingPage);
    updateSwapTableEntry(frameToReplace, missingPage);

    markValidAndUpdateTLB(missingPage);
}


static void markInvalidAndSave(swap_t *frameToReplace)
{
    disable_interrupts();
    markPTEntryNotValid(frameToReplace->sw_pte);
    markTLBEntryNotValid(frameToReplace->sw_pte);
    enable_interrupts();
    writeFromPoolToDev(frameToReplace);
}


static void markValidAndUpdateTLB(pteEntry_t *page)
{
    disable_interrupts();
    markEntryPresentAtIndex(page, frameIndexToReplace);      //TODO: Verify
    updateEntryInTLB(page);
    enable_interrupts();
}



static void updateFrameIndexToReplace()
{
    for (int i = 0; i < POOLSIZE; i++) {
        if (swapTable[i].sw_asid == -1){
            frameIndexToReplace = i;
            return;
        }
    }

    frameIndexToReplace = (frameIndexToReplace + 1) % POOLSIZE;
}


static inline int frameIsOccupied(swap_t* frame)
{
    return frame->sw_asid >= 1 && frame->sw_asid <= 8;
}


static void markPTEntryNotValid(pteEntry_t* entry)
{
    entry->pte_entryLO &= ~VALIDON;
}


static void markTLBEntryNotValid(pteEntry_t* entry)
{
    setENTRYHI(entry->pte_entryHI);
    TLBP();
    unsigned int indexReg = getINDEX();

    if (indexReg >> 31 == OFF) {  //check p bit for valid index
        TLBR();
        setENTRYHI(entry->pte_entryHI);
        unsigned int entryLO = getENTRYLO();
        entryLO &= ~VALIDON;
        setENTRYLO(entryLO);
        TLBWI();
    }
}


static inline int frameAddrInPool(int frameIndex)
{
    return swapFloor + frameIndex * 4096;
}

int getPageNum(unsigned int entryHi)
{
    unsigned int blockNum = ENTRYHI_GET_VPN(entryHi);

    if (blockNum == LASTPAGEMASK) {
        blockNum = MAXPAGES - 1;
    }

    return blockNum;
}

static void writeFromPoolToDev(swap_t* frame)
{
    int devNo = frame->sw_asid - 1;
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(IL_FLASH, devNo);
    int blockNum = frame->sw_pageNo;        //TODO check index 31. 
    disable_interrupts();
    dev->dtp.data0 = FRAMEPOOLSTART + ENTRYLO_GET_PFN(frame->sw_pte->pte_entryLO) * PAGESIZE;
    dev->dtp.command = blockNum << 8 | WRITEBLK;
    SYSCALL(IOWAIT, FLASHINT, devNo, 0);
    enable_interrupts();
}

static void writeFromDevToPool(pteEntry_t *page)
{
    int devNo = ENTRYHI_GET_ASID(page->pte_entryHI) - 1;
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(IL_FLASH, devNo);
    unsigned int blockNum = getPageNum(page->pte_entryHI);
    disable_interrupts();
    dev->dtp.data0 = FRAMEPOOLSTART + frameIndexToReplace * PAGESIZE;
    dev->dtp.command = blockNum << 8 | READBLK;
    SYSCALL(IOWAIT, FLASHINT, devNo, 0);
    enable_interrupts();
}


static void updateSwapTableEntry(swap_t *swapEntry, pteEntry_t* pageToAdd)
{
    swapEntry->sw_asid = ENTRYHI_GET_ASID(pageToAdd->pte_entryHI);
    swapEntry->sw_pte = pageToAdd;
    swapEntry->sw_pageNo = getPageNum(pageToAdd->pte_entryHI);
}


static void markEntryPresentAtIndex(pteEntry_t *page, int newIndex)
{
    unsigned int frameAddr = FRAMEPOOLSTART + frameIndexToReplace * PAGESIZE;
    page->pte_entryLO = frameAddr | VALIDON | DIRTYON;
}



void updateEntryInTLB(pteEntry_t* entry)
{
    setENTRYHI(entry->pte_entryHI);
    TLBP();
    unsigned int indexReg = getINDEX();

    if (indexReg >> 31 == OFF) {  //check p bit for valid index
        //TLBR();
        setENTRYHI(entry->pte_entryHI);
        setENTRYLO(entry->pte_entryLO);
        TLBWI();
    }
}


void removePagesFromTLB(int asid)
{
    support_t* supp = (support_t*)SYSCALL(GETSUPPORTPTR, 0, 0, 0);

    for (int i = 0; i < MAXPAGES; i++) {
        if (PTEentryIsValid(&supp->sup_privatePgTbl[i])) {
            markTLBEntryNotValid(&supp->sup_privatePgTbl[i]);
        }
    }
}



int PTEentryIsValid(pteEntry_t *entry)
{
    return (entry->pte_entryLO & ENTRYLO_VALID) >> ENTRYLO_VALID_BIT;
}



// void markReadOnlyPages(support_t *supp)
// {
//     pteEntry_t* header = &supp->sup_privatePgTbl[0];
//     writeFromDevToPool(header);

//     unsigned int *text_mem_start = (unsigned int *)FRAMEPOOLSTART + PAGESIZE * (supp->sup_asid - 1) + 0x0004;
//     unsigned int *text_mem_size = (unsigned int *)FRAMEPOOLSTART + PAGESIZE * (supp->sup_asid - 1) + 0x000C;

//     for (int i = 0; i < MAXPAGES; i++) {
//         pteEntry_t *page = &supp->sup_privatePgTbl[i];
//         memaddr vpn = ENTRYHI_GET_VPN(page->pte_entryHI);

//         if (vpn < *text_mem_size) {
//             page->pte_entryLO &= ~DIRTYON;
//         }
//     }
// }


void setSwapFloor()
{
    unsigned int *data_mem_start = (unsigned int *)0x0018;
    unsigned int *data_mem_size = (unsigned int *)0x001C;
    swapFloor = *data_mem_start + *data_mem_size;
}




//#define DEBUG
#ifdef DEBUG
#include "tests/vmSupportTest.h"
#include "tests/vmSupportTest.c"
#endif // DEBUG

