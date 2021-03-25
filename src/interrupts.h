#ifndef PROGETTOSO20_21_INTERRUPTS_H
#define PROGETTOSO20_21_INTERRUPTS_H

#include "init.h"
#include "scheduler.h"

void handleInterrupts();

static void handlePLTInterrupt();

static void handleIntervalTimerInterrupt();

static void handleDeviceInterrupt(unsigned int interruptLine);

static unsigned int getDeviceNoFromLine(unsigned int interruptLine);

static inline void acknowledgeInterrupt(memaddr devBaseAddr);

static unsigned int getSemNumber(interruptLine, deviceNo);

static inline int terminalIsRECV(memaddr devAddrBase);

#endif