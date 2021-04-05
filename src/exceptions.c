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
            handleSystemcalls();
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
    copyStateInfo((state_t *) BIOSDATAPAGE, &(currentProcess -> p_supportStruct -> sup_exceptState[exceptionType]));
    LDCXT(currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_stackPtr,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_status,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_pc);
  }

  terminate_Process(currentProcess);
  scheduler();

}

//placeholder to replace in future implementations
 /*void uTLB_RefillHandler() {
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST ((STATE_PTR) 0x0FFFF000);
}*/
