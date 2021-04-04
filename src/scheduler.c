#include "pandos_const.h"

#include "init.h"
#include "scheduler.h"
#include "interrupts.h"
#include "exceptions.h"
#include "asl.h"
#include "pcb.h"
#include "stateUtil.h"

#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>

void scheduler() {

  pcb_t *p = removeProcQ(&readyQueue);
  if(p != NULL){
    prepareSwitch(p, TIMESLICE);
  }

  if(processCount == 0){
    HALT();
  } else {
    if(softBlockedCount > 0){
      currentProcess = NULL;
      setStatusForWaiting();
      WAIT();
    } else {                     //deadlock
      PANIC();
    }
  }
}

/*caso in cui lo scheduler decida di eseguire un altro processo. In tal caso salviamo il processo attuale e richiamiamo
la funzione di exception handling del Bios (che ci è stata fornita) passando lo stato del processo corrente.
*/
void contextSwitch(pcb_t *current){
  currentProcess = current;
  LDST(&(currentProcess->p_s));
}

void setStatusForWaiting() {
    setTIMER((unsigned int) 0xFFFFFFFF);
    unsigned int newState = ALLOFF | IECON | IMON;
    setSTATUS(newState);
}

void prepareSwitch(pcb_t *p, int time) {
  STCK(startT);
  setTIMER(time);
  contextSwitch(p);
}

