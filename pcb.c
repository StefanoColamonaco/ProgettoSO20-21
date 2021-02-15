#include "pcb.h"

pcb_PTR pcbFree_h;

pcb_t pcbFree_table[MAXPROC];


/* Fills pcbFree_h list */

void initPcbs() {
    pcbFree_h = &pcbFree_table[0];
    for (int i = 0; i < MAXPROC; i++) {
        pcbFree_table[i % MAXPROC].p_next = &pcbFree_table[(i + 1) % MAXPROC];
        pcbFree_table[(i + 1) % MAXPROC].p_prev = &pcbFree_table[i % MAXPROC];
    }
}


/* Insert a PCB into pcbFree_h list */

void freePcb(pcb_t *p) {
    p->p_next = pcbFree_h->p_next;
    p->p_prev = pcbFree_h;
    pcbFree_h->p_next = p;
    p->p_next->p_prev = p;
}


/* PCB allocation and initialization*/

pcb_t *allocPcb() {
    if (pcbFree_h == NULL) {
        return NULL;
    } else {
        pcb_PTR toReturn = pcbFree_h->p_next;
        pcbFree_h->p_next = toReturn->p_next;
        toReturn->p_next->p_prev = toReturn->p_prev;

        toReturn->p_prev = NULL;
        toReturn->p_next = NULL;
        toReturn->p_child = NULL;
        toReturn->p_prnt = NULL;
        toReturn->p_next_sib = NULL;
        toReturn->p_prev_sib = NULL;
        toReturn->p_time = 0;
        //toReturn->p_s = NULL; //todo figure out what to do with this member

        return toReturn;
    }
}


/* Make an empty PCB list */

pcb_t *mkEmptyProcQ() {
    return NULL;
}


/* Returns TRUE if the list is empty, FALSE otherwise */

int emptyProcQ(pcb_t *tp) {
    if (pcbFree_h == NULL)
        return TRUE;
    else
        return FALSE;
}


void insertProcQ(pcb_t **tp, pcb_t *p);                              /* Insert PCB p at the end of tp list */
