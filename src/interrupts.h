#ifndef PROGETTOSO20_21_INTERRUPTS_H
#define PROGETTOSO20_21_INTERRUPTS_H

#include "init.h"
#include "scheduler.h"

void handleInterrupts();


/*UTILS*/

void releaseSemAssociatedToDevice(int deviceNo, unsigned int status);

unsigned int getSemNumber(unsigned int interruptLine, unsigned int deviceNo, int termIsTRANSM);

#endif