
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


void handleSupportLevelExceptions()
{
    support_t* supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
    unsigned int cause = supp->sup_exceptState[GENERALEXCEPT].cause;

    if (cause == EXC_MOD) {     //trying to write on read-only
        programTrapHandler(supp);
    } else {
        handleSupportSystemcalls(supp);
    }
}

unsigned int tlbIndex = 0;

void updateTLBIndex()
{
    tlbIndex = (tlbIndex + 1) % TLBSIZE;
}


void uTLB_RefillHandler ()
{
    pteEntry_t *pageToWrite = getMissingPage();
    setENTRYHI(pageToWrite->pte_entryHI);
    setENTRYLO(pageToWrite->pte_entryLO);
    setINDEX(tlbIndex << INDEXSHIFT);
    TLBWI();    //TODO replace with TLBWI() after a replacing algorithm is implemented
    updateTLBIndex();
    loadProcess(currentProcess);
    //LDST(&currentProcess->p_supportStruct->sup_exceptState[0]); //non capisco perchè non va
}


pteEntry_t *getMissingPageFromSupp(support_t* supp)
{
    pteEntry_t *pageTable = supp->sup_privatePgTbl;
    state_t* saved_state = &supp->sup_exceptState[PGFAULTEXCEPT];
    unsigned int badVAddr = ENTRYHI_GET_VPN(saved_state->entry_hi);

    for (int i = 0; i < MAXPAGES; i++) {
        if ( ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == badVAddr) {
            return &pageTable[i];
        }
    }

    SYSCALL(TERMPROCESS, 0, 0, 0);
    return NULL;
}



pteEntry_t *getMissingPage()
{
    unsigned int badVAddr = getBADVADDR();
    pteEntry_t *pageTable = currentProcess->p_supportStruct->sup_privatePgTbl;

    for (int i = 0; i < MAXPAGES; i++) {
        if ( ENTRYHI_GET_VPN(pageTable[i].pte_entryHI) == ENTRYHI_GET_VPN(badVAddr)) {
            return &pageTable[i];
        }
    }

    SYSCALL(TERMPROCESS, 0, 0, 0);
    return NULL;
}

void programTrapHandler( support_t *supportStruct)
{

    //controllare se ci sono mutue esclusioni da rilasciare

    /*if(supportStruct != NULL && supportStruct != 0 && supportStruct->sup_exceptContext != NULL){

    }*/

    SYSCALL(TERMINATE, 0, 0, 0);
}


