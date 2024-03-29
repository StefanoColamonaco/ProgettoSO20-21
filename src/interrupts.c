#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include <umps3/umps/arch.h>

#include "init.h"
#include "pcb.h"
#include "asl.h"
#include "scheduler.h"
#include "stateUtil.h"
#include "interrupts.h"
#include "nucleousSystemCalls.h"

#define PLTINT 1
#define INTERTIMEINT 2


cpu_t stopT;

static void handlePLTInterrupt();

static void handleIntervalTimerInterrupt();

static void handleDeviceInterrupt(unsigned int interruptLine);

static unsigned int getDeviceNoFromLine(unsigned int interruptLine);

static inline void acknowledgeDTPInterrupt(dtpreg_t *dev);

static inline void acknowledgeTermInterrupt(termreg_t *dev);

static inline int terminalIsRECV(termreg_t *dev);

static inline unsigned int getTermStatus(termreg_t*dev);



void handleInterrupts()
{
    int timeLeft = getTIMER();
    unsigned int cause = getCAUSE();
    STCK(stopT);

    if ((cause & CAUSE_IP_MASK) != 0) {  //check for pending interrupts

        if (cause & CAUSE_IP(PLTINT)) {
            handlePLTInterrupt(stopT);
        } else if (cause & CAUSE_IP(INTERTIMEINT)) {
            handleIntervalTimerInterrupt();
        } else if (cause & CAUSE_IP(DISKINT)) {
            handleDeviceInterrupt(DISKINT);
        } else if (cause & CAUSE_IP(FLASHINT)) {
            handleDeviceInterrupt(FLASHINT);
        } else if (cause & CAUSE_IP(NETWINT)) {
            handleDeviceInterrupt(NETWINT);
        } else if (cause & CAUSE_IP(PRNTINT)) {
            handleDeviceInterrupt(PRNTINT);
        } else if (cause & CAUSE_IP(TERMINT)) {
            handleDeviceInterrupt(TERMINT);
        }
    }

    if (currentProcess != NULL) {
        currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
        copyState((state_t*) BIOSDATAPAGE, &(currentProcess -> p_s));
        updateTimeAndSwitch(currentProcess, timeLeft);
    } else {
        scheduler();
    }
}

//TODO refactor repeated code
void handlePLTInterrupt(int stopT)
{
    setTIMER(5000 * (*((cpu_t *)TIMESCALEADDR)));
    if (currentProcess != NULL) {
        currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
        copyState((state_t*)BIOSDATAPAGE, &(currentProcess -> p_s));
        insertProcQ(&readyQueue, currentProcess);
        scheduler();
    } else {
        PANIC();
    }
}

void handleIntervalTimerInterrupt()
{
    LDIT(100000);
    pcb_t *tmp = removeBlocked(&clockSemaphore);

    while (tmp != NULL) {
        insertProcQ(&readyQueue, tmp);
        softBlockedCount--;
        tmp = removeBlocked(&clockSemaphore);
    }

    clockSemaphore = 0;

}


void handleDeviceInterrupt(unsigned int interruptLine)
{
    unsigned int deviceNo = getDeviceNoFromLine(interruptLine);
    devreg_t *dev = (devreg_t*)DEV_REG_ADDR(interruptLine, deviceNo);
    unsigned int savedStatus = 0;
    int recv = 0;

    if (interruptLine == TERMINT) {
        recv = terminalIsRECV(&dev->term);
        savedStatus = getTermStatus(&dev->term);
        acknowledgeTermInterrupt(&dev->term);
    } else {
        savedStatus = dev->dtp.status;
        acknowledgeDTPInterrupt(&dev->dtp);
    }

    int semIndex = getSemIndex(interruptLine, deviceNo, recv);
    releaseSemAndUpdateStatus(semIndex, savedStatus);

    startT = getTIMER();
}

unsigned int getDeviceNoFromLine(unsigned int interruptLine)
{
    unsigned int *bitmap = (unsigned int*)CDEV_BITMAP_ADDR(interruptLine);

    for (int i = 0; i < DEVPERINT; i++) {
        if ( ((*bitmap >> i) & 1U) == ON ) {
            return i;
        }
    }

    return -1;
}


/*V operation on the semaphore associated to the device number */
void releaseSemAndUpdateStatus(int deviceNo, unsigned int status)
{
    softBlockedCount--;

    if (deviceSemaphores[deviceNo] <= 0) {
        pcb_t *tmp = removeBlocked(&deviceSemaphores[deviceNo]);

        if (tmp != NULL) {
            tmp -> p_s.reg_v0 = status;
            insertProcQ(&readyQueue, tmp);
            deviceSemaphores[deviceNo]++;
        }
    }
}


static void acknowledgeDTPInterrupt(dtpreg_t *dev)
{
    dev->command = ACK;
}

static void acknowledgeTermInterrupt(termreg_t *dev)
{
    if (terminalIsRECV(dev)) {
        dev->recv_command = ACK;
    } else {
        dev->transm_command = ACK;
    }
}

/*check if terminal is RECV or TRANSM and returns the status accordingly*/
static inline unsigned int getTermStatus(termreg_t* dev)
{
    if (terminalIsRECV(dev)) {
        return dev->recv_status;
    } else {
        return dev->transm_status;
    }
}

/*returns semaphore index associated to the device*/
unsigned int getSemIndex(unsigned int interruptLine, unsigned int deviceNo, int termIsRECV)
{
    switch (interruptLine) {
        case INTERTIMEINT:
            return DEVICE_NUM - 1;  //TODO REPLACE WITH MACRO

        case DISKINT:
        case FLASHINT:

        //return (interruptLine - DISKINT)*8 + deviceNo;
        case NETWINT:
        case PRNTINT:
            return (interruptLine - DISKINT) * 8 + deviceNo;

        case TERMINT:
            if (termIsRECV) {
                return (interruptLine - DISKINT) * 8 + deviceNo;
            } else {
                return (interruptLine - DISKINT + 1) * 8 + deviceNo;
            }

        default:
            PANIC();

            return 0;
    }
}


static int terminalIsRECV(termreg_t *dev)
{
    return dev->recv_status != READY;
}


void disable_interrupts()
{
    setSTATUS(getSTATUS() & DISABLEINTS);
}

void enable_interrupts()
{
    setSTATUS(getSTATUS() | ~DISABLEINTS);
}