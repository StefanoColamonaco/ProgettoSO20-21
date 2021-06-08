#ifndef PRINT_H
#define PRINT_H

#define PRINTCHR	2
#define BYTELEN	8
#define RECVD	5

#define	CREATETHREAD	1	/* create thread */
#define	TERMINATETHREAD	2	/* terminate thread */
#define	PASSERN			3	/* P a semaphore */
#define	VERHOGEN		4	/* V a semaphore */
#define	WAITIO			5	/* delay on a io semaphore */
#define	GETCPUTIME		6	/* get cpu time used to date */
#define	WAITCLOCK		7	/* delay on the clock semaphore */
#define	GETSPTPTR		8	/* return support structure ptr. */

#define PRINTCHR	2
#define BYTELEN	8
#define RECVD	5

#define CLOCKINTERVAL	100000UL	/* interval to V clock semaphore */

#define TERMSTATMASK	0xFF
#define CAUSEMASK		0xFF
#define VMOFF 			0xF8FFFFFF

#define SYSCAUSE		(0x8 << 2)
#define BUSERROR		6
#define RESVINSTR   	10
#define ADDRERROR		4
#define SYSCALLEXCPT	8

#define QPAGE			1024

#define IEPBITON		0x4
#define KUPBITON		0x8
#define KUPBITOFF		0xFFFFFFF7
#define TEBITON			0x08000000

#define CAUSEINTMASK	0xFD00
#define CAUSEINTOFFS	10

#define MINLOOPTIME		30000
#define LOOPNUM 		10000

#define CLOCKLOOP		10
#define MINCLOCKLOOP	3000	

#define BADADDR			0xFFFFFFFF
#define	TERM0ADDR		0x10000254



#define CREATENOGOOD	-1

/* just to be clear */
#define SEMAPHORE		int
#define NOLEAVES		4	/* number of leaves of p8 process tree */
#define MAXSEM			20

typedef unsigned int devregtr;

int term_mut;

void print(char *msg);

#endif // !PRINT_H


