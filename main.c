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
    pcb_PTR q;

    pcb_t *procp[MAXPROC];
    for (int i = 0; i < 10; i++) {
        if ((procp[i] = allocPcb()) == NULL)
            printf("allocPcb: unexpected NULL   ");
    }

    for (int i = 1; i < 10; i++) {
        insertChild(procp[0], procp[i]);
    }
    printf("Inserted 9 children   \n");

    pcb_t *iter = procp[0]->p_child;
    int i = 0;
    while (iter != NULL) {
        i++;
        iter = iter->p_next_sib;
    }
    printf("Numero childs:%d\n", 1 );

    if (emptyChild(procp[0]))
        printf("emptyChild: unexpected TRUE   ");

    q = outChild(procp[1]);
    if (q == NULL || q != procp[1])
        printf("outChild failed on first child   ");
    q = outChild(procp[4]);
    if (q == NULL || q != procp[4])
        printf("outChild failed on middle child   ");
    if (outChild(procp[0]) != NULL)
        printf("outChild failed on nonexistent child   ");
    printf("outChild ok   \n");

    if (emptyChild(procp[0]))
        printf("emptyChild: unexpected TRUE   ");

    iter = procp[0]->p_child;
    i = 0;
    while (iter != NULL) {
        i++;
        iter = iter->p_next_sib;
    }
    printf("Numero childs:%d\n", 1 );

    /* Check removeChild */
    printf("Removing...   \n");
    for (int i = 0; i < 7; i++) {
        if ((q = removeChild(procp[0])) == NULL)
            printf("removeChild: unexpected NULL   ");
    }
    if (removeChild(procp[0]) != NULL)
        printf("removeChild: removes too many children   ");

}

