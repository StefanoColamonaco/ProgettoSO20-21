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
    pcb_PTR tp = mkEmptyProcQ();


    insertProcQ(&tp, p);
    insertProcQ(&tp, p1);
    insertProcQ(&tp, p2);
/*
    if (outProcQ(&tp, p1) == p1)
        printf("outProcQ ha cancellato p1\n");

    if (headProcQ(tp) == p)
        printf("p è l'head\n");

    if (removeProcQ(&tp) == p)
        printf("p rimosso correttamente\n");
    else
        printf("processo sbagliato rimosso\n");

    if (headProcQ(tp) == p1) {
        printf("p1 è l'head\n");
    }

    if (removeProcQ(&tp) == p2)
        printf("p2 rimosso correttamente\n");
    else
        printf("processo sbagliato rimosso\n");

    if (outProcQ(&tp, p1) == p1)
        printf("outProcQ ha cancellato p1\n");
    else 
        printf("Non è stato possibile cancellare p1 poichè non presente nella lista\n");

    if (emptyProcQ(tp)) {
        printf("Queue is empty\n");
    } else {
        printf("Queue NOT empty\n");
    }
*/
    if (removeProcQ(&tp) == p)
        printf("p rimosso correttamente\n");
    else
        printf("processo sbagliato rimosso\n");
    
    if (removeProcQ(&tp) == p1)
        printf("p1 rimosso correttamente\n");
    else
        printf("processo sbagliato rimosso\n");

    if (removeProcQ(&tp) == p2)
        printf("p2 rimosso correttamente\n");
    else
        printf("processo sbagliato rimosso\n");
        
     if (emptyProcQ(tp)) {
        printf("Queue is empty\n");
    } else {
        printf("Queue NOT empty\n");
    }


}

