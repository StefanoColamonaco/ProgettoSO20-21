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

    pcb_t *procp[MAXPROC];
    for (int i = 0; i < MAXPROC; i++) {
        if ((procp[i] = allocPcb()) == NULL)
            printf("allocPcb: unexpected NULL   ");
    }

    if (allocPcb() != NULL) {
        printf("errore non è null");
    }

}

