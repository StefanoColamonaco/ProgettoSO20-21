#include "asl.h"
#include "pcb.h"

#define MAXADD 0x7FFFFFFF

/*Table of free semaphores*/
static semd_t semd_table[MAXPROC+2];

/* Pointer to list of free semaphores*/
static semd_PTR semdFree_h;

/* Pointer to list of active semaphores */
static semd_PTR semd_h;


/* Fills semdFree_h list and initialize active semaphores list */

void initASL(){

    semdFree_h = NULL;

    /* Definition for lower and upper limits for semd_h */
    semd_t *lowerLimit = &(semd_table[0]);                  //Sentinel for lower limit
    semd_t *upperLimit = &(semd_table[1]);                  //Sentinel for upper limit
    lowerLimit -> s_next = upperLimit;
    lowerLimit -> s_procQ = mkEmptyProcQ();
    lowerLimit -> s_semAdd = 0;
    upperLimit -> s_next = NULL;
    upperLimit -> s_procQ = mkEmptyProcQ();
    upperLimit -> s_semAdd = (int*)MAXADD;         //0x7FFFFFFF is a bitmask for 'max'

    semd_h = lowerLimit;                                    //semd_h initialization

    /* Fills semdFree_h list */
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

/* Utility function to find semd_t value associated to the element before semAdd */

semd_t *findSemInActiveList(int *semAdd){
  semd_t *tmp = semd_h;
  
  while(tmp -> s_next -> s_semAdd < semAdd && tmp -> s_next -> s_semAdd != (int*)MAXADD)
      tmp = tmp -> s_next;
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

/* Returns an element to the free list */

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

/*
    Insert p into the queue of blocked processes associated to semAdd
    Return TRUE if semdFree_h is empty, FALSE otherwise
*/

int insertBlocked(int *semAdd, pcb_t *p){
    semd_t *tmp = findSemInActiveList(semAdd);

    if(tmp -> s_next -> s_semAdd == semAdd){    //Semaphore is already allocated, we only need to insert p in proc queue
        p -> p_semAdd = semAdd;
        insertProcQ(&tmp -> s_next -> s_procQ, p);
        return FALSE;
    } else {                                   //We need to allocate a semaphore
        semd_t *newSem = newSemd();
        if(newSem == NULL)
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
    if(tmp -> s_next -> s_semAdd == semAdd){
        pcb_t *removed = removeProcQ(&tmp -> s_next -> s_procQ);
        if(emptyProcQ(tmp -> s_next -> s_procQ)){                 //If the process queue is empty the semaphore is freed
            semd_t *tmp_emptyProcQueue = tmp -> s_next;
            tmp -> s_next = tmp_emptyProcQueue -> s_next;
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
    pcb_t *releasedPcb = outProcQ(&tmp -> s_next -> s_procQ, p);
    
    if(releasedPcb == NULL)
      return NULL;

    if(emptyProcQ(tmp -> s_next -> s_procQ)){                    //If the process queue is empty the semaphore is freed
      semd_t *tmp_emptyProcQueue = tmp -> s_next;
      tmp -> s_next = tmp_emptyProcQueue -> s_next;
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

    if( emptyProcQ(tmp -> s_next -> s_procQ ) || tmp == NULL)             //Non-mandatory security measure
        return NULL;

    if( tmp -> s_next -> s_semAdd != semAdd )               //SemAdd is not in semd_h
        return NULL;

    return headProcQ(tmp -> s_next -> s_procQ);
}

