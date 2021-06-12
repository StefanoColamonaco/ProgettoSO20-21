#include <umps3/umps/cp0.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>

#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "stateUtil.h"
#include "pcb.h"
#include "asl.h"

#define NUCLEUS_STACKPAGE_TOP 0x20001000
#define clockSemaphore deviceSemaphores[DEVICE_NUM-1] //only for convenience

int processCount = 0;           //number of started but not yet terminated processes
int softBlockedCount = 0;       //number of processes blocked due to I/O
pcb_t *readyQueue = NULL;       //tail pointer to queue of ready processes
pcb_t *currentProcess;          //pointer to pcb that is in running state
cpu_t globalStartT;
cpu_t startT;
int deviceSemaphores[DEVICE_NUM];   //last device is interval time


/* Support level semaphores */
semd_t printerSemaphores[N_DEV_PER_IL];
semd_t termWriteSemaphores[N_DEV_PER_IL];
semd_t termReadSemaphores[N_DEV_PER_IL];


static inline void initDevSemaphores();

static passupvector_t *initPassupVector();

static pcb_t *initFirstProcess();

static inline void loadIntervalTimer (unsigned int timeInMicroSecs);

extern void uTLB_RefillHandler();

int main() {
    initPassupVector();  
    initPcbs();
    initASL();

    currentProcess = NULL;
    readyQueue = mkEmptyProcQ();
    clockSemaphore = 0;

    loadIntervalTimer(100000);
    initFirstProcess();
    scheduler();
    return 0;
}

pcb_t *initFirstProcess() {
    pcb_t *firstProcess = allocPcb();
    if(firstProcess != NULL) {
        state_t *state = &(firstProcess->p_s);
        state->reg_t9 = (memaddr)test_phase_3;
        state->pc_epc = state->reg_t9;                                 //set PC to test function
        firstProcess->p_s.status = ALLOFF | IECON | IMON | TEBITON;    //set status
        memaddr ramTop;
        RAMTOP(ramTop);
        state->reg_sp = ramTop;                                        

        firstProcess->p_time = 0;
        firstProcess->p_semAdd = NULL;
        firstProcess->p_supportStruct = NULL;

        insertProcQ(&readyQueue, firstProcess);
        
        return firstProcess;
    } else {
        PANIC();
    }    
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
    for (int i = 0; i < (N_DEV_PER_IL); i++){
        printerSemaphores[i].s_next = NULL;
        termWriteSemaphores[i].s_next = NULL;
        termReadSemaphores[i].s_next = NULL;

        printerSemaphores[i].s_procQ = mkEmptyProcQ();
        termWriteSemaphores[i].s_procQ = mkEmptyProcQ();
        termReadSemaphores[i].s_procQ = mkEmptyProcQ();

        printerSemaphores[i].s_semAdd = 0;
        termWriteSemaphores[i].s_semAdd = 0;
        termReadSemaphores[i].s_semAdd = 0;
    }

}