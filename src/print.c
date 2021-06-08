#include "print.h"
#include <umps3/umps/const.h>

int term_mut = 1;



void print(char *msg) {

	char *s = msg;
	devregtr * base = (devregtr *) (TERM0ADDR);
	devregtr status;
	
	SYSCALL(PASSERN, (int)&term_mut, 0, 0);			/* P(term_mut) */
	while (*s != EOS) {
		*(base + 3) = PRINTCHR | (((devregtr) *s) << BYTELEN);
		status = SYSCALL(WAITIO, TERMINT, 0, 0);	
		if ((status & TERMSTATMASK) != RECVD)
			PANIC();
		s++;	
	}
	SYSCALL(VERHOGEN, (int)&term_mut, 0, 0);				/* V(term_mut) */
}