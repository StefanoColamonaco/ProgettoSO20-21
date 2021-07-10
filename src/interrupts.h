#ifndef PROGETTOSO20_21_INTERRUPTS_H
#define PROGETTOSO20_21_INTERRUPTS_H


extern unsigned int old_status;

inline void disable_interrupts();

inline void enable_interrupts();


/*interrupt handling entry point*/
void handleInterrupts();


/*UTILS*/

void releaseSemAndUpdateStatus(int deviceNo, unsigned int status);

unsigned int getSemIndex(unsigned int interruptLine, unsigned int deviceNo, int termIsRECV);

#endif