
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

#include "init.h"
#include "scheduler.h"
#include "sysSupport.h"
#include "supportSystemCalls.h"

/* function that handle support level exceptions */
void handleSupportLevelExceptions(){    
                                                //sono necessarie inizializzazioni ?                    
    handleSupportSystemcalls();
}

unsigned int tlbIndex = 0;

void updateTLBIndex(){
    tlbIndex = (tlbIndex + 1) % TLBSIZE;
}

void uTLB_RefillHandler () {
	pteEntry_t *pageToWrite = getMissingPage();
	setENTRYHI(pageToWrite->pte_entryHI);
	setENTRYLO(pageToWrite->pte_entryLO);
    setINDEX(tlbIndex << INDEXSHIFT);
	TLBWI();    //TODO replace with TLBWI() after a replacing algorithm is implemented
    updateTLBIndex();	
	contextSwitch(currentProcess);
}




pteEntry_t *getMissingPage() {      //TODO add GETSUPP so we can use this in level 3
    unsigned int badVAddr = getBADVADDR();
    pteEntry_t *pageTable = currentProcess->p_supportStruct->sup_privatePgTbl;
    for (int i = 0; i < MAXPAGES; i++) {
        if ( ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == ENTRYHI_GET_VPN(badVAddr)) {
            return &pageTable[i];
        }
    }
    SYSCALL(TERMPROCESS, 0, 0, 0);
    return -1;
}