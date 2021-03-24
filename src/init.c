#include <umps3/umps/cp0.h>
#include <umps3/umps/types.h>

#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "stateUtil.h"
#include "p2test.c"

#define NUCLEUS_STACKPAGE_TOP 0x20001000

int processCount = 0;        //number of started but not yet terminated processes

int softBlockCount = 0;      //number of processes blocked due to I/O

pcb_t *readyQueue = mkEmptyProcQ();       //tail pointer to queue of ready processes

pcb_t *currentProcess = NULL;   //pointer to pcb that is in running state

cpu_t startT;

int deviceStat[DEVICE_NUM];

int deviceSemaphores[DEVICE_NUM];

#define clockSemaphore deviceSemaphores[DEVICE_NUM-1]

int main() {
    initPcbs();
    initASL();
    initDevSemaphores();
    passupvector_t *passupVector = initPassupVector();      //todo rename passupVector to avoid ambiguity
    initFirstProcess();
    loadIntervalTimer(100000);
    scheduler();
}

passupvector_t *initPassupVector() {
    passupvector_t *toReturn = (passupvector_t*)PASSUPVECTOR;
    toReturn->(memaddr)tlb_refill_handler = (memaddr)uTLB_RefillHandler;    //todo check this for correct typing
    toReturn->tlb_refill_stackPtr = NUCLEUS_STACKPAGE_TOP;
    toReturn->exception_handler = (memaddr)handleExceptions();
    toReturn->exception_stackPtr = NUCLEUS_STACKPAGE_TOP;
    return toReturn;
}

inline void loadIntervalTimer (unsigned int timeInMicroSecs) {
    LDIT(timeInMicroSecs);
}

pcb_t *initFirstProcess() {
    processCount++;
    pcb_t *firstProcess = allocPcb();
    state_t *state = firstProcess->p_s;
    //todo consider refactoring for clarity
    setStatusBitToValue(state, STATUS_IEp_BIT, 0); //disables interrupts
    setStatusBitToValue(state, STATUS_TE_BIT, 1); //enable local timer
    setStatusBitToValue(state, STATUS_KUp_BIT, 0); //kernel mode
    firstProcess->p_s.(memaddr)pc_epc = (memaddr)test; //set PC to test function
    RAMTOP(firstProcess->p_s.reg_sp);    //side effect also sets the stackpointer

    firstProcess->p_time = 0;
    firstProcess->p_semAdd = NULL;
    firstProcess->p_supportStruct = NULL;

    return firstProcess;
}

void initDevSemaphores() {
    for (int i = 0; i < (DEVICE_NUM); i++){
        deviceSemaphores[i] = 0;
    }
}