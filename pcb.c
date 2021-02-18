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
    if (tp == NULL)
        return TRUE;
    else
        return FALSE;
}


/* Insert PCB p at the end of tp list */

void insertProcQ(pcb_t **tp, pcb_t *p) {
    if (emptyProcQ(*tp)) {
        p->p_prev = p->p_next = p;
    } else {
        p->p_prev = *tp;
        p->p_next = (*tp)->p_next;

        (*tp)->p_next = p;
        p->p_next->p_prev = p;
    }
    *tp = p;
}


/* Return the element at the end of tp ( NULL otherwise ) */

pcb_t *headProcQ(pcb_t *tp) {
    if (emptyProcQ(tp)) return NULL;
    else return tp->p_next;
}


/* Removes oldest element from tp and return a pointer */

pcb_t *removeProcQ(pcb_t **tp) {
    if(emptyProcQ(*tp))
        return NULL;
    if(headProcQ(*tp) == (*tp)) {
      pcb_t *temp = (*tp) -> p_next;
      temp -> p_prev -> p_next = temp -> p_next;
      *tp = NULL;
      return temp;
    }
    pcb_t *temp = (*tp) -> p_next;
    (*tp) -> p_next = temp -> p_next;
    temp -> p_next -> p_prev = *tp;
    temp -> p_prev = temp -> p_next = NULL;
    return temp;

}


/* Removes PCB p from tp list ( return NULL otherwise ) */

pcb_t *outProcQ(pcb_t **tp, pcb_t *p) { //todo test, and possibly rewrite
    if(emptyProcQ((*tp)))
        return NULL;
    if((*tp) == p)
        return removeProcQ(tp);
    pcb_t *tpTemp = (*tp);
    while(tpTemp -> p_next != (*tp)){
        if(tpTemp -> p_next == p){
            pcb_t *temp = p;
            temp -> p_next -> p_prev = temp -> p_prev;
            temp -> p_prev -> p_next = temp -> p_next;
            temp -> p_prev = temp -> p_next = NULL;
            return temp;
        } else {
            tpTemp = tpTemp -> p_next;
        }
    }
  return NULL;
}
