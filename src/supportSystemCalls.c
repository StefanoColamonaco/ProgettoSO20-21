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

void stop2()
{
}

state_t *state;
state_t *newState;

void handleSupportSystemcalls(state_t *systemState, support_t *support)
{
	newState = (state_t*) &(support->sup_exceptState[GENERALEXCEPT]);
	int currentSyscall = systemState->reg_a0;
	state = systemState;

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
	support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0); //verificare se serve sta cosa
	int asid = supp->sup_asid;
	int printerAddress = DEV_REG_ADDR(PRINTINTERRUPT, asid - 1);
	;
	unsigned int *base = (unsigned int *)(printerAddress);
	unsigned int status;

	SYSCALL(PASSEREN, (unsigned int)&(termReadSemaphores[asid]), 0, 0);
	while (*virtAddr != EOS)
	{
		*base = PRINTCHR | (((unsigned int)*virtAddr) << BYTELENGTH);
		status = SYSCALL(IOWAIT, PRINTINTERRUPT, asid - 1, 0);
		if ((status & PRINTSTATMASK) != PRINTDEVREADY)
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
			//PANIC();
		}
		else
		{
			virtAddr++;
			retValue++;
		}
	}
	SYSCALL(VERHOGEN, (unsigned int)&(termReadSemaphores[asid]), 0, 0);
	//se c' è errore sovrascrivo retvalue con - il valore dello status
	newState->reg_v0 = retValue;
	newState -> pc_epc +=4;
	LDST(newState);

	//note:
	//dobbiamo avere la mutua esclusione sul dispositivo (stampante) controllano l'asid del processo corrente
	//funzionamento analogo a print: caricamento di un carattere, attesa attraverso la sys5, lettura dello stato e carattere successivo
	//se l'indirizzo della stringa da stampare è fuori dalla memoria virtuale del processo o la lunghezza della stringa è 0 terminiamo il processo
	//output: numero dei caratteri stampati in caso di successo, oppure bisogna restituire lo stato di errore negato (con - davanti)
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

	support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0); //verificare se serve sta cosa
	int asid = supp->sup_asid;
	int terminalAddress = DEV_REG_ADDR(TERMINT, asid - 1); // controllare se è giusto
	unsigned int *base = (unsigned int *)(terminalAddress);
	int status;
	SYSCALL(PASSEREN, (unsigned int)(&termWriteSemaphores[asid]), 0, 0);
	while (*virtAddr != EOS)
	{
		*(base + TRANCOMMAND) = PRINTCHR | (((unsigned int)*virtAddr) << BYTELENGTH);
		status = SYSCALL(IOWAIT, TERMINT, asid - 1, FALSE);
		if ((status & TERMSTATMASK) != RECVD)
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
			//PANIC();
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

char rcvd;


void read_From_Terminal()
{	/*
	char *virtAddr = (char *)(state->reg_a1);
	if (virtAddr < (char *)UPROCSTARTADDR)
	{
		SYSCALL(TERMINATE, 0, 0, 0);
	}
	
	support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0); //verificare se serve sta cosa
	int asid = supp->sup_asid;
	int terminalAddress = DEV_REG_ADDR(TERMINT, asid - 1);

	unsigned int *base = (unsigned int *)(terminalAddress);
	unsigned int status = READY;
	char initial_stringa = EOS;
    char *stringa = &initial_stringa;
	int retValue = 0;
	char received_char = '0';

	SYSCALL(PASSEREN, (unsigned int)&(termReadSemaphores[asid]), 0, 0);
	while (received_char != EOS  && ((status == READY) || (status == 5) ))
	{
		stop2();
		*(base + RECVCOMMAND) = RECVCHR | (((unsigned int)*virtAddr) );
		status = SYSCALL(IOWAIT, TERMINT, asid - 1, TRUE);
		received_char = *(base + RECVSTATUS) ;
		rcvd = received_char;
		if ((status & TERMSTATMASK) != RECVD)
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
			//PANIC();
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
	//note:
	//come sys12 ma legge da terminale invece di scrivere
	//mentre l'input viene letto il processo deve essere sospeso
	*/
	char *virtAddr = (char *)(state->reg_a1);
	if (virtAddr < (char *)UPROCSTARTADDR)
	{
		SYSCALL(TERMINATE, 0, 0, 0);
	}
	
	support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0); //verificare se serve sta cosa
	int asid = supp->sup_asid;
	devreg_t* address = (devreg_t*) DEV_REG_ADDR(TERMINT, asid - 1);
	termreg_t* term = &(address->term);
	
	unsigned int status = READY;
	int retValue = 0;
	char received_char = '0';

	while (received_char != EOS  && ((status == READY) || (status == 5) ))
	{
		term->recv_command = RECVCHR;
		status = SYSCALL(IOWAIT, TERMINT, asid - 1, TRUE);
		received_char = (term->recv_status) >> 8 ;
		rcvd = received_char;
		stop2();
		if ((status & TERMSTATMASK) != RECVD)
		{
			retValue = status * -1; //da controllare manuale umps
			*virtAddr = EOS;
			//PANIC();
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

