#ifndef PROGETTOSO20_21_SCHEDULER_H
#define PROGETTOSO20_21_SCHEDULER_H

#include "pandos_types.h"

void scheduler();
void contextSwitch(pcb_t *currentProc);       /* function that implement context switching when necessary */
void copyStateInfo(state_t *src, state_t *dest);      /* change a state from a source to a destination state* */

#endif