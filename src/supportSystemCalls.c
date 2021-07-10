#include "nucleousSystemCalls.h"
#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "stateUtil.h"
#include "interrupts.h"
#include "supportSystemCalls.h"

#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>
#include <umps3/umps/const.h>

state_t *state;
state_t *newState;
support_t *supp;

void handleSupportSystemcalls(state_t *systemState, support_t *support)
{
	newState = (state_t*) &(support->sup_exceptState[GENERALEXCEPT]);
	int currentSyscall = systemState->reg_a0;
	state = systemState;
	supp = support;

	switch (currentSyscall)
	{
		/* Support level sys calls */

	case TERMINATE:
	{
		terminate_support();
		break;
	}
	case GET_TOD:
	{
		get_TOD();
		break;
	}
	case WRITEPRINTER:
	{
		write_To_Printer();
		break;
	}
	case WRITETERMINAL:
	{
		write_To_Terminal();
		break;
	}
	case READTERMINAL:
	{
		read_From_Terminal();
		break;
	}
	}
}

/* Wrapper for SYS2 at support level */
void terminate_support()
{
	SYSCALL(TERMPROCESS, 0, 0, 0); //accertarsi che rilasci correttamente i semafori a livello supporto
}

/* Returns the number of microseconds from system power on */
void get_TOD()
{
	cpu_t stopT;
	STCK(stopT);
	state->reg_v0 = globalStartT - stopT;
	LDST(state);
}

/* system call that manages the printing of an entire string passed as argument*/
void write_To_Printer()
{

	char *virtAddr = (char *)(state->reg_a1);
	int strlen = state->reg_a2;
	int retValue = 0;

	if (strlen <= 0 || strlen > 128 || virtAddr < (char *)UPROCSTARTADDR || (virtAddr + strlen) >= (char *)USERSTACKTOP /*indirizzo fuori dalla VM*/)
	{
		SYSCALL(TERMINATE, 0, 0, 0);
	}

	int asid = supp->sup_asid;
	devreg_t* base = (devreg_t*)DEV_REG_ADDR(PRNTINT, asid - 1);
	dtpreg_t* dtp = &(base->dtp);

	unsigned int status;

	SYSCALL(PASSEREN, (unsigned int)&(printerSemaphores[asid]), 0, 0);
	while (*virtAddr != EOS)
	{
		disable_interrupts();
		dtp->data0 = *virtAddr;
		dtp->command = PRINTCHR;
		status = SYSCALL(IOWAIT, PRNTINT, asid - 1, 0);
		enable_interrupts();

		if (((status & PRINTSTATMASK) != PRINTDEVREADY) && ((status & PRINTSTATMASK) != PRINTDEVREADY))
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
		}
		else
		{
			virtAddr++;
			retValue++;
		}
	}
	SYSCALL(VERHOGEN, (unsigned int)&(printerSemaphores[asid]), 0, 0);

	newState->reg_v0 = retValue;
	newState -> pc_epc +=4;
	LDST(newState);
}

void write_To_Terminal()
{
	char *virtAddr = (char *)(state->reg_a1);
	int strlen = state->reg_a2;
	int retValue = 0;

	if (strlen <= 0 || strlen > 128 || virtAddr < (char *)UPROCSTARTADDR || (virtAddr + strlen) >= (char *)USERSTACKTOP /*indirizzo fuori dalla VM*/)
	{
		SYSCALL(TERMINATE, 0, 0, 0);
	}

	int asid = supp->sup_asid;
	devreg_t* base = (devreg_t*)DEV_REG_ADDR(TERMINT, asid - 1);
	termreg_t* term = &(base ->term); 
	int status;

	SYSCALL(PASSEREN, (unsigned int)(&termWriteSemaphores[asid]), 0, 0);
	while (*virtAddr != EOS)
	{
		disable_interrupts();
		term->transm_command = PRINTCHR | (((unsigned int)*virtAddr) << BYTELENGTH);
		status = SYSCALL(IOWAIT, TERMINT, asid - 1, FALSE);
		enable_interrupts();
		if ((status & TERMSTATMASK) != RECVD)
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
		}
		else
		{
			virtAddr++;
			retValue++;
		}
	}
	SYSCALL(VERHOGEN, (unsigned int)(&termWriteSemaphores[asid]), 0, 0);
	newState->reg_v0 = retValue;
	newState -> pc_epc +=4;
	LDST(newState);
}


void read_From_Terminal()
{
	char *virtAddr = (char*)(state->reg_a1);
	if (virtAddr < (char *)UPROCSTARTADDR)
	{
		SYSCALL(TERMINATE, 0, 0, 0);
	}
	
	int asid = supp->sup_asid;
	devreg_t* address = (devreg_t*) DEV_REG_ADDR(TERMINT, asid - 1);
	termreg_t* term = &(address->term);
	
	unsigned int status = READY;
	int retValue = 0;
	char rcvd = '0';


	SYSCALL(PASSEREN, (unsigned int)&(termReadSemaphores[asid]), 0, 0);
	while ((rcvd != '\n' ) && (((status & TERMSTATMASK) == READY) || ((status & TERMSTATMASK) == 5) ))
	{
		disable_interrupts();
		term->recv_command = RECVCHR; 
		status = SYSCALL(IOWAIT, TERMINT, asid - 1, TRUE);
		enable_interrupts();
		*virtAddr = status >> 8; 
		rcvd = *virtAddr;
		
		if ((status & TERMSTATMASK) != RECVD && (status & TERMSTATMASK) != READY)
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
		}
		else
		{
			virtAddr++;
			retValue++;
		}
	}
	SYSCALL(VERHOGEN, (unsigned int)&(termReadSemaphores[asid]), 0, 0);
	
	newState->reg_v0 = retValue;
	newState -> pc_epc +=4;

	LDST(newState);
}

