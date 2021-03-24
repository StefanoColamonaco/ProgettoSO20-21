#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

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
        copyStateInfo(((state_t*) BIOSDATAPAGE), &(currentProcess -> p_s));
        prepareSwitch(currentProcess, timeLeft);
    }
}

void handlePLTInterrupt() {

}

void handleIntervalTimerInterrupt() {

}

void handleDeviceInterrupt(unsigned int interruptLine) {
    unsigned int deviceNo = getDeviceNo(interruptLine);
    unsigned int devAddrBase = DEV_REG_ADDR(interruptLine, deviceNo);
    unsigned int savedStatus = *devAddrBase;
    acknowledgeInterrupt(devAddrBase);
    verhogen();

}

unsigned int getDeviceNo(unsigned int interruptLine) {
    unsigned int *bitmap = (memaddr)CDEV_BITMAP_ADDR(interruptLine);
    for (int i = 0; i < DEVPERINT; i++) {
        if (*bitmap & 1U << i)
            return i;
    }
}

static inline void acknowledgeInterrupt(memaddr devBaseAddr) {
    *(devBaseAddr + 0x4) = 1;
}
