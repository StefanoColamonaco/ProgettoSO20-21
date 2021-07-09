#ifndef PROGETTOSO20_21_STATEUTIL_H
#define PROGETTOSO20_21_STATEUTIL_H

#include <umps3/umps/types.h>

typedef __SIZE_TYPE__ size_t;

/*Copy state from a source process to a target process*/
void copyState(state_t *src, state_t *dest);     

void * memcpy (void *dest, const void *src, size_t len); 


#endif //PROGETTOSO20_21_STATEUTIL_H
