#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/regdef.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/cp0.h>

#include "pcb.h"
#include "p3test.h"
#include "sysSupport.h"
#include "vmSupport.h"
#include "init.h"
#include "exceptions.h"
#include "initSupp.h"

int masterSem = 0;


static state_t uproc_state;
static support_t uproc_supp[UPROCMAX + 1]; //this way we can index directly by ASID. 0 is unused


static void initDevSemaphores();

static void initUProcs();

static void waitForUprocs();

static void init_uproc_state(int asid);

static void init_uproc_support(support_t* supp);

static void initSuppStack();


void test_phase_3()
{
    initSwapStructs();
    initDevSemaphores();
    initSuppStack();
    masterSem = 0;
    initUProcs();
    waitForUprocs();
    SYSCALL(TERMPROCESS, 0, 0, 0);
}


static void initDevSemaphores()
{
    for (int i = 0; i < (DEVICE_NUM); i++) {
        deviceSemaphores[i] = 1;
    }

    for (int i = 1; i < N_DEV_PER_IL + 1; i++) {
        printerSemaphores[i] = 1;
        termWriteSemaphores[i] = 1;
        termReadSemaphores[i] = 1;
    }
}


void initUProcs()
{
    for (int i = 0; i < UPROCMAX; i++) {
        support_t* supp = alloc_supp();
        supp->sup_asid = i + 1;
        init_uproc_support(supp);
        init_uproc_state(supp->sup_asid);
        SYSCALL(1, (unsigned int) &uproc_state, (unsigned int) supp, 0);
    }
}


static void init_uproc_state(int asid)
{
    uproc_state.pc_epc = 0x800000B0;
    uproc_state.reg_t9 = 0x800000B0;
    uproc_state.reg_sp = 0xC0000000;
    uproc_state.status = IECON | IMON | TEBITON;
    uproc_state.entry_hi = asid << ASIDSHIFT;
}

static void init_uproc_support(support_t* supp)
{
    supp->sup_exceptContext[PGFAULTEXCEPT].pc = (memaddr)handlePageFault;
    supp->sup_exceptContext[GENERALEXCEPT].pc = (memaddr)handleSupportLevelExceptions;
    supp->sup_exceptContext[PGFAULTEXCEPT].status = TEBITON | IECON; //local timer and interrupts enabled, kernel mode by default
    supp->sup_exceptContext[GENERALEXCEPT].status = TEBITON | IECON;
    memaddr ramTop;
    RAMTOP(ramTop);
    supp->sup_exceptContext[PGFAULTEXCEPT].stackPtr = ramTop - (supp->sup_asid * 2) * PAGESIZE;	//stack grows downward
    supp->sup_exceptContext[GENERALEXCEPT].stackPtr = ramTop - (supp->sup_asid * 2 + 1) * PAGESIZE;
    init_uproc_pagetable(supp);
}

static void waitForUprocs()
{
    for (int i = 0; i < UPROCMAX; i++) {
        SYSCALL(PASSEREN, (unsigned int) &masterSem, 0, 0);
    }
}


void notifyTerminated()
{
    SYSCALL(VERHOGEN, (unsigned int) &masterSem, 0, 0);
}


static void initSuppStack()
{
    for (int i = 0; i < UPROCMAX; i++) {
        dealloc_supp(&uproc_supp[i]);
    }

}