
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



/*Utility functions*/
static passupvector_t initPassupVector();

static inline void loadIntervalTimer (unsigned int timeInMicroSecs);

static pcb_t *initFirstProcess();





/*test function*/
extern void test();




#endif //PROGETTOSO20_21_INIT_H
