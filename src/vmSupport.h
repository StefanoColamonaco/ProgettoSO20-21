#ifndef PROGETTOSO20_21_VMSUPPORT_H
#define PROGETTOSO20_21_VMSUPPORT_H

extern int freeAsidBitmap;
extern unsigned int swapFloor;

int getFreeAsid();

void initUprocPageTable(pcb_t *uproc);

void initSwapStructs();

void removePagesFromTLB(int asid);

void markTLBEntryNotValid(pteEntry_t* entry);

void markReadOnlyPages(support_t *supp);

void setSwapFloor();

#endif //PROGETTOSO20_21_VMSUPPORT_H