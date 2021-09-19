#include "initSupp.h"

#include "pandos_const.h"
#include "pandos_types.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/regdef.h>
#include <umps3/umps/arch.h>
#include <umps3/umps/cp0.h>



static support_t* freeSupp_tp = NULL;

support_t* alloc_supp()
{
    if (freeSupp_tp == NULL) {
        return NULL;
    }

    support_t* supp = freeSupp_tp;
    freeSupp_tp = freeSupp_tp->prev_supp;
    freeSupp_tp->next_supp = NULL;

    supp->next_supp = supp->prev_supp = NULL;

    return supp;

}

void dealloc_supp(support_t* supp)
{
    if (supp == NULL) {
        return;
    } else if (freeSupp_tp == NULL) {
        freeSupp_tp = supp;
        supp->next_supp = supp->prev_supp = NULL;
    } else {
        supp->next_supp = NULL;
        supp->prev_supp = freeSupp_tp;
        freeSupp_tp->next_supp = supp;
        freeSupp_tp = supp;
    }
}


