
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

#include "stateUtil.h"
#include "init.h"
#include "scheduler.h"
#include "sysSupport.h"
#include "supportSystemCalls.h"

/* function that handle support level exceptions */
void handleSupportLevelExceptions(){
    support_t* supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0 ,0);
    unsigned int cause = supp->sup_exceptState[GENERALEXCEPT].cause;
 
    if (cause == EXC_MOD) {     //trying to write on read-only
        //treat as program trap
        programTrapHandler(supp);
    } else {                 
        handleSupportSystemcalls(supp);
    }
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
	loadProcess(currentProcess);
    //LDST((state_t*)BIOSDATAPAGE);
}

pteEntry_t *getMissingPageVariant(support_t* supp) {      //TODO add GETSUPP so we can use this in level 3
    pteEntry_t *pageTable = supp->sup_privatePgTbl;
    state_t* saved_state = &supp->sup_exceptState[0];
    unsigned int badVAddr = ENTRYHI_GET_VPN(saved_state->entry_hi);
    for (int i = 0; i < MAXPAGES; i++) {
        if ( ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == badVAddr) {
            return &pageTable[i];
        }
    }
    SYSCALL(TERMPROCESS, 0, 0, 0);
    return NULL;
}



pteEntry_t *getMissingPage() {      //TODO add GETSUPP so we can use this in level 3
    unsigned int badVAddr = getBADVADDR();
    // unsigned int savedEntryHI = ((state_t*)BIOSDATAPAGE)->entry_hi;
    // unsigned int badVAddr = ENTRYHI_GET_VPN(savedEntryHI);
    pteEntry_t *pageTable = currentProcess->p_supportStruct->sup_privatePgTbl;
    for (int i = 0; i < MAXPAGES; i++) {
        if ( ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == ENTRYHI_GET_VPN(badVAddr)) {
            return &pageTable[i];
        }
    }
    SYSCALL(TERMPROCESS, 0, 0, 0);
    return NULL;
}

void programTrapHandler( support_t *supportStruct){

    //controllare se ci sono mutue esclusioni da rilasciare

    /*if(supportStruct != NULL && supportStruct != 0 && supportStruct->sup_exceptContext != NULL){                                 
        
    }*/

    SYSCALL(TERMINATE,0,0,0);
}