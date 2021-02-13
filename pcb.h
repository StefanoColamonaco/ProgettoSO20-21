#include "pandos_types.h"
#include "pandos_const.h"

/* Pointer to list of free pcbs*/
pcb_PTR pcbFree_h;

/*Table of free pcbs*/
pcb_t pcbFree_table[MAX_PROC];

/* Fills pcbFree_h list */
void initPcbs();

#include "pandos_const.h"
#include "pandos_types.h"
#include <stdio.h>

pcb_PTR pcbFree_h;               //unused PCBs list
pcb_t pcbFree_table[MAXPROC];    //PCB array

void initPcbs() {
    pcb_t *sentinel, *tmp;
    pcbFree_h = sentinel;
    *sentinel = pcbFree_table[0];
    *tmp = pcbFree_table[0];
    for (int i = 1; i<MAXPROC; i++) {
        *tmp->p_next = pcbFree_table[i];
        *tmp = pcbFree_table[i];
        *tmp->p_prev = pcbFree_table[i-1];
    }
    *tmp = pcbFree_table[MAXPROC-1];
    *tmp->p_next = pcbFree_table[0];
    *tmp = pcbFree_table[0];
    *tmp->p_prev = pcbFree_table[MAXPROC-1];
}
