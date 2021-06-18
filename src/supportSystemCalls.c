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



void handleSupportSystemcalls() {
  state_t *systemState = (state_t *) BIOSDATAPAGE;
  int currentSyscall = systemState -> reg_a0;  
  
  //necessario il controllo per la user mode?

  copyState(systemState, &(currentProcess -> p_s));
  currentProcess -> p_s.pc_epc = currentProcess -> p_s.pc_epc + 4;

  switch(currentSyscall){
    /* Support level sys calls */

    case TERMINATE: {
      terminate_support();
      break;
    }
    case GET_TOD: {
      get_TOD();
      break;
    }
    case WRITEPRINTER: {
      write_To_Printer();
      break;
    }
    case WRITETERMINAL: {
      write_To_Terminal();
      break;
    }
    case READTERMINAL: {
      read_From_Terminal();
      break;
    }
  }
}

/* Wrapper for SYS2 at support level */
void terminate_support() {
  SYSCALL(TERMPROCESS, 0, 0, 0);           //accertarsi che rilasci correttamente i semafori a livello supporto
}

/* Returns the number of microseconds from system power on */
void get_TOD() {
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_s.reg_v0 = globalStartT - stopT;  
  contextSwitch(currentProcess);
}

/* system call that manages the printing of an entire string passed as argument*/
void write_To_Printer() {
  char *virtAddr = currentProcess -> p_s.reg_a1;
  int strlen = currentProcess -> p_s.reg_a2;
  int retValue = 0;
  
  if(strlen <= 0 || strlen > 128 || virtAddr < (char*) UPROCSTARTADDR || ( virtAddr + strlen ) >= (char*) USERSTACKTOP/*indirizzo fuori dalla VM*/) {
    SYSCALL(TERMINATE, 0, 0, 0);
  }

  support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);      //verificare se serve sta cosa
  int asid = supp -> sup_asid;
  int printerAddress = DEV_REG_ADDR(PRINTINTERRUPT,asid-1); ;
	unsigned int * base = (unsigned int *) (printerAddress);
	unsigned int status;

  SYSCALL(PASSEREN,&(termReadSemaphores[asid]), 0, 0); 
  while (*virtAddr != EOS) {
		///stampa
		//status = SYSCALL(IOWAIT,);
		if ((status & TERMSTATMASK) != RECVD)
			PANIC();
		*virtAddr++;
    retValue++;	
	}
  SYSCALL(VERHOGEN,&(termReadSemaphores[asid]), 0, 0); 
   //se c' è errore sovrascrivo retvalue con - il valore dello status
  currentProcess -> p_s.reg_v0 = retValue;
  contextSwitch(currentProcess);
  
  
  //note:
  //dobbiamo avere la mutua esclusione sul dispositivo (stampante) controllano l'asid del processo corrente
  //funzionamento analogo a print: caricamento di un carattere, attesa attraverso la sys5, lettura dello stato e carattere successivo
  //se l'indirizzo della stringa da stampare è fuori dalla memoria virtuale del processo o la lunghezza della stringa è 0 terminiamo il processo
  //output: numero dei caratteri stampati in caso di successo, oppure bisogna restituire lo stato di errore negato (con - davanti)
}

void write_To_Terminal() {
  char *virtAddr = currentProcess -> p_s.reg_a1;
  int strlen = currentProcess -> p_s.reg_a2;
  int retValue = 0;

  if(strlen <= 0 || strlen > 128 || virtAddr < (char*) UPROCSTARTADDR || ( virtAddr + strlen ) >= (char*) USERSTACKTOP/*indirizzo fuori dalla VM*/) {
    SYSCALL(TERMINATE, 0, 0, 0); 
  }

  support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);     //verificare se serve sta cosa
  int asid = supp -> sup_asid;
  int terminalAddress = DEV_REG_ADDR(TERMINT,asid-1);                // controllare se è giusto
	unsigned int * base = (unsigned int *) (terminalAddress);
	int status;
	
	SYSCALL(PASSEREN,&(termWriteSemaphores[asid]), 0, 0); 
	while (*virtAddr != EOS) {
		*(base + 3) = PRINTCHR | (((unsigned int) *virtAddr) << BYTELENGTH);
		status = SYSCALL(IOWAIT, TERMINT, 0, 0);
		if ((status & TERMSTATMASK) != RECVD){
      retValue = status * -1;                                         //da controllare manuale umps
      *virtAddr = EOS;
      //PANIC();
    }else{
      *virtAddr++;
      retValue++;
    }	
	}
	SYSCALL(VERHOGEN,&(termWriteSemaphores[asid]), 0, 0); 
  currentProcess -> p_s.reg_v0 = retValue;
  contextSwitch(currentProcess);
}


void read_From_Terminal() {
  char *virtAddr = currentProcess -> p_s.reg_a1;
  if(virtAddr < (char*) UPROCSTARTADDR) {
    terminate(currentProcess);
  }

  support_t *supp = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);      //verificare se serve sta cosa
  int asid = supp -> sup_asid;
  int terminalAddress = DEV_REG_ADDR(TERMINT,asid-1); ;
	unsigned int * base = (unsigned int *) (terminalAddress);
	unsigned int status;
	int retValue = 0;

  SYSCALL(PASSEREN,&(termReadSemaphores[asid]), 0, 0); 
  while(*virtAddr != EOS /*verificare se servono altri controlli di fine stringa es \n \r etc*/) {

  }
  SYSCALL(VERHOGEN,&(termReadSemaphores[asid]), 0, 0); 
  currentProcess -> p_s.reg_v0 = retValue;
  contextSwitch(currentProcess);
  //note:
  //come sys12 ma legge da terminale invece di scrivere
  //mentre l'input viene letto il processo deve essere sospeso

}