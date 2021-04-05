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

/*Handler for exception labeled as system calls*/
void handleSystemcalls(){
  state_t *systemState = (state_t *) BIOSDATAPAGE;
  int currentSyscall = systemState -> reg_a0;                    
  if(systemState->status & USERPON){                      //user mode -> trap/exception
    passupOrDie(GENERALEXCEPT);    
  }

  copyStateInfo(systemState, &(currentProcess -> p_s));
  currentProcess -> p_s.pc_epc = currentProcess -> p_s.pc_epc + 4;     //(+4 bytes)

  switch(currentSyscall){
    case CREATEPROCESS: {
      create_Process();
      break;
    }

    case TERMPROCESS: {
      terminate_Process(currentProcess);
      scheduler();
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

/* is used to create a process as a child of the process calling this sys call */
void create_Process() {
    pcb_t *tmp = allocPcb();

    if(tmp == NULL) {
        currentProcess->p_s.reg_v0 = -1;                        /* we can't create a new process */
    }else{ 

      copyStateInfo((state_t*)currentProcess->p_s.reg_a1, &tmp->p_s);    
      support_t *supportData = (support_t*)currentProcess -> p_s.reg_a2;    /* we assign to tmp the support structure if present*/
      if(supportData != NULL && supportData != 0) {
          tmp -> p_supportStruct = supportData;
      }

      insertProcQ(&readyQueue, tmp);
      insertChild(currentProcess, tmp);

      currentProcess->p_s.reg_v0 = 0;    //OK, process created correctly
    }
    contextSwitch(currentProcess);
}

static void terminate_Process_rec(pcb_t *current);

/*ends the invoking process and all its progeny*/
void terminate_Process(pcb_t *current) {
    if(current->p_semAdd != NULL) {
      *(current -> p_semAdd) = *(current -> p_semAdd) + 1;  
      outBlocked(current);
    } else {
      outProcQ(&readyQueue, current);
    }
    while(!emptyChild(current)){
        terminate_Process(removeChild(current));
    }
    outChild(current);
    freePcb(current);
}

static void terminate_Process_rec(pcb_t *current) {
  
}

/* operation with which the resource relating to a semaphore is requested */
void passeren() {
  mutex = (int*)(currentProcess -> p_s.reg_a1);
  *mutex = *mutex-1;
  if(*mutex < 0){
    blockCurrentProcessAt(mutex);        
    scheduler();
  } 
  else {  
    contextSwitch(currentProcess);
  }
}

/* operation with which the resource relating to a semaphore is released */
void verhogen() {
  mutex = (int*)(currentProcess -> p_s.reg_a1);
  *mutex = *mutex + 1;
  pcb_t *tmp = removeBlocked(mutex); ///se rimane la lista vuota allora incremento
  if (tmp != NULL)
    insertProcQ(&readyQueue, tmp);
  contextSwitch(currentProcess);
}

/*
blocks a process until an I / O operation is completed
I / O operations take an arbitrary time, so the process is put "on pause"
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

/*function that add current process to the semaphore's associated list of blocked pcbs*/
void blockCurrentProcessAt(int *sem) {
  cpu_t stopT;
  STCK(stopT);
  currentProcess -> p_time = currentProcess -> p_time + (stopT - startT);
  insertBlocked(sem, currentProcess);
  currentProcess = NULL;
}