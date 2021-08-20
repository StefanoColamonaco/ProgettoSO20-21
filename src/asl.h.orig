#ifndef PROGETTOSO20_21_ASL_H
#define PROGETTOSO20_21_ASL_H

#include "pandos_const.h"
#include "pandos_types.h"

#define MAXADD 0x7FFFFFFF


/* ASL management */
void initASL();                                                     /* Fills semdFree list */
int insertBlocked(int *semAdd,pcb_t *p);                            /* Insert p into the queue of blocked processes associated to semAdd */
pcb_t *removeBlocked(int *semAdd);                                  /* Returns the first PCB from the queue of blocked processes */
pcb_t *outBlocked(pcb_t *p);                                        /* Remove p from the queue where it is blocked ( returns NULL otherwise ) */
pcb_t *headBlocked(int *semAdd);                                    /* Returns the PCB at the head of semAdd's queue */

/*Utility*/
semd_t *findSemInActiveList(int *semAdd);
semd_t* newSemd();
void freeSemd(semd_t *semd);

#endif //PROGETTOSO20_21_ASL_H
