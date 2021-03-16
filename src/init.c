#include "init.h"


int processCount = 0;        //number of started but not yet terminated processes

int softBlockCount = 0;      //number of processes blocked due to I/O

pcb_t *readyQueue = mkEmptyProcQ();       //tail pointer to queue of ready processes

pcb_t *currentProcess = NULL;   //pointer to pcb that is in running state

//todo populate pass-up vector

int main() {
    initPcbs();
    initASL();
}