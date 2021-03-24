#include "systemCalls.h"
#include "init.h"
#include "exceptions.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "stateUtil.h"

int *mutualExclusion = 0;

void handleSystemcalls(){

  state_t *systemState = (state_t *) BIOSDATAPAGE;
  int currentSyscall = systemState -> reg_a0;                    //in a0 (gpr[3]) troviamo il numero della sys call
  if(systemState -> status & USERPON != 0){                      //processo in user mode -> trap/eccezione
    kill(GENERALEXCEPT);    
  }

  copyStateInfo(systemState, &(currentProcess -> p_s));
  currentProcess -> p_s.pc_epc = currentProcess -> p_s.pc_epc + 4;     //nota: pc_epc è il pc salvato nello stato del processo (4 bytes)

  switch(currentSyscall){
    case CREATEPROCESS: {
      create_Process();
    }

    case TERMPROCESS: {
      terminate_Process(currentProcess);
      scheduler();                                  //terminato il processo si richiama lo scheduler
    }

    case PASSEREN: {
      passeren();
    }

    case VERHOGEN: {
      verhogen();
    }

    case IOWAIT: {
      wait_For_IO();
    }

    case GETTIME: {
        get_Cpu_Time();
    }

    case CLOCKWAIT: {
      wait_For_Clock();
    }

    case GETSUPPORTPTR: {
      get_Support_Data();
    }
    // otherwise (sys call number > 8 or < 1)
    default: {
      kill(GENERALEXCEPT);
    }
  }

}

/*serve a creare un processo come figlio del processo che invoca questa sys call*/

int create_Process() {
    pcb_t *tmp = allocPcb();

    if(tmp == NULL) {
        currentProcess -> p_s.reg_v0 = -1;                        /* non possiamo creare il processo, ritorniamo -1 */
        return -1;
    }

    copyStateInfo(&currentProcess -> p_s.reg_a1, &tmp -> p_s);    /* we copy the process into tmp and assign it the support structure if present*/
    support_t *supportData = currentProcess -> p_s.reg_a2;
    if(supportData != NULL || supportData != 0) {
        tmp -> p_supportStruct = supportData;
    }
    processCount++;
    insertProcQ(&readyQueue, tmp);
    insertChild(currentProcess, tmp);
    currentProcess -> p_s.reg_v0 = 0;                                   //Since process is "ready", we can return 0 (ok)
    contextSwitch(currentProcess);
}

/*termina il processo invocante e tutta la sua progenie*/

void terminate_Process(pcb_t *current) {
    while(!emptyChild(current)){
        terminate_Process(removeChild(current));
    }
    outChild(current);
}

/*operazione con cui si richiede la risorsa relativa ad un semaforo*/

void passeren() {
  mutualExclusion = (int *) currentProcess -> p_s.reg_a1;
  *mutualExclusion--;
  if(*mutualExclusion < 0){
    blockCurrentProc(mutualExclusion);
  } else contextSwitch(currentProcess);
}

/*operazione con cui si rilascia la risorsa relativa ad un semaforo*/

void verhogen() {
  mutualExclusion = (int *) currentProcess -> p_s.reg_a1;
  *mutualExclusion++;
  if(*mutualExclusion <= 0){
    pcb_t *tmp = removeBlocked(mutualExclusion);
    if(tmp != NULL){
      insertProcQ(&readyQueue, tmp);
    }
  }
  contextSwitch(currentProcess);
}

/*
blocca un processo fino al termine di un operazione di I/O 
le operazioni di I/O necessitano di un tempo arbitrario, quindi il processo viene messo "in pausa"
*/

int  wait_For_IO() {
  int lineNumber = currentProcess -> p_s.reg_a1;   //interrupt line number
  int deviceNumber = currentProcess -> p_s.reg_a2;     //device instance
  deviceNumber = deviceNumber + ((lineNumber - DISKINT) * DEVPERINT);

  if((lineNumber == TERMINT) && (currentProcess -> p_s.reg_a3)){     //distinzione tra lettura e scrittura nel caso di un terminale
    deviceNumber = deviceNumber + DEVPERINT;
  }

  deviceSemaphores[deviceNumber]--;

  if(deviceSemaphores[deviceNumber] < 0){
    softBlockCount++;
    blockCurrentProcessAt(&(deviceSemaphores[deviceNumber]));
  } else {
    currentProcess -> p_s.reg_v0  = deviceStat[deviceNumber];     //ritorno il registro di stato del dispositivi richiesto
    contextSwitch(currentProcess);
  }
}

/*restituisce il tempo di esecuzione totale del processo che la invoca*/
// nota: si tiene traccia del tempo all'interno dell'interrupt handler

int get_Cpu_Time() {
   currentProcess -> p_s.reg_v0 = currentProcess -> p_time;  
}

/*consente al processo invocante di "bloccarsi" in attesa di un giro dell'interval timer (prossimo tick del dispositivo)*/

int wait_For_Clock() {
  clockSemaphore--;
  if(clockSemaphore < 0){
    softBlockCount++;
    blockCurrentProcessAt(&clockSemaphore);
  }
  contextSwitch(currentProcess);
}

/*Restituisce un puntatore alla struttura di supporto del processo corrente*/

support_t *get_support_data() {
  //aggiungere controllo?
  currentProcess -> p_s.reg_v0 = currentProcess -> p_supportStruct;
}


void blockCurrentProcessAt(int *sem){
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
  insertBlocked(sem, currentProcess);
  currentProcess = NULL;
  scheduler();
}