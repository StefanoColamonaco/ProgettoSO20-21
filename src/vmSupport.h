#ifndef PROGETTOSO20_21_VMSUPPORT_H
#define PROGETTOSO20_21_VMSUPPORT_H

extern int freeAsidBitmap;

int getFreeAsid();

void initUprocPageTable(pcb_t *uproc);

void initSwapPool();

#endif //PROGETTOSO20_21_VMSUPPORT_H