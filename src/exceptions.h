#ifndef PROGETTOSO20_21_EXCEPTIONS_H
#define PROGETTOSO20_21_EXCEPTIONS_H


void handleExceptions();        //entry point for exceptions and interrupt handling

void TLBExceptionHandler();       //handler for exception related to TLB (it kill the process witch exception PGFAULTEXCEPT)

void passupOrDie(int exceptionType);     //detailed description in exception.c

#endif //PROGETTOSO20_21_EXCEPTIONS_H
