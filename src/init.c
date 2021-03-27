#include <umps3/umps/cp0.h>
#include <umps3/umps/types.h>

#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "stateUtil.h"
#include "pcb.h"
#include "asl.h"

#define NUCLEUS_STACKPAGE_TOP 0x20001000
#define clockSemaphore deviceSemaphores[DEVICE_NUM-1]       //TODO refactor

int processCount = 0;        //number of started but not yet terminated processes
int softBlockedCount = 0;      //number of processes blocked due to I/O
pcb_t *readyQueue = NULL;       //tail pointer to queue of ready processes
pcb_t *currentProcess = NULL;   //pointer to pcb that is in running state
cpu_t startT;
int deviceStat[DEVICE_NUM];
int deviceSemaphores[DEVICE_NUM];   //starting from line 3 up to 6. then 8 devs for term_receive and 8 more for term_transmit (line 7). Last device is interval timer

static inline void initDevSemaphores();
static passupvector_t *initPassupVector();
static inline void loadIntervalTimer (unsigned int timeInMicroSecs);
static pcb_t *initFirstProcess();

extern void uTLB_RefillHandler();

int main() {
    readyQueue = mkEmptyProcQ();
    initPcbs();
    initASL();
    initDevSemaphores();
    initPassupVector();      
    initFirstProcess();
    loadIntervalTimer(100000);
    scheduler();
}

passupvector_t *initPassupVector() {
    passupvector_t *toReturn = (passupvector_t*)PASSUPVECTOR;
    toReturn->tlb_refill_handler = (memaddr)uTLB_RefillHandler; 
    toReturn->tlb_refill_stackPtr = NUCLEUS_STACKPAGE_TOP;
    toReturn->exception_handler = (memaddr)handleExceptions;
    toReturn->exception_stackPtr = NUCLEUS_STACKPAGE_TOP;
    return toReturn;
}

inline void loadIntervalTimer (unsigned int timeInMicroSecs) {
    LDIT(timeInMicroSecs);
}

pcb_t *initFirstProcess() {
    processCount++;
    pcb_t *firstProcess = allocPcb();
    state_t *state = &(firstProcess->p_s);
    //todo consider refactoring for clarity
    setStatusBitToValue(&state->status, STATUS_IEp_BIT, 0); //disables interrupts
    setStatusBitToValue(&state->status, STATUS_TE_BIT, 1); //enable local timer
    setStatusBitToValue(&state->status, STATUS_KUp_BIT, 0); //kernel mode
    state->pc_epc = (memaddr)test; //set PC to test function
    RAMTOP(state->reg_sp);    //RAMTOP's side effect sets the stackpointer

    firstProcess->p_time = 0;
    firstProcess->p_semAdd = NULL;
    firstProcess->p_supportStruct = NULL;

    return firstProcess;
}

static inline void initDevSemaphores() {
    for (int i = 0; i < (DEVICE_NUM); i++){
        deviceSemaphores[i] = 0;
    }
}