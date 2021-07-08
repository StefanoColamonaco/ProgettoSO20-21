#include <umps3/umps/types.h>
#include "pandos_const.h"
#include "stateUtil.h"


/*Copy state from a source process to a target process*/
void copyState(state_t *src, state_t *dest){
    for (int i = 0; i < STATE_GPR_LEN; i++) {
        dest -> gpr[i] = src -> gpr[i];
    }
    dest -> entry_hi = src -> entry_hi;
    dest -> pc_epc = src -> pc_epc;
    dest -> cause = src -> cause;
    dest -> status = src -> status;
    dest -> hi = src -> hi;
}
