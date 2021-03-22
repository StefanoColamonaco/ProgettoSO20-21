#include "pandos_const.h"

#include "init.h"
#include "scheduler.h"
#include "interrupts.h"
#include "exceptions.h"
#include "asl.h"
#include "pcb.h"

#include "/usr/include/umps3/umps/libumps.h"

void scheduler() {

  pcb_t *p = removeProcQ(&readyQueue);
  if(p != NULL){
    //STCK(startT);
    setTIMER(5000);
    contextSwitch(p);
  }

  if(processCount == 0){
    HALT();
  } else {
    if(softBlockCount > 0){
      currentProcess = NULL;
      WAIT();
    } else {                     //we got deadlock
      PANIC();
    }
  }
}

void copyStateInfo(state_t *src, state_t *dest){
  for (int i = 0; i < STATE_GPR_LEN; i++) {
    dest -> gpr[i] = src -> gpr[i];
  }
  dest -> pc_epc = src -> pc_epc;
  dest -> cause = src -> cause;
  dest -> status = src -> status;
  dest -> hi = src -> hi;
}

/*caso in cui lo scheduler decida di eseguire un altro processo. In tal caso salviamo il processo attuale e richiamiamo
la funzione di exception handling del Bios (che ci è stata fornita) passando lo stato del processo corrente.
*/
void contextSwitch(pcb_t *current){
  currentProcess = current;
  LDST(&(currentProcess -> p_s));
}
