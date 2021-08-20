#ifndef PROGETTOSO20_21_SYSSUPPORT_H
#define PROGETTOSO20_21_SYSSUPPORT_H

#include "pandos_const.h"
#include "pandos_types.h"

void handleSupportLevelExceptions();        //entry point for support level exceptions handling

void uTLB_RefillHandler();

pteEntry_t *getMissingPage();

pteEntry_t *getMissingPageFromSupp(support_t* supp);

void programTrapHandler( support_t *supp);

#endif //PROGETTOSO20_21_SYSSUPPORT_H