#include <umps3/umps/cp0.h>
#include <umps3/umps/types.h>

#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "stateUtil.h"
#include "pcb.h"
#include "asl.h"

#include <umps3/umps/libumps.h>

#define NUCLEUS_STACKPAGE_TOP 0x20001000
#define clockSemaphore deviceSemaphores[DEVICE_NUM-1] //only for convenience

int processCount = 0;           //number of started but not yet terminated processes
int softBlockedCount = 0;       //number of processes blocked due to I/O
pcb_t *readyQueue = NULL;       //tail pointer to queue of ready processes
pcb_t *currentProcess;          //pointer to pcb that is in running state
cpu_t startT;
int deviceSemaphores[DEVICE_NUM];   //starting from line 3 up to 6. then 8 devs for term_receive and 8 more for term_transmit (line 7). Last device is interval time

static inline void initDevSemaphores();
static passupvector_t *initPassupVector();
static inline void loadIntervalTimer (unsigned int timeInMicroSecs);
extern void uTLB_RefillHandler();

int main() {

    initPassupVector();  
    
    initPcbs();
    initASL();

    currentProcess = NULL;
    readyQueue = mkEmptyProcQ();
    clockSemaphore = 0;

    initDevSemaphores();
    loadIntervalTimer(100000);

    /*first process initialization*/
    memaddr ramTop;
    RAMTOP(ramTop);
    pcb_t *firstProcess = allocPcb();
    if(firstProcess != NULL) {
        state_t *state = &(firstProcess->p_s);
        state->reg_t9 = (memaddr)test;
        state->pc_epc = state->reg_t9;                                 //set PC to test function
        firstProcess->p_s.status = ALLOFF | IECON | IMON | TEBITON;    //set status
        state->reg_sp = ramTop;                                        //RAMTOP's side effect sets the stackpointer

        firstProcess->p_time = 0;
        firstProcess->p_semAdd = NULL;
        firstProcess->p_supportStruct = NULL;

        insertProcQ(&readyQueue, firstProcess);
        
        scheduler();
    }else{
        PANIC();
    }    
    return 0;
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

static inline void initDevSemaphores() {
    for (int i = 0; i < (DEVICE_NUM); i++){
        deviceSemaphores[i] = 0;
    }
}