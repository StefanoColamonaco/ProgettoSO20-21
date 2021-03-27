#ifndef PROGETTOSO20_21_STATEUTIL_H
#define PROGETTOSO20_21_STATEUTIL_H


#include <umps3/umps/types.h>

void copyStateInfo(state_t *src, state_t *dest);      /* change a state from a source to a destination state* */

void setStatusBitToValue(unsigned int *status, unsigned int bitPosition, unsigned int value);

#endif //PROGETTOSO20_21_STATEUTIL_H
