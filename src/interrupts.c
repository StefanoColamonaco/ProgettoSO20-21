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

void handleInterrupts() {
    STCK(stopT);
    unsigned int cause = getCAUSE();
    int timeLeft = getTIMER();

    while ((cause & CAUSE_IP_MASK) != 0) {  //check for pending interrupts
        if (cause & CAUSE_IP(PLTINT)){
            handlePLTInterrupt();
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
    }
    if(currentProcess != NULL){
        //PANIC();
        currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
        copyStateInfo((state_t*) BIOSDATAPAGE, &(currentProcess -> p_s));
        prepareSwitch(currentProcess, timeLeft);
    }
}

void handlePLTInterrupt() {

}

void handleIntervalTimerInterrupt() {

}

void handleDeviceInterrupt(unsigned int interruptLine) {
    unsigned int deviceNo = getDeviceNoFromLine(interruptLine);
    unsigned int *devBase = DEV_REG_ADDR(interruptLine, deviceNo);
    unsigned int savedStatus = *devBase;
    acknowledgeInterrupt(devBase);
    SYSCALL(VERHOGEN, &deviceSemaphores[getSemNumber(interruptLine, deviceNo)], 0, 0);
    
}

unsigned int getDeviceNoFromLine(unsigned int interruptLine) {
    unsigned int *bitmap = (memaddr)CDEV_BITMAP_ADDR(interruptLine);
    for (int i = 0; i < DEVPERINT; i++) {
        if ((*bitmap & 1U) << i == ON)
            return i;
    }
}

static inline void acknowledgeInterrupt(unsigned int *devBase) {
    *(devBase + 0x4) = ACK;
}

static unsigned int getSemNumber(interruptLine, deviceNo) {
    switch (interruptLine) {
    case INTERTIMEINT:
        return DEVICE_NUM-1;    //TODO REPLACE WITH MACRO
    
    case DISKINT:
    case FLASHINT:
    case NETWINT:
    case PRNTINT:
        return (interruptLine - 3)*8 + deviceNo;

    case TERMINT:
        unsigned int devAddrBase = DEV_REG_ADDR(interruptLine, deviceNo);
        if (terminalIsRECV(devAddrBase))
            return (interruptLine - 3)*8 + deviceNo;
        else 
            return (interruptLine - 2)*8 + deviceNo;
    }
}

static inline int terminalIsRECV(unsigned int *devBase) {
    return *devBase != READY;
}