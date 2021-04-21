#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <pandos_types.h>
#include <pandos_const.h>
#include <asl.h>

#include "init.h"

swap_t swapTable[POOLSIZE];
int swapSemaphore = 1;

unsigned int freeAsidBitmap = 0b11111111;   


void initUprocPageTable(pcb_t *uproc);

static int getVPNAddress(int index);

static inline int getFreeAsid();

static inline int getVPNAddress(int index);



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
    unsigned int cause = currentProcess->p_s.cause;
    if (cause == TLB)



}