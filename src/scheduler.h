#ifndef PROGETTOSO20_21_SCHEDULER_H
#define PROGETTOSO20_21_SCHEDULER_H

#include "pandos_types.h"

void scheduler();

void loadProcess(pcb_t *current);

void setStatusForWaiting();

void updateTimeAndSwitch(pcb_t *p, int time);

#endif