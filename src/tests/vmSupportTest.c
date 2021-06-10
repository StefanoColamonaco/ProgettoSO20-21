#include "vmSupportTest.h"
#include "../vmSupport.h"

#include "../pcb.h"
#include "../print.h"
#include <umps3/umps/types.h>

/*unit tests for this module. To move to another file*/

void testGetFreeAsid() {
    extern unsigned int freeAsidBitmap;

    freeAsidBitmap = 2;
    int asid = getFreeAsid();
    if (asid != 1) {
        print("asid is incorrect: not 1\n");
    }

    freeAsidBitmap = 0;
    asid = getFreeAsid();
    if (asid != -1) {
        print ("no free asid but -1 not returned\n");
    }
}


void testInitUProcPageTable() {
    pcb_t* uproc = allocPcb();
    int asid = uproc->p_supportStruct->sup_asid;
    initUprocPageTable(uproc);

    for(int i = 0; i < MAXPAGES; i++) {
        pteEntry_t *page = &uproc->p_supportStruct->sup_privatePgTbl[i];
        unsigned int target_entryHI = (0x80000 + i) << (VPNSHIFT + asid); 
        //unsigned int target_entryLO = ()
        if (page->pte_entryHI != target_entryHI){
            print("incorrect EntryHi");
        }
        // if (page->pte_entryLO != target_entryLO) {
        //     print("incorrect EntryLo");
        // }
    }


}