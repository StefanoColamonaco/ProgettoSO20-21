#include <umps3/umps/types.h>
#include "pandos_const.h"
#include "stateUtil.h"


void setStatusBitToValue(unsigned int *status, unsigned int bitPosition, unsigned int value) {
   *status = (*status << bitPosition) & value;
}

void copyStateInfo(state_t *src, state_t *dest){
    for (int i = 0; i < STATE_GPR_LEN; i++) {
        dest -> gpr[i] = src -> gpr[i];
    }
    dest -> pc_epc = src -> pc_epc;
    dest -> cause = src -> cause;
    dest -> status = src -> status;
    dest -> hi = src -> hi;
}
