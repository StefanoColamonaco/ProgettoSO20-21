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
    if (emptyProcQ(tp))
        return NULL;
    else
        return tp->p_next;
}


/* Removes oldest element from tp and return a pointer */

/*pcb_t *removeProcQ(pcb_t **tp) {
    if (emptyProcQ(*tp))
        return NULL;
    if (headProcQ(*tp) == (*tp)) {
      pcb_t *tmp = (*tp) -> p_next;
      tmp -> p_prev -> p_next = tmp -> p_next;
      *tp = NULL;
    } else {
        pcb_t *tmp = (*tp) -> p_next;
        (*tp) -> p_next = tmp -> p_next;
        tmp -> p_next -> p_prev = *tp;
        tmp -> p_prev = tmp -> p_next = NULL;
    }
    return tmp;

}*/

pcb_t *removeProcQ(pcb_t **tp) {
    if (emptyProcQ(*tp))
        return NULL;

    pcb_t *tmp = headProcQ(*tp);

    if (tmp == *tp) {
        *tp = NULL;
    }

    return unlinkPCB(tmp);
}


/* Removes PCB p from tp list ( return NULL otherwise ) */

pcb_t *outProcQ(pcb_t **tp, pcb_t *p) {         //todo refactor to remove repeated code
    if (emptyProcQ((*tp)))
        return NULL;

    if (*tp == p) {
        if (*tp != (*tp)->p_prev) {
            *tp = (*tp)->p_prev;
        } else {
            *tp = NULL;
        }
        return unlinkPCB(p);
    }

    pcb_t *tmp = (*tp)->p_next;
    do {
        if (tmp == p) {
            return unlinkPCB(tmp);
        } else {
            tmp = tmp->p_next;
        }
    } while (tmp != *tp);

    return NULL;
}

/*
pcb_t *outProcQ(pcb_t **tp, pcb_t *p) {
    if(emptyProcQ((*tp)))
        return NULL;
    if((*tp) == p)
        return removeProcQ(tp);
    pcb_t *tpTmp = (*tp);
    while(tpTmp -> p_next != (*tp)){    //qua non possiamo usare removeProcQ?
        if(tpTmp -> p_next == p){
            pcb_t *tmp = p;     //tmp possiamo toglierlo, volendo
            tmp -> p_next -> p_prev = tmp -> p_prev;
            tmp -> p_prev -> p_next = tmp -> p_next;
            tmp -> p_prev = tmp -> p_next = NULL;
            return tmp;
        } else {
            tpTmp = tpTmp -> p_next;
        }
    }
    return NULL;
}
*/

/* Returns TRUE if p has no childrens, FALSE otherwise */

int emptyChild(pcb_t *p) {
    return (p->p_child == NULL);
}

/* Insert p as child of prnt  */

/*void insertChild(pcb_t *prnt, pcb_t *p){
    p -> p_prnt = prnt;
    p -> p_prev_sib = NULL;
    if (emptyChild(prnt)) {
        p -> p_next_sib = NULL;
    } else {
        p->p_next_sib = prnt->p_child;
        prnt->p_child->p_prev_sib = p;
    }
    prnt -> p_child = p;
}*/

/* function is O(n) because the list is required to be NULL terminated and we only have a head-pointer */
void insertChild(pcb_t *prnt, pcb_t *p) {
    p->p_prnt = prnt;
    p->p_next_sib = NULL;
    if (emptyChild(prnt)) {
        prnt->p_child = p;
        p->p_prev_sib = NULL;
    } else {
        pcb_t *iter = prnt->p_child;
        while (iter->p_next != NULL) {
            iter = iter->p_next;
        }
        iter->p_next_sib = p;
        p->p_prev_sib = iter;
    }

}

/* Removes first child of p and returns it ( return NULL otherwise ) */

/*
pcb_t *removeChild(pcb_t *p){
  if(emptyChild(p))
    return NULL;

  //pcb_PTR child = p->p_child;

  if(p -> p_child -> p_prev_sib == NULL){
    pcb_t *tmp = p -> p_child;
    p -> p_child = NULL;
    return tmp;
  }
  pcb_t *tmp = p -> p_child;
  tmp -> p_prev_sib -> p_next_sib = NULL;
  tmp -> p_prnt -> p_child = tmp -> p_prev_sib;
  tmp -> p_prnt = NULL;
  tmp -> p_prev_sib = NULL;
  tmp -> p_prnt = NULL;
  tmp -> p_prev_sib = NULL;
  return tmp;
}
*/

pcb_t *removeChild(pcb_t *p) {
    if (emptyChild(p))
        return NULL;

    pcb_PTR child = p->p_child;
    p->p_child = child->p_next_sib;

    unlinkChild(child);

    return child;
}

/* Removes P from the father's children */

/*pcb_t *outChild(pcb_t *p){
  if(p -> p_prnt == NULL)
    return NULL;

  if(p -> p_prnt -> p_child == p)
    return removeChild(p -> p_prnt);
  
  if(p -> p_prev_sib == NULL){
    p -> p_next_sib -> p_prev_sib = NULL;
    p -> p_next_sib = NULL;
    p -> p_prnt = NULL;
    return p;
  }
  pcb_t *tmp = p;
  p -> p_next_sib -> p_prev_sib = p -> p_prev_sib;
  p -> p_prev_sib -> p_next_sib = p -> p_next_sib;
  p -> p_next_sib = NULL;
  p -> p_prev_sib = NULL;
  p -> p_prnt = NULL;
  return tmp;
}*/

pcb_t *outChild(pcb_t *p) {
    if (p->p_prnt == NULL)
        return NULL;

    if (p->p_prnt->p_child == p) { //if p is first child, update parent's child pointer
        p->p_prnt->p_child = p->p_next_sib;
    }

    unlinkChild(p);
    return p;
}

pcb_t *unlinkPCB(pcb_t *p) {
    p->p_prev->p_next = p->p_next;
    p->p_next->p_prev = p->p_prev;
    p->p_prev = NULL;
    p->p_next = NULL;
    return p;
}

pcb_t *unlinkChild(pcb_t *p) {
    p->p_prnt = NULL;
    if (p->p_prev_sib != NULL) {
        p->p_prev_sib->p_next_sib = p->p_next_sib;
        p->p_prev_sib = NULL;
    }
    if (p->p_next_sib != NULL) {
        p->p_next_sib->p_prev_sib = p->p_prev_sib;
        p->p_next_sib = NULL;
    }
    return p;
}