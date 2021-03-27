#include "exceptions.h"

#include "pandos_const.h"
#include "pandos_types.h"

#include "init.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "systemCalls.h"
#include "interrupts.h"
#include "stateUtil.h"

#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>


void handleException() {
    unsigned int cause = getCAUSE();
    switch (CAUSE_GET_EXCCODE(cause)) {
        case EXC_INT:
            handleInterrupts();
            break;

        case EXC_MOD:
        case EXC_TLBL:
        case EXC_TLBS:
            //handleTLBEvents
            break;

        case EXC_ADEL:
        case EXC_ADES:
        case EXC_IBE:
        case EXC_DBE:
            //handleProgramTrap
            break;

        case EXC_SYS:
            handleSystemcalls();
            break;

        case EXC_BP:
        case EXC_RI:
        case EXC_CPU:
        case EXC_OV:
            //handleprogramTrap
            break;

    }
}

void TLBExceptionHandler() {
    passupOrDie(PGFAULTEXCEPT);
}

/*
se il processo corrente non ha una struttura di supporto questo viene terminato e il controllo torna allo scheduler, 
altrimenti accediamo all'area di memoria (BIOSDATAPAGE) in cui il BIOS ha salvato lo stato del processo prima che la 
system call fosse sollevata e diamo il controllo al BIOS exceptions handler
(anche in questo caso terminiamo il processo e passiamo il controllo allo scheduler)
*/
void passupOrDie(int exceptionType){
  if(currentProcess -> p_supportStruct != NULL){                                  
    copyStateInfo((state_t *) BIOSDATAPAGE, &(currentProcess -> p_supportStruct -> sup_exceptState[exceptionType]));
    LDCXT(currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_stackPtr,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_status,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_pc);
  }

  terminate_Process(currentProcess);
  scheduler();

}

//placeholder to replace in future implementations
/* void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST ((STATE_PTR) 0x0FFFF000);
} */
