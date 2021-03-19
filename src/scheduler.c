#include "pandos_const.h"

#include "init.h"
#include "scheduler.h"
#include "interrupts.h"
#include "exceptions.h"
#include "asl.h"
#include "pcb.h"

void scheduler() {}



void copyStateInfo(state_t *src, state_t *dest){
  for (int i = 0; i < STATE_GPR_LEN; i++) {
    dest -> gpr[i] = src -> gpr[i];
  }
  dest -> pc_epc = src -> pc_epc;
  dest -> cause = src -> cause;
  dest -> status = src -> status;
  dest -> hi = src -> hi;
}

void contextSwitch(pcb_t *currentProc){}
