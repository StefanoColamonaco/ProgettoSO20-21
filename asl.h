#ifndef PROGETTOSO20_21_ASL_H
#define PROGETTOSO20_21_ASL_H


#include <pandos_const.h>
#include <pandos_types.h>

/*Table of free semaphores*/
//extern semd_table[MAXPROC];

/* Pointer to list of free semaphores*/
//extern semdFree_h;                                        // [Attenzione] manca il tipo del semaforo da definire

/* Pointer to list of active semaphores */
//extern semd_h

/* ASL management */
int insertBlocked(int *semAdd,pcb_t *p);                            /* Insert p into the queue of blocked processes associated to semAdd */
pcb_t *removeBlocked(int *semAdd);                                  /* Returns the first PCB from the queue of blocked processes */
pcb_t *outBlocked(pcb_t *p);                                        /* Remove p from the queue where it is blocked ( returns NULL otherwise ) */
pcb_t *headBlocked(int *semAdd);                                    /* Returns the PCB at the head of semAdd's queue */
void initASL();                                                     /* Fills semdFree list */



#endif //PROGETTOSO20_21_ASL_H
