//
// Created by dreac on 14/02/21.
//

#include <stdlib.h>
#include <stdio.h>

#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"

int main() {
    initPcbs();
    pcb_PTR p = allocPcb();

    pcb_PTR iter = pcbFree_h;
    int i = 0;
    do {
        i++;
        iter = iter->p_next;
    } while (iter != pcbFree_h);
    printf("Available processes: %d", i);

    freePcb(p);
    i = 0;
    do {
        i++;
        iter = iter->p_next;
    } while (iter != pcbFree_h);
    printf("Available processes: %d", i);
}

