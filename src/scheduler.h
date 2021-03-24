#ifndef PROGETTOSO20_21_SCHEDULER_H
#define PROGETTOSO20_21_SCHEDULER_H

#include "pandos_types.h"

void scheduler();

void contextSwitch(pcb_t *current);       /* function that implement context switching when necessary */

void setStatusForWaiting();

void prepareSwitch(pcb_t *p, int time);

#endif