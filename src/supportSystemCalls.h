#ifndef PROGETTOSO20_21_SUPPORTSYSCALLS_H
#define PROGETTOSO20_21_SUPPORTSYSCALLS_H

void handleSupportSystemcalls();

#define PRINTCHR 2
#define RECVD 5
#define TERMSTATMASK 0xFF

/* System caòòs for support evel */

void terminate_support();

void get_TOD();

void write_To_Printer();

void write_To_Terminal();

void read_From_Terminal();

#endif //PROGETTOSO20_21_SUPPORTSYSCALLS_H