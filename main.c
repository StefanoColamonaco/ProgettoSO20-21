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
    pcb_PTR p1 = allocPcb();
    pcb_PTR p2 = allocPcb();
    pcb_PTR p3 = allocPcb();
    //pcb_PTR tp = mkEmptyProcQ();


    insertChild(p, p1);
    insertChild(p, p2);
    insertChild(p, p3);


    if (outChild(p1) == p1) {
        printf("out child corretto p1\n");
    }

    if (outChild(p2) == p2) {
        printf("rimosso child corretto p2\n");
    }

    if (removeChild(p) == p3) {
        printf("remove child p3 corretto \n");
    }

    if (emptyChild(p)) {
        printf("empty child\n");
    }


}

