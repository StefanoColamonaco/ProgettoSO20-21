#include "init.h"
#include "exceptions.h"

#define NUCLEUS_STACKPAGE_TOP 0x20001000


int processCount = 0;        //number of started but not yet terminated processes

int softBlockCount = 0;      //number of processes blocked due to I/O

pcb_t *readyQueue = mkEmptyProcQ();       //tail pointer to queue of ready processes

pcb_t *currentProcess = NULL;   //pointer to pcb that is in running state

//todo populate pass-up vector

int main() {
    initPcbs();
    initASL();

    passupvector_t *passupvector = (*passupvector)PASSUPVECTOR;
    passupvector->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
    passupvector->tlb_refill_stackPtr = NUCLEUS_STACKPAGE_TOP;
    passupvector->exception_handler = (memaddr)handleExceptions();




}