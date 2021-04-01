#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

#include "stateUtil.h"
#include "interrupts.h"
#include "systemCalls.h"

#define PLTINT 1
#define INTERTIMEINT 2

cpu_t stopT;

static void handlePLTInterrupt();

static void handleIntervalTimerInterrupt();

static void handleDeviceInterrupt(unsigned int interruptLine);

static unsigned int getDeviceNoFromLine(unsigned int interruptLine);

static inline void acknowledgeInterrupt(unsigned int *devBase);

static inline int terminalIsRECV(unsigned int *devBase);

static inline int terminalIsTRANSM(unsigned int *devBase);

unsigned int devBaseTest;

void handleInterrupts() {
    devBaseTest = DEV_REG_ADDR(7, getDeviceNoFromLine(7));
    STCK(stopT);
    unsigned int cause = getCAUSE();
    int timeLeft = getTIMER();

    while ((cause & CAUSE_IP_MASK) != 0) {  //check for pending interrupts
        if (cause & CAUSE_IP(PLTINT)){
            handlePLTInterrupt(stopT);
        }

        else if (cause & CAUSE_IP(INTERTIMEINT)){
            handleIntervalTimerInterrupt();
        }

        else if (cause & CAUSE_IP(DISKINT)) {
            handleDeviceInterrupt(DISKINT);
        }

        else if (cause & CAUSE_IP(FLASHINT)) {
            handleDeviceInterrupt(FLASHINT);
        }

        else if (cause & CAUSE_IP(NETWINT)) {
            handleDeviceInterrupt(NETWINT);
        }

        else if (cause & CAUSE_IP(PRNTINT)) {
            handleDeviceInterrupt(PRNTINT);
        }

        else if (cause & CAUSE_IP(TERMINT)) {
            handleDeviceInterrupt(TERMINT);
        }
        cause = getCAUSE();
    }
    if(currentProcess != NULL){
        currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
        copyStateInfo((state_t*) BIOSDATAPAGE, &(currentProcess -> p_s));
        prepareSwitch(currentProcess, timeLeft);
    }
}

void handlePLTInterrupt(int stopT) {
    if(currentProcess != NULL){
        currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
        copyStateInfo(((state_t*)BIOSDATAPAGE), &(currentProcess -> p_s));
        insertProcQ(&readyQueue, currentProcess);
        scheduler();
    } else {
        PANIC();
    }
}

void handleIntervalTimerInterrupt() {
    LDIT(100000);
    pcb_t *tmp = removeBlocked(&clockSemaphore);
    while(tmp != NULL){
        insertProcQ(&readyQueue, tmp);
        softBlockedCount++;
        tmp = removeBlocked(&clockSemaphore);
    }
    clockSemaphore = 0;
    if(currentProcess == NULL){
        scheduler();
    }
}

void handleDeviceInterrupt(unsigned int interruptLine) {
    unsigned int deviceNo = getDeviceNoFromLine(interruptLine);
    unsigned int *devBase = DEV_REG_ADDR(interruptLine, deviceNo);
    int termIsTRANSM = 0;
    if (interruptLine == TERMINT && terminalIsTRANSM(devBase)) {
        termIsTRANSM = 1;
        devBase += 0x8U;
    }
    unsigned int savedStatus = *devBase;
    acknowledgeInterrupt(devBase);
    releaseSemAssociatedToDevice(getSemNumber(interruptLine, deviceNo, termIsTRANSM), savedStatus);
    startT = getTIMER();
    if(currentProcess == NULL){
        scheduler();
    } else {
        contextSwitch(currentProcess);
    }
}

unsigned int getDeviceNoFromLine(unsigned int interruptLine) {
    unsigned int *bitmap = (memaddr)CDEV_BITMAP_ADDR(interruptLine);
    for (int i = 0; i < DEVPERINT; i++) {
        if ((*bitmap >> i) & 1U == ON)
            return i;
    }
}

/* is a V operation on the semaphore associated to the device number */
void releaseSemAssociatedToDevice(int deviceNo, unsigned int status) {
    softBlockedCount--;
    if(deviceSemaphores[deviceNo] <= 0){
        pcb_t *tmp = removeBlocked(&deviceSemaphores[deviceNo]);
        if(tmp != NULL){
            tmp -> p_s.reg_v0 = status;
            insertProcQ(&readyQueue, tmp);
        }
    }
}

static void acknowledgeInterrupt(unsigned int *devBase) {
    *(devBase + 0x4U) = ACK;
}

unsigned int getSemNumber(unsigned int interruptLine, unsigned int deviceNo, int termIsTRANSM) {
    switch (interruptLine) {
    case INTERTIMEINT:
        return DEVICE_NUM-1;    //TODO REPLACE WITH MACRO
    
    case DISKINT:
    case FLASHINT:
    case NETWINT:
    case PRNTINT:
        return (interruptLine - DISKINT)*8 + deviceNo;

    case TERMINT:
        if (termIsTRANSM)
            return (interruptLine - DISKINT + 1)*8 + deviceNo;
        else 
            return (interruptLine - DISKINT)*8 + deviceNo;
            
        //TODO handle default case
    }
}


static int terminalIsRECV(unsigned int *devBase) {
    return *devBase != READY;
}

static int terminalIsTRANSM(unsigned int *devBase) {
    return *(devBase + 0x8U) != READY;
}

void flashInterrupts(int lineNum){

}

void networkInterrupts(int lineNum){

}

void printerInterrupts(int lineNum){

}

void terminalInterrupts(int lineNum){

}