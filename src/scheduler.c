#include "pandos_const.h"

#include "init.h"
#include "scheduler.h"
#include "interrupts.h"
#include "exceptions.h"
#include "asl.h"
#include "pcb.h"
#include "stateUtil.h"

#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>

/*A simple preemptive and round robin scheduler algorithm with time-slice of 5ms*/
void scheduler()
{
	pcb_t *p = removeProcQ(&readyQueue);
	if (p != NULL)
	{
		prepareAndSwitch(p, TIMESLICE);
	}

	if (processCount == 0)
	{ //no process to execute
		HALT();
	}
	else
	{
		if (softBlockedCount > 0)
		{ //we need to wait for an interrupt from one of the softBlocked processes
			currentProcess = NULL;
			setStatusForWaiting();
			WAIT();
		}
		else
		{ //deadlock
			PANIC();
		}
	}
}

/*case the scheduler decides to run another process. In this case we save the current process and call back
the exception handling function of the Bios (which was provided to us) passing the status of the current process.*/
void loadProcess(pcb_t *current)
{
	currentProcess = current;
	LDST(&(currentProcess->p_s));
}

/*this function creates a new state capable of receiving interrupts during the WAIT phase*/
void setStatusForWaiting()
{
	setTIMER((unsigned int)0xFFFFFFFF);
	unsigned int newState = ALLOFF | IECON | IMON;
	setSTATUS(newState);
}

/*This function handles termination and initialization of process time and executes context switch*/
void prepareAndSwitch(pcb_t *p, int time)
{
	STCK(startT);
	setTIMER(time);
	loadProcess(p);
}