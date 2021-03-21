#include "pandos_const.h"
#include "pandos_types.h"

#include "init.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "exceptions.h"
#include "systemCalls.h"

#include "/usr/include/umps3/umps/libumps.h"   //attenzione, non so se funziona per tutti

void TLBExceptionHandler() {
    kill(PGFAULTEXCEPT);
}

/*
se il processo corrente non ha una struttura di supporto questo viene terminato e il controllo torna allo scheduler, 
altrimenti accediamo all'area di memoria (BIOSDATAPAGE) in cui il BIOS ha salvato lo stato del processo prima che la 
system call fosse sollevata e diamo il controllo al BIOS exceptions handler
(anche in questo caso terminiamo il processo e passiamo il controllo allo scheduler)
*/
void kill(int exceptionType){
  if(currentProcess -> p_supportStruct != NULL){                                  
    copyStateInfo((state_t *) BIOSDATAPAGE, &(currentProcess -> p_supportStruct -> sup_exceptState[exceptionType]));
    LDCXT(currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_stackPtr,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_status,
          currentProcess -> p_supportStruct -> sup_exceptContext[exceptionType].c_pc);
  }

  terminate_Process(currentProcess);
  scheduler();

}