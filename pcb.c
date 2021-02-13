

#include "pcb.h"

/* Fills pcbFree_h list */
void initPcbs() {
    pcbFree_h = pcbFree_table[0];
    for (int i = 0; i < MAX_PROC; i++) {
        pcbFree_table[i%MAX_PROC].p_next = pcbFree_table[(i+1)%MAX_PROC];
        pcbFree_table[(i+1)%MAX_PROC].p_prev = pcbFree_table[i%MAX_PROC];
    }

}