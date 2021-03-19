#include "pandos_const.h"
#include "pandos_types.h"

#include "init.h"
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"

void systemcallsHandler(){

  state_t *systemState = (state_t *) BIOSDATAPAGE;
  int currentSyscall = systemState -> reg_a0;                    //in a0 (gpr[3]) troviamo il numero della sys call
  if(systemState -> status & USERPON != 0){                      //processo in user mode -> trap/eccezione
    kill();    //eccezione
  }

  copyStateInfo(systemState, &(currentProcess -> p_s));
  currentProcess -> p_s.pc_epc = currentProcess -> p_s.pc_epc + 4;     //nota: pc_epc è il pc salvato nello stato del processo

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
      passUpOrDie(GENERALEXCEPT);
    }
  }

}

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

void terminate_Process(pcb_t *current) {
    while(!emptyChild(current)){
        terminate_Process(removeChild(current));
    }
    outChild(current);
}

void passeren() {

}

void verhogen() {

}

int  wait_For_IO() {

}

int get_Cpu_Time() {

}

int wait_For_Clock() {

}

support_t *get_support_data() {

}

kill() {

}