#include "systemCalls.h"
#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "stateUtil.h"
#include "interrupts.h"

#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>


int *mutex = 0;

/*Handler for exceptions labeled as system calls*/
void handleSystemcalls(){
  state_t *systemState = (state_t *) BIOSDATAPAGE;
  int currentSyscall = systemState -> reg_a0;                    
  if(systemState->status & USERPON){                      //user mode -> trap/exception
    passupOrDie(GENERALEXCEPT);    
  }

  copyState(systemState, &(currentProcess -> p_s));
  currentProcess -> p_s.pc_epc = currentProcess -> p_s.pc_epc + 4;     //(+4 bytes)

  switch(currentSyscall){
    case CREATEPROCESS: {
      create_Process();
      break;
    }

    case TERMPROCESS: {
      terminate_Process(currentProcess);
      break;                                
    }

    case PASSEREN: {
      passeren();
      break;
    }

    case VERHOGEN: {
      verhogen();
      break;
    }

    case IOWAIT: {
      wait_For_IO();
      break;
    }

    case GETTIME: {
      get_Cpu_Time();
      break;
    }

    case CLOCKWAIT: {
      wait_For_Clock();
      break;
    }

    case GETSUPPORTPTR: {
      get_support_data();
      break;
    }

    /* Support level sys calls */

    case TERMINATE: {
      terminate();
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

    /* otherwise (sys call number > 13 or < 1) */

    default: {
      passupOrDie(GENERALEXCEPT);
      break;
    }
  }

}

/* creates process as a child of the calling pcb  */
void create_Process() {
    pcb_t *tmp = allocPcb();

    if (tmp == NULL) {
        currentProcess->p_s.reg_v0 = -1;                        /* we can't create a new process */
    } else { 
      copyState((state_t*)currentProcess->p_s.reg_a1, &tmp->p_s);    
      support_t *supportData = (support_t*)currentProcess->p_s.reg_a2;    /* we assign to tmp the support structure if present*/
      if(supportData != NULL && supportData != 0) {
          tmp->p_supportStruct = supportData;
      }
      insertProcQ(&readyQueue, tmp);
      insertChild(currentProcess, tmp);
      currentProcess->p_s.reg_v0 = 0;    //OK, process created correctly
    }
    contextSwitch(currentProcess);
}

static void terminate_Process_rec(pcb_t *current);

/*terminates the invoking process and all its progeny*/
void terminate_Process(pcb_t *current) {
  terminate_Process_rec(current);
  scheduler();
}

static void terminate_Process_rec(pcb_t *current) {
  if (current == NULL) {
    return;
  }
  outChild(current);
  while(!emptyChild(current)){
    terminate_Process_rec(removeChild(current));
  }
  if(current->p_semAdd != NULL) {
    *(current -> p_semAdd) = *(current -> p_semAdd) + 1;  
    outBlocked(current);
  } else {
    outProcQ(&readyQueue, current);
  }
    freePcb(current);
}

/* operation with which the resource relating to a semaphore is requested */
void passeren() {
  mutex = (int*)(currentProcess->p_s.reg_a1);
  *mutex = *mutex - 1;
  if (*mutex < 0) {
    blockCurrentProcessAt(mutex);        
    scheduler();
  } else {  
    contextSwitch(currentProcess);
  }
}

/* operation with which the resource relating to a semaphore is released */
void verhogen() {
  mutex = (int*)(currentProcess -> p_s.reg_a1);
  *mutex = *mutex + 1;
  pcb_t *tmp = removeBlocked(mutex); ///se rimane la lista vuota allora incremento
  if (tmp != NULL){
    insertProcQ(&readyQueue, tmp);
  }
  contextSwitch(currentProcess);
}

/*
blocks a process until an I / O operation is completed
I / O operations take arbitrary time, so the process is put "on hold"
*/
void  wait_For_IO() {
  int lineNumber = currentProcess -> p_s.reg_a1;   
  int deviceNumber = currentProcess -> p_s.reg_a2;
  unsigned int deviceIndex = getSemIndex(lineNumber, deviceNumber, currentProcess->p_s.reg_a3);
  deviceSemaphores[deviceIndex]--;
  softBlockedCount++;
  blockCurrentProcessAt(&(deviceSemaphores[deviceIndex]));
  scheduler();
}

/* returns the total execution time of the process that invokes it */
void get_Cpu_Time() {
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
  currentProcess -> p_s.reg_v0 = currentProcess -> p_time;  
  startT = stopT;
  contextSwitch(currentProcess);
}

/* allows the invoking process to wait for a spin of the interval timer (next device tick) */
void wait_For_Clock() {
  clockSemaphore--;
  if(clockSemaphore < 0) {
    softBlockedCount++;
    blockCurrentProcessAt(&clockSemaphore);
    scheduler();
  }
  contextSwitch(currentProcess);
}

/* Returns a pointer to the support structure of the current process */
void get_support_data() {
    currentProcess -> p_s.reg_v0 = (unsigned int)currentProcess->p_supportStruct;
    contextSwitch(currentProcess);
}

/* Wrapper for SYS2 at support level */
void terminate() {
  terminate_Process(currentProcess);
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
  char *str = currentProcess -> p_s.reg_a1;
  int strlen = currentProcess -> p_s.reg_a2;

  if(strlen <= 0 /*|| indirizzo fuori dalla VM*/) {
    terminate(currentProcess);
  }
  //note:
  //dobbiamoavere la mutua esclusione sul dispositivo (stampante) controllano l'asid del processo corrente
  //funzionamento analogo a print: caricamento di un carattere, attesa attraverso la sys5, lettura dello stato e carattere successivo
  //se l'indirizzo della stringa da stampare è fuori dalla memoria virtuale del processo o la lunghezza della stringa è 0 terminiamo il processo
  //output: numero dei caratteri stampati in caso di successo, oppure bisogna restituire lo stato di errore negato (con - davanti)
}

void write_To_Terminal() {
  char *str = currentProcess -> p_s.reg_a1;
  int strlen = currentProcess -> p_s.reg_a2;

  if(strlen <= 0 /*|| indirizzo fuori dalla VM*/) {
    terminate(currentProcess);
  }
  //note:
  //come la sys 11 ma adattata alla scrittura su terminale
}

void read_From_Terminal() {
  char *str = currentProcess -> p_s.reg_a1;
  int strlen = currentProcess -> p_s.reg_a2;

  if(strlen <= 0 /*|| indirizzo fuori dalla VM*/) {
    terminate(currentProcess);
  }
  //note:
  //come sys12 ma legge da terminale invece di scrivere
  //mentre l'input viene letto il processo deve essere sospeso

}

/*function that add current process to the semaphore's associated list of blocked pcbs*/
void blockCurrentProcessAt(int *sem) {
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
  insertBlocked(sem, currentProcess);
  currentProcess = NULL;
}

