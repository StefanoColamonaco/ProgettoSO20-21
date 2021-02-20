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

/*
void freePcb(pcb_t *p) {
    if (pcbFree_h == NULL) {
        pcbFree_h = p;
        p->p_next = p->p_prev = p;
    } else {
        p->p_next = pcbFree_h->p_next;
        p->p_prev = pcbFree_h;
        pcbFree_h->p_next = p;
        p->p_next->p_prev = p;
    }
}
*/

void freePcb(pcb_t *p) {    //list is circular. pcb is inserted at the end (pcbFree_h->p_prev)
    if (pcbFree_h == NULL) {
        pcbFree_h = p;
        p->p_next = p->p_prev = p;
    } else {
        p->p_next = pcbFree_h;
        p->p_prev = pcbFree_h->p_prev;
        pcbFree_h->p_prev = p;
        p->p_prev->p_next = p;
    }
}


/* PCB allocation and initialization*/

/*pcb_t *allocPcb() {
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
}*/

pcb_t *allocPcb() {
    if (pcbFree_h == NULL) {
        return NULL;
    } else {
        pcb_PTR toReturn = pcbFree_h;
        if (pcbFree_h != pcbFree_h->p_next) {
            pcbFree_h = pcbFree_h->p_next;
        } else {
            pcbFree_h = NULL;
        }

        unlinkPCB(toReturn);

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


/* Returns TRUE if p has no childrens, FALSE otherwise */

int emptyChild(pcb_t *p) {
    return p->p_child == NULL;
}

/* Insert p as child of prnt  */
/* function is O(n) because the list is required to be NULL terminated and we only have a head-pointer */
void insertChild(pcb_t *prnt, pcb_t *p) {
    p->p_prnt = prnt;
    p->p_next_sib = NULL;
    if (emptyChild(prnt)) {
        prnt->p_child = p;
        p->p_prev_sib = NULL;
    } else {
        pcb_t *iter = prnt->p_child;
        while (iter->p_next_sib != NULL) {
            iter = iter->p_next_sib;
        }
        iter->p_next_sib = p;
        p->p_prev_sib = iter;
    }

}

/* Removes first child of p and returns it ( return NULL otherwise ) */


pcb_t *removeChild(pcb_t *p) {
    if (emptyChild(p))
        return NULL;

    pcb_PTR child = p->p_child;
    p->p_child = child->p_next_sib;

    unlinkChild(child);

    return child;
}

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