#ifndef PROGETTOSO20_21_SYSTEMCALLS_H
#define PROGETTOSO20_21_SYSTEMCALLS_H

#include "pandos_types.h"
#include "pandos_const.h"
#include "pcb.h"
#include "asl.h"

extern int *mutex;      

/* System calls handler */

void handleSystemcalls();


/* System calls 1-8 */

void  create_Process();

void terminate_Process();

void passeren();

void verhogen();

void  wait_For_IO();

void get_Cpu_Time();

void wait_For_Clock();


/*helper functions*/

void get_support_data();

void blockCurrentProcessAt(int *sem);


#endif //PROGETTOSO20_21_SYSTEMCALLS_H