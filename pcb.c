#include "pcb.h"

pcb_PTR pcbFree_h;

pcb_t pcbFree_table[MAXPROC];


/* Fills pcbFree_h list */

void initPcbs() {
    pcbFree_h = &pcbFree_table[0];
    for (int i = 0; i < MAXPROC; i++) {
        pcbFree_table[i%MAXPROC].p_next = &pcbFree_table[(i+1)%MAXPROC];
        pcbFree_table[(i+1)%MAXPROC].p_prev = &pcbFree_table[i%MAXPROC];
    }
}