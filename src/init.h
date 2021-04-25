
#ifndef PROGETTOSO20_21_INIT_H
#define PROGETTOSO20_21_INIT_H


#include "pandos_types.h"
#include "pandos_const.h"

#define DEVICE_NUM 49

extern int processCount;        //number of started but not yet terminated processes

extern int softBlockedCount;      //number of processes blocked due to I/O

extern pcb_t *readyQueue;       //tail pointer to queue of ready processes

extern pcb_t *currentProcess;   //pointer to pcb that is in running state

extern cpu_t startT;

extern cpu_t globalStartT;

extern int deviceSemaphores[];

#define clockSemaphore deviceSemaphores[DEVICE_NUM-1]


/*test function*/
extern void test();                          //for phase 2
extern void test_phase_3();                  //for phase 3




#endif //PROGETTOSO20_21_INIT_H
