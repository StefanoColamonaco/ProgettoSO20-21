#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/regdef.h>
#include <umps3/umps/arch.h>
#include "print.h"

#include "tests/vmSupportTest.h"
#include "vmSupport.h"
#include "init.h"

static state_t uproc_state[UPROCMAX + 1];
static support_t uproc_supp[UPROCMAX + 1];


void test_phase_3() {
	initSwapTable();
	initSwapSemaphore();
	initDevSemaphores();
	initUProcs();
	SYSCALL(TERMPROCESS, 0, 0, 0);
}


void initUProcs() {
	for (int i = 0; i < UPROCMAX; i++) {
		int asid = getFreeAsid();
		init_uproc_state(asid);
		init_uproc_support(asid);
	}
}


void init_uproc_state(int asid) {
	uproc_state[asid].pc_epc = 0x800000B0;
	uproc_state[asid].reg_t9 = 0x800000B0;
	uproc_state[asid].reg_sp = 0xC0000000;
	uproc_state[asid].status = ALLOFF | IECON | IMON | TEBITON; 
	uproc_state[asid].entry_hi = asid << ASIDSHIFT;
}

void init_supp_structures(int asid) {
	uproc_supp[asid].sup_asid = asid;


}
