#ifndef PROGETTOSO20_21_ASL_H
#define PROGETTOSO20_21_ASL_H


#include "pandos_const.h"
#include "pandos_types.h"

/*Table of free semaphores*/
extern semd_t semd_table[];

/* Pointer to list of free semaphores*/
extern semd_PTR semdFree_h;

/* Pointer to list of active semaphores */
extern semd_PTR semd_h;

/* ASL management */
void initASL();                                                     /* Fills semdFree list */
int insertBlocked(int *semAdd,pcb_t *p);                            /* Insert p into the queue of blocked processes associated to semAdd */
pcb_PTR removeBlocked(int *semAdd);                                  /* Returns the first PCB from the queue of blocked processes */
pcb_PTR outBlocked(pcb_t *p);                                        /* Remove p from the queue where it is blocked ( returns NULL otherwise ) */
pcb_PTR headBlocked(int *semAdd);                                    /* Returns the PCB at the head of semAdd's queue */


#endif //PROGETTOSO20_21_ASL_H
