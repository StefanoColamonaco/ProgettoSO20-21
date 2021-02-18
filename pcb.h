#ifndef PROGETTOSO20_21_PCB_H
#define PROGETTOSO20_21_PCB_H


#include "pandos_types.h"
#include "pandos_const.h"


/* Pointer to list of free pcbs*/
extern pcb_PTR pcbFree_h;


/*Table of free pcbs*/
extern pcb_t pcbFree_table[MAXPROC];


/* PCB allocation */

void initPcbs();                                                    /* Fills pcbFree_h list */
void freePcb(pcb_t *p);                                            /* Insert a PCB in pcbFree_h list */
pcb_PTR allocPcb();                                                  /* PCB allocation and initialization*/

/* PCB list */

pcb_PTR mkEmptyProcQ();                                              /* Make an empty PCB list */
intemptyProcQ(pcb_t *tp);                                          /* Returns TRUE if the list is empty, FALSE otherwise */
void insertProcQ(pcb_t **tp, pcb_t *p);                              /* Insert PCB p at the end of tp list */
pcb_PTR headProcQ(pcb_t *tp);                                       /* Return the element at the end of tp ( NULL otherwise ) */
pcb_PTR removeProcQ(pcb_t **tp);                                     /* Removes oldest element from tp and return a pointer */
pcb_PTR outProcQ(pcb_t **tp, pcb_t *p);                               /* Removes PCB p from tp list ( return NULL otherwise ) */

/* PCB trees */

int emptyChild(pcb_t *p);                                           /* Returns TRUE if p has no childrens, FALSE otherwise */
void insertChild(pcb_t *prnt, pcb_t *p);                              /* Insert p as child of prnt  */
pcb_PTR removeChild(pcb_t *p);                                       /* Removes firs child of p ( return NULL otherwise ) */
pcb_PTR outChild(pcb_t *p);                                          /* Removes P from the father's children */




#endif //PROGETTOSO20_21_PCB_H
