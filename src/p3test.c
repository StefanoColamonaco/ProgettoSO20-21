#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/regdef.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/cp0.h>
#include "print.h"

#include "pcb.h"
#include "p3test.h"
#include "sysSupport.h"
#include "tests/vmSupportTest.h"
#include "vmSupport.h"
#include "init.h"
#include "exceptions.h"

int masterSem = 0;


static state_t uproc_state[UPROCMAX + 1];	//this way we can index directly by ASID. 0 is unused
static support_t uproc_supp[UPROCMAX + 1];


static void initDevSemaphores();

static void initUProcs();

static void startUProcs();

static void waitForUprocs();

static void init_uproc_state(int asid);

static void init_uproc_support(int asid);

// ATTENZIONE, VARIABILE DA SOSTITUIRE CON UPROCMAX IN TUTTO IL FILE:
int procNum = 2;

void test_phase_3() {
	initSwapStructs();
	initDevSemaphores();
	setSwapFloor();
	masterSem = 0;
	initUProcs();
	startUProcs();
	waitForUprocs();
	SYSCALL(TERMPROCESS, 0, 0, 0);
}


static void initDevSemaphores() {
    for (int i = 0; i < (DEVICE_NUM); i++){
        deviceSemaphores[i] = 1;
    }
	for (int i = 1; i < N_DEV_PER_IL+1; i++){
        printerSemaphores[i] = 1;
		termWriteSemaphores[i] = 1;
		termReadSemaphores[i] = 1;
    }
}


void initUProcs() {
	for (int i = 0; i < procNum; i++) {
		int asid = i+1;//getFreeAsid();
		init_uproc_state(asid);
		init_uproc_support(asid);
		markReadOnlyPages(&uproc_supp[asid]);
	}
}


static void init_uproc_state(int asid) {
	uproc_state[asid].pc_epc = 0x800000B0;
	uproc_state[asid].reg_t9 = 0x800000B0;
	uproc_state[asid].reg_sp = 0xC0000000;
	uproc_state[asid].status = IECON | IMON | TEBITON; 
	uproc_state[asid].entry_hi = asid << ASIDSHIFT;
}

static void init_uproc_support(int asid) {
	uproc_supp[asid].sup_asid = asid;
	uproc_supp[asid].sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)handlePageFault; 
	uproc_supp[asid].sup_exceptContext[GENERALEXCEPT].pc = (memaddr)handleSupportLevelExceptions; 
	uproc_supp[asid].sup_exceptContext[PGFAULTEXCEPT].status = TEBITON | IECON; //local timer and interrupts enabled, kernel mode by default
	uproc_supp[asid].sup_exceptContext[GENERALEXCEPT].status = TEBITON | IECON;
	uproc_supp[asid].sup_exceptContext[PGFAULTEXCEPT].stackPtr = (unsigned int) &(uproc_supp[asid].sup_stackTLB[499]);	//stack grown downward
	uproc_supp[asid].sup_exceptContext[GENERALEXCEPT].stackPtr = (unsigned int) &(uproc_supp[asid].sup_stackGen[499]);
	init_uproc_pagetable(&uproc_supp[asid]);
}


static void startUProcs() {
	for (int i = 1; i < procNum+1; i++) {
		SYSCALL(1, (unsigned int) &uproc_state[i], (unsigned int) &uproc_supp[i], 0);
	}
}


static void waitForUprocs() {
	for (int i = 0; i < procNum; i++)
	{
		SYSCALL(PASSEREN, (unsigned int) &masterSem, 0, 0);
	}
}


void notifyTerminated() {
	SYSCALL(VERHOGEN, (unsigned int) &masterSem, 0, 0);
}