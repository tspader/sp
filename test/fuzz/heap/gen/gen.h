#ifndef FUZZ_HEAP_GEN_H
#define FUZZ_HEAP_GEN_H

#include "types.h"

sp_prng_profile_t* profile_new(sp_mem_t mem);
u64                gen_state(state_t* state, const sp_prng_profile_t* profile);
op_t               sample_op(state_t* state);
u64                gen_size(state_t* state);
u8                 gen_fill(state_t* state);
slot_t*            get_live_slot(state_t* state);

#endif
