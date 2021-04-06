#ifndef PROGETTOSO20_21_INTERRUPTS_H
#define PROGETTOSO20_21_INTERRUPTS_H

#include "init.h"
#include "scheduler.h"

/*interrupt handling entry point*/
void handleInterrupts();


/*UTILS*/

void releaseSemAndUpdateStatus(int deviceNo, unsigned int status);

unsigned int getSemIndex(unsigned int interruptLine, unsigned int deviceNo, int termIsTRANSM);

#endif