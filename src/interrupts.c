#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>

#include "interrupts.h"

#define PROCESS_LOCAL_TIMER 1
#define INTERVAL_TIMER 2


void handleInterrupts() {
    unsigned int cause = getCAUSE();

    while ((cause & CAUSE_IP_MASK) != 0) {
        if (cause & CAUSE_IP(PROCESS_LOCAL_TIMER)){
            handlePLTInterrupt();
        }

        else if (cause & CAUSE_IP(INT)){
            handleIntervalTimerInterrupt();
        }

        else if (cause & CAUSE_IP(DISKINT)) {
            handleDeviceInterrupt(DISKINT);
        }

        else if (cause & CAUSE_IP(FLASHINT)) {
            handleDeviceInterrupt(FLASHINT);
        }

        else if (cause & CAUSE_IP(NETWINT)) {
            handleDeviceInterrupt(NETWINT)
        }

        else if (cause & CAUSE_IP(PRNTINT)) {
            handleDeviceInterrupt(PRNTINT)
        }

        else if (cause & CAUSE_IP(TERMINT)) {
            handleDeviceInterrupt(TERMINT)
        }
    }
}

void handlePLTInterrupt() {

}

void handleIntervalTimerInterrupt() {

}

void handleDeviceInterrupt(unsigned int interruptLine) {
    unsigned int deviceNo = getDeviceNo(interruptLine);
    unsigned int debAddrBase = computeDevAddrBase(interruptLine, deviceNo);
}

unsigned int getDeviceNo(unsigned int interruptLine) {
    switch (interruptLine) {

    }
}

unsigned int computeDevAddrBase(int interruptLine, int deviceNo) {
    return 0x10000054 + ((interruptLine - 3) * 0x80) + (deviceNo * 0x10)
}