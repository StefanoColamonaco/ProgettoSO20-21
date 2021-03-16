
#ifndef PROGETTOSO20_21_INIT_H
#define PROGETTOSO20_21_INIT_H


#include "pandos_types.h"
#include "pandos_const.h"
#include "pcb.h"
#include "asl.h"

extern int processCount;        //number of started but not yet terminated processes

extern int softBlockCount;      //number of processes blocked due to I/O

extern pcb_t *readyQueue;       //tail pointer to queue of ready processes

extern pcb_t *currentProcess;   //pointer to pcb that is in running state

extern semd_t deviceSemaphores[];


//todo add device semaphores




#endif //PROGETTOSO20_21_INIT_H
