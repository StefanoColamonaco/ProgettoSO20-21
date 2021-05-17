#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>


#include "pandos_types.h"
#include "pandos_const.h"
#include "asl.h"
#include "init.h"
#include "scheduler.h"
#include "interrupts.h"

#define READBLK 2 
#define WRITEBLK 3


swap_t swapTable[POOLSIZE];
int swapSemaphore = 1;

unsigned int freeAsidBitmap = 0b11111111; 

static int frameIndexToReplace = 0;


void initUprocPageTable(pcb_t *uproc);

static int getVPNAddress(int index);

static inline int getFreeAsid();

static inline int getVPNAddress(int index);

pteEntry_t *getMissingPage();

int getMissingPageNumber();

static inline int frameIsOccupied(swap_t* frame);




void initUprocPageTable(pcb_t *uproc) {
    int asid = uproc->p_supportStruct->sup_asid;
    int vpn;
    for (int i = 0; i < USERPGTBLSIZE; i++) {
        vpn = getVPNAddress(i);
        unsigned int entryHI = vpn << VPNSHIFT + asid << ASIDSHIFT;
        unsigned int entryLO = DIRTYON;
        uproc->p_supportStruct->sup_privatePgTbl[i].pte_entryHI = entryHI;
        uproc->p_supportStruct->sup_privatePgTbl[i].pte_entryLO = entryLO;
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


void initSwapTable() {
    for (int i = 0; i < POOLSIZE; i++) {
        swapTable->sw_asid = -1;
    }
}



void handlePageFault() {
    SYSCALL(GETSUPPORTPTR, 0, 0 ,0);
    support_t *supp = currentProcess->p_s.reg_v0;
    unsigned int cause = supp->sup_exceptState[0].cause;
    if (cause == EXC_MOD) {
        //treat as program trap
    } else {
        SYSCALL(PASSEREN, &swapSemaphore, 0, 0);

        int missingPageNumber = getMissingPageNumber();
        pteEntry_t *missingPage = &supp->sup_privatePgTbl[missingPageNumber];
        int frameIndexToReplace = getFrameIndexToReplace();
        swap_t* frameToReplace = &swapTable[frameIndexToReplace];

        if(frameIsOccupied(&frameToReplace)) {
            //update with interrupts disabled
            unsigned int oldStatus = getSTATUS();
            setSTATUS(oldStatus & DISABLEINTS);
            markPTEntryNotValid(frameToReplace->sw_pte);
            markTLBEntryNotValid(frameToReplace->sw_pte);
            setSTATUS(oldStatus);
            writeFrameToBackingStore(frameToReplace);
        }
        //TODO read currentProcess´ status from backing store to frame i

        getPageFromBackingStore(supp->sup_asid, missingPageNumber, missingPage);
        updateSwapPoolEntry(frameToReplace, supp->sup_asid, missingPage);
        
        ///update with interrupts disabled
        unsigned int oldStatus = getSTATUS();
        setSTATUS(oldStatus & DISABLEINTS);
        markEntryPresentAtIndex(missingPage, frameIndexToReplace);
        updateEntryInTLB(missingPage);
        setSTATUS(oldStatus);

        SYSCALL(VERHOGEN, &swapSemaphore, 0, 0);    
        contextSwitch(currentProcess);   

    }
}

/*stub to edit at the end*/
int getFrameIndexToReplace() {       
    int result = frameIndexToReplace;
    frameIndexToReplace = (frameIndexToReplace + 1)%POOLSIZE;
    return result;
}

/* static inline int frameIsOccupied(pteEntry_t* frame) {
    return (ENTRYHI_GET_ASID(frame->pte_entryHI) < 0);
} */

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

void writeFrameToBackingStore(swap_t* frame) {
    int devNo = frame->sw_asid-1;
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(IL_FLASH, devNo);
    int blockNum = frame->sw_pageNo;
    dev->dtp.data0 = ENTRYLO_GET_PFN(frame->sw_pte->pte_entryLO);
    dev->dtp.command = blockNum << 8 | WRITEBLK;
    SYSCALL(IOWAIT, deviceSemaphores[getSemIndex(IL_FLASH, devNo, 0)], 0 ,0);
}

void readPageFromBackingStore(int asid, int missingPageNum, pteEntry_t *missingPage) {
    int devNo = asid - 1;
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(IL_FLASH, devNo);
    int blockNum = missingPageNum;
    dev->dtp.data0 = ENTRYLO_GET_PFN(missingPage->pte_entryLO);
    dev->dtp.command = blockNum << 8 | READBLK;
    SYSCALL(IOWAIT, deviceSemaphores[getSemIndex(IL_FLASH, devNo, 0)], 0 ,0);

}


updateSwapPoolEntry(swap_t *poolFrame, int asid, pteEntry_t* pageToAdd) {
    poolFrame->sw_asid = asid;
    poolFrame->sw_pte = pageToAdd;
    poolFrame->sw_pageNo = (pageToAdd->pte_entryHI & GETPAGENO) >> 12;
}

markEntryPresentAtIndex(pteEntry_t *missingPage, int newIndex) {
    missingPage->pte_entryLO |= VALIDON;
    missingPage->pte_entryLO &= ~ENTRYLO_PFN_MASK | newIndex << ENTRYLO_PFN_BIT;
}


static void updateEntryInTLB(pteEntry_t* entry) {
    setENTRYHI(entry->pte_entryHI);
    TLBP();
    unsigned int indexReg = getINDEX();
    if (indexReg >> 31 == OFF) {  //check p bit for valid index
        TLBR();
        setENTRYLO(entry->pte_entryLO);
        TLBWI();
    }
}

/*unit tests for this module. To move to another file*/

void testGetFreeAsid() {
    extern unsigned int freeAsidBitmap;

    freeAsidBitmap = 2;
    int asid = getFreeAsid();
    if (asid != 1) {
        print("asid is incorrect: not 1\n");
    }

    freeAsidBitmap = 0;
    asid = getFreeAsid();
    if (asid != -1) {
        print ("no free asid but -1 not returned\n");
    }
}

