#ifndef PROGETTOSO20_21_SYSTEMCALLS_H
#define PROGETTOSO20_21_SYSTEMCALLS_H

#include "pandos_types.h"
#include "pandos_const.h"
#include "pcb.h"
#include "asl.h"

extern int *mutualExclusion;      

/* System calls handler */
void systemcallsHandler();

/* System calls 1-8 */
int create_Process();
void terminate_Process();
void passeren();
void verhogen();
int  wait_For_IO();
int get_Cpu_Time();
int wait_For_Clock();
support_t *get_support_data();


#endif //PROGETTOSO20_21_SYSTEMCALLS_H