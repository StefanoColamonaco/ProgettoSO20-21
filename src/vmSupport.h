#ifndef PROGETTOSO20_21_VMSUPPORT_H
#define PROGETTOSO20_21_VMSUPPORT_H

extern unsigned int freeAsidBitmap;
extern unsigned int swapFloor;


int getVPNAddress(int index);

void init_uproc_pagetable(support_t * supp, int asid);

int getFreeAsid();

void initUprocPageTable(pcb_t *uproc);

void initSwapStructs();

void removePagesFromTLB(int asid);

void markReadOnlyPages(support_t *supp);

void handlePageFault();

void setSwapFloor();

#endif //PROGETTOSO20_21_VMSUPPORT_H