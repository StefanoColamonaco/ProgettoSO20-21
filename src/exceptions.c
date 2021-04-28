#include "exceptions.h"

#include "pandos_const.h"
#include "pandos_types.h"

#include "init.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "nucleousSystemCalls.h"
#include "interrupts.h"
#include "stateUtil.h"

#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

/*Handler for exceptions*/
/*Linked through the passupvector*/
void handleExceptions() {
    unsigned int cause = getCAUSE();
    unsigned int excCode = CAUSE_GET_EXCCODE(cause);
    switch (excCode) {
        case EXC_INT:
            handleInterrupts();
            break;

        case EXC_MOD:
        case EXC_TLBL:
        case EXC_TLBS:
            TLBExceptionHandler();
            break;

        case EXC_ADEL:
        case EXC_ADES:
        case EXC_IBE:
        case EXC_DBE:
            passupOrDie(GENERALEXCEPT);
            break;

        case EXC_SYS:
            handleNucleusSystemcalls();
            break;

        case EXC_BP:
        case EXC_RI:
        case EXC_CPU:
        case EXC_OV:
            passupOrDie(GENERALEXCEPT);
            break;

    }
}

void TLBExceptionHandler() {
    passupOrDie(PGFAULTEXCEPT);
}

/*
if the current process has no support structure this is terminated and control returns to the scheduler,
otherwise we access the memory area (BIOSDATAPAGE) in which the BIOS has saved the state of the process before the
system call was raised and we give control to the BIOS exceptions handler
(also in this case we terminate the process and pass control to the scheduler)
*/
void passupOrDie(int exceptionType){
  if(currentProcess -> p_supportStruct != NULL && currentProcess -> p_supportStruct != 0){                                  
    copyState((state_t *) BIOSDATAPAGE, &(currentProcess -> p_supportStruct -> sup_exceptState[exceptionType]));
    LDCXT(currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].stackPtr,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].status,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].pc);
    // here call the support level general exception handler (then the support level systemcalls handler)
  }

  terminate_Process(currentProcess);
  scheduler();

}

static pteEntry_t *getMissingPage();

void uTLB_RefillHandler () {
	pteEntry_t *pageToWrite = getMissingPage();
	setENTRYHI(pageToWrite->pte_entryHI);
	setENTRYLO(pageToWrite->pte_entryLO);
	TLBWR();    //TODO replace with TLBWI() after a replacing algorithm is implemented	
	contextSwitch(currentProcess);
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