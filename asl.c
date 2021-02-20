#include "asl.h"
#include "pcb.h"

/*
Nota implementativa per la "relazione" siccome il valore vero e proprio del semaforo è un puntatore ad intero pr effettuare 
una ricerca all'interndo della lista di semafori attivi dobbiamo basarci su questo.
Per semplificare le operazioni di ricerca quindi è opportuno procedere in ordine di "dimensione" dell'indirizzo grazie al fatto che 
la definzione della sem table è quella di un array e quindi avviene in celle di memoria contigue.
Proprio per questo motivo è opportuno aumentare il numero di semafori nella table per averne due che fungano da sentinella, 
rispettivamente con valore 0 e 0x7FFFFFFF (bitmask per il max) 
*/


semd_t semd_table[MAXPROC+2];

semd_PTR semdFree_h;

semd_PTR semd_h;

void initASL(){

    semdFree_h = NULL;

    semd_t *lowerLimit = &(semd_table[0]);        //sentinel for lower limit
    semd_t *upperLimit = &(semd_table[1]);        //sentinel for upper limit
    lowerLimit -> s_next = upperLimit;
    lowerLimit -> s_procQ = mkEmptyProcQ();
    lowerLimit -> s_semAdd = 0;
    upperLimit -> s_next = NULL;
    upperLimit -> s_procQ = mkEmptyProcQ();
    upperLimit -> s_semAdd = 0x7FFFFFFF;         //0x7FFFFFFF is a bitmask for 'max'

    semd_h = lowerLimit;

    for(int i = 2; i < MAXPROC+2; i++){
        semd_t *tmp = semdFree_h;
        if(semdFree_h == NULL){
            semdFree_h = &semd_table[i];
            semdFree_h -> s_next = NULL;
        } else {
            semdFree_h = &semd_table[i];
            semdFree_h -> s_next = tmp;
        }
    }
}

/* Utility function to find semd_t value associated to the element before semAdd*/

semd_t *findSemInActiveList(int *semAdd){
  semd_t *tmp = semd_h;
  
  while(tmp -> s_next -> s_semAdd < semAdd) tmp = tmp -> s_next;
  return tmp;
}

/* Take a semaphore from semFree list, initializes it and returns it */

semd_t* newSemd() {
    if( semdFree_h == NULL ) {
        return NULL;
    } else {
        semd_t *tmp = semdFree_h;
        semdFree_h = tmp -> s_next;
        tmp -> s_next = NULL;
        tmp -> s_procQ = mkEmptyProcQ();
        tmp -> s_semAdd = NULL;
        return tmp; 
    }
}

/* returns an element to the free list */

void freeSemd(semd_t *semd){
    semd_t *tmp = semdFree_h;
    if(semdFree_h == NULL){
        semdFree_h = semd;
        semdFree_h -> s_next = NULL;
    } else {
        semdFree_h = semd;
        semdFree_h -> s_next = tmp;
    }
}

/* Insert p into the queue of blocked processes associated to semAdd */
// return TRUE if semdFree_h is empty, FALSE otherwise

int insertBlocked(int *semAdd, pcb_t *p){
    semd_t *tmp = findSemInActiveList(semAdd);

    if(tmp -> s_next -> s_semAdd == semAdd){    //semaphore is already allocated, we only need to insert p in proc queue
        p -> p_semAdd = semAdd;
        insertProcQ(&tmp -> s_next -> s_procQ, p);
        return FALSE;
    } else {                                   //we need to allocate a semaphore
        semd_t *newSem = newSemd();
        if((newSem == NULL))                  
            return TRUE;

        semd_t *tmp_allocated = findSemInActiveList(semAdd);
        p -> p_semAdd = semAdd;
        newSem -> s_semAdd = semAdd;
        newSem -> s_procQ = mkEmptyProcQ();
        newSem -> s_next = tmp_allocated -> s_next;
        tmp_allocated -> s_next = newSem;
        insertProcQ(&newSem -> s_procQ, p);
        return FALSE;
    }
}

/* Returns the first PCB from the queue of blocked processes */

pcb_t *removeBlocked(int *semAdd){
    semd_t *tmp = findSemInActiveList(semAdd);
    tmp = tmp -> s_next;
    if(tmp -> s_semAdd == semAdd){
        pcb_t *removed = removeProcQ(&tmp -> s_procQ);
        if(emptyProcQ(tmp -> s_procQ)){                 //if the process queue is empty the semaphore is freed
            semd_t *tmp_emptyProcQueue = tmp;
            tmp = tmp_emptyProcQueue -> s_next;
            tmp_emptyProcQueue -> s_next = NULL;
            tmp_emptyProcQueue -> s_semAdd = NULL;
            tmp_emptyProcQueue -> s_procQ = mkEmptyProcQ();
            freeSemd(tmp_emptyProcQueue);
        }
        return removed;
    } else return NULL;
}

/* Remove p from the queue where it is blocked ( returns NULL otherwise ) */

pcb_t *outBlocked(pcb_t *p){
    semd_t *tmp = findSemInActiveList(p -> p_semAdd);
    tmp = tmp -> s_next;
    pcb_t *releasedPcb = outProcQ(&tmp -> s_procQ, p);
    
    if(releasedPcb == NULL)
      return NULL;

    if(emptyProcQ(tmp -> s_procQ)){                    //if the process queue is empty the semaphore is freed
      semd_t *tmp_emptyProcQueue = tmp;
      tmp = tmp_emptyProcQueue -> s_next;
      tmp_emptyProcQueue -> s_next = NULL;
      tmp_emptyProcQueue -> s_semAdd = NULL;
      tmp_emptyProcQueue -> s_procQ = mkEmptyProcQ();
      freeSemd(tmp_emptyProcQueue);
    }
    return releasedPcb;
}












/* Returns the PCB at the head of semAdd's queue */

pcb_t *headBlocked(int *semAdd){
    semd_t *tmp = findSemInActiveList(semAdd);

    if( tmp == NULL)
        return NULL;

    if( emptyProcQ(tmp -> s_next -> s_procQ ) )
        return NULL;

    return headProcQ(tmp -> s_next -> s_procQ);
}