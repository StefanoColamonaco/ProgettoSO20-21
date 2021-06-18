
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

#include "init.h"
#include "sysSupport.h"
#include "supportSystemCalls.h"

static pteEntry_t *getMissingPage();

/* function that handle support level exceptions */
void handleSupportLevelExceptions(){    
                                                //sono necessarie inizializzazioni ?                    
    handleSupportSystemcalls();
}

void uTLB_RefillHandler () {
	pteEntry_t *pageToWrite = getMissingPage();
	setENTRYHI(pageToWrite->pte_entryHI);
	setENTRYLO(pageToWrite->pte_entryLO);
	TLBWR();    //TODO replace with TLBWI() after a replacing algorithm is implemented	
	contextSwitch(currentProcess);
}

int *getMissingPageNumber() {
    unsigned int badVAddr = ((state_t *)BIOSDATAPAGE)->gpr[CP0_BadVAddr]; //TODO check if that´s the correct status
    pteEntry_t *pageTable = currentProcess->p_supportStruct->sup_privatePgTbl;
    for (int i = 0; i < MAXPAGES; i++) {
        if (ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == badVAddr) {
            //return ENTRYLO_GET_PFN(pageTable[i].pte_entryLO);
            return i;
        }
    }
    SYSCALL(TERMPROCESS, 0, 0, 0); //no page matching
}

pteEntry_t *getMissingPage() {
    unsigned int badVAddr = ((state_t *)BIOSDATAPAGE)->gpr[CP0_BadVAddr]; //TODO check if that´s the correct status
    pteEntry_t *pageTable = currentProcess->p_supportStruct->sup_privatePgTbl;
    for (int i = 0; i < MAXPAGES; i++) {
        if (ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == badVAddr) {
            //return ENTRYLO_GET_PFN(pageTable[i].pte_entryLO);
            return &pageTable[i];
        }
    }
    SYSCALL(TERMPROCESS, 0, 0, 0); //no page matching
}