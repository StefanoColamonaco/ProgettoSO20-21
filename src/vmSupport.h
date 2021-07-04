#ifndef PROGETTOSO20_21_VMSUPPORT_H
#define PROGETTOSO20_21_VMSUPPORT_H

extern unsigned int freeAsidBitmap;
extern unsigned int swapFloor;


int getVPNAddress(int index);

void init_uproc_pagetable(support_t * supp);

int getFreeAsid();

void initUprocPageTable(pcb_t *uproc);

void initSwapStructs();

void removePagesFromTLB(int asid);

void markReadOnlyPages(support_t *supp);

void setSwapFloor();

#endif //PROGETTOSO20_21_VMSUPPORT_H