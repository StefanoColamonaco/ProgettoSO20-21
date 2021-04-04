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

void handleSystemcalls(){
  state_t *systemState = (state_t *) BIOSDATAPAGE;
  int currentSyscall = systemState -> reg_a0;                    //in a0 (gpr[3]) troviamo il numero della sys call
  if(systemState->status & USERPON){                      //processo in user mode -> trap/eccezione
    passupOrDie(GENERALEXCEPT);    
  }

  copyStateInfo(systemState, &(currentProcess -> p_s));
  currentProcess -> p_s.pc_epc = currentProcess -> p_s.pc_epc + 4;     //nota: pc_epc è il pc salvato nello stato del processo (4 bytes)

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
    // otherwise (sys call number > 8 or < 1)
    default: {
      passupOrDie(GENERALEXCEPT);
      break;
    }
  }

}

/*serve a creare un processo come figlio del processo che invoca questa sys call*/
void create_Process() {
    pcb_t *tmp = allocPcb();

    if(tmp == NULL) {
        currentProcess->p_s.reg_v0 = -1;                        /* non possiamo creare il processo, ritorniamo -1 */                                            //TODO check if return value il correct
    }else{ 

      copyStateInfo((state_t*)currentProcess->p_s.reg_a1, &tmp->p_s);    /* we copy the process into tmp and assign it the support structure if present*/
      support_t *supportData = (support_t*)currentProcess -> p_s.reg_a2;
      if(supportData != NULL && supportData != 0) {
          tmp -> p_supportStruct = supportData;
      }

      processCount++;
      insertProcQ(&readyQueue, tmp);
      insertChild(currentProcess, tmp);

      currentProcess->p_s.reg_v0 = 0;
    }
    contextSwitch(currentProcess);
}

/*termina il processo invocante e tutta la sua progenie*/
void terminate_Process(pcb_t *current) {
    while(!emptyChild(current)){
        terminate_Process(removeChild(current));
    }
    processCount--;
    outChild(current);
    scheduler();
}

/*operazione con cui si richiede la risorsa relativa ad un semaforo*/
void passeren() {
  mutex = (int*)(currentProcess -> p_s.reg_a1);
  if(*mutex <= 0){
    blockCurrentProcessAt(mutex);        
    scheduler();
  } 
  else {  
    (*mutex) = (*mutex)-1;
    contextSwitch(currentProcess);
  }
}

/*operazione con cui si rilascia la risorsa relativa ad un semaforo*/
void verhogen() {
  mutex = (int*)(currentProcess -> p_s.reg_a1);
  //if((*mutex)+1 > 0){
    if(headBlocked(mutex) == NULL) {
        (*mutex) = (*mutex)+1;
    }else{
    pcb_t *tmp = removeBlocked(mutex); ///se rimane la lista vuota allora incremento
    if(tmp != NULL){
      insertProcQ(&readyQueue, tmp);
    }
    }
  //}  
  contextSwitch(currentProcess);
}

/*
blocca un processo fino al termine di un operazione di I/O 
le operazioni di I/O necessitano di un tempo arbitrario, quindi il processo viene messo "in pausa"
*/

void  wait_For_IO() {
  int lineNumber = currentProcess -> p_s.reg_a1;   
  int deviceNumber = currentProcess -> p_s.reg_a2;
  unsigned int deviceIndex = getSemNumber(lineNumber, deviceNumber, currentProcess->p_s.reg_a3);
  deviceSemaphores[deviceIndex]--;
  softBlockedCount++;
  blockCurrentProcessAt(&(deviceSemaphores[deviceIndex]));
  scheduler();
}

/*restituisce il tempo di esecuzione totale del processo che la invoca*/
// nota: si tiene traccia del tempo all'interno dell'interrupt handler
void get_Cpu_Time() {
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
  currentProcess -> p_s.reg_v0 = currentProcess -> p_time;  
  startT = stopT;
  contextSwitch(currentProcess);
}

/*consente al processo invocante di "bloccarsi" in attesa di un giro dell'interval timer (prossimo tick del dispositivo)*/
void wait_For_Clock() {
  clockSemaphore--;
  if(clockSemaphore < 0) {
    softBlockedCount++;
    blockCurrentProcessAt(&clockSemaphore);
    scheduler();
  }
  contextSwitch(currentProcess);
}

/*Restituisce un puntatore alla struttura di supporto del processo corrente*/
void get_support_data() {
    currentProcess -> p_s.reg_v0 = (unsigned int)currentProcess->p_supportStruct;
    contextSwitch(currentProcess);
}


void blockCurrentProcessAt(int *sem) {
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
  insertBlocked(sem, currentProcess);
  currentProcess = NULL;
}