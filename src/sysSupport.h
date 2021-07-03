#ifndef PROGETTOSO20_21_SYSSUPPORT_H
#define PROGETTOSO20_21_SYSSUPPORT_H

#include "pandos_const.h"
#include "pandos_types.h"

void handleSupportLevelExceptions();        //entry point for support level exceptions handling

void uTLB_RefillHandler();

int getMissingPageNumber();

pteEntry_t *getMissingPage();

#endif //PROGETTOSO20_21_SYSSUPPORT_H