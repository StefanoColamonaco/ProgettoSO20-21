#ifndef PROGETTOSO20_21_PCB_H
#define PROGETTOSO20_21_PCB_H


#include "pandos_types.h"
#include "pandos_const.h"


/* Pointer to list of free pcbs*/
extern pcb_PTR pcbFree_h;


/*Table of free pcbs*/
extern pcb_t pcbFree_table[MAXPROC];



/* Fills pcbFree_h list */
void initPcbs();


/*
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
*/


#endif //PROGETTOSO20_21_PCB_H
