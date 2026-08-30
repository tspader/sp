#include "gen/gen.h"


u64 gen_state(state_t* state, const sp_prng_profile_t* profile) {
  sp_prng_generate(&state->prng, profile, &state->profile);
  state->max_live = (u32)(state->profile.big ? state->profile.big_max_live : state->profile.max_live);
  return state->profile.big ? state->profile.big_steps : state->profile.steps;
}

static bool is_allocating(op_t op) {
  switch (op) {
    case OP_ALLOC:
    case OP_ALLOC_UNINIT:
      return true;
    case OP_FREE:
    case OP_REALLOC:
    case OP_REALLOC_UNINIT:
      return false;
    case OP_COUNT: break;
  }
  sp_unreachable_return(false);
}

op_t sample_op(state_t* state) {
  if (!state->live) {
    static const op_t allocs [] = { OP_ALLOC, OP_ALLOC_UNINIT };
    u64 weights [] = { state->profile.ops[allocs[0]], state->profile.ops[allocs[1]] };
    return allocs[sp_prng_weighted(&state->prng, weights, sp_carr_len(allocs))];
  }

  op_t op = (op_t)sp_prng_weighted(&state->prng, state->profile.ops, OP_COUNT);
  if (state->live >= state->max_live && is_allocating(op)) return OP_FREE;
  return op;
}

u64 gen_size(state_t* state) {
  sp_prng_t* prng = &state->prng;
  switch ((size_class_t)sp_prng_weighted(prng, state->profile.sizes, SIZE_COUNT)) {
    case SIZE_TINY: return sp_prng_range(prng, 0, 16);
    case SIZE_SMALL: return sp_prng_range(prng, 1, SP_MEM_HEAP_MAX_SMALL);
    case SIZE_EDGE: {
      u64 bucket = sp_mem_heap_bucket_size((u32)sp_prng_below(prng, SP_MEM_HEAP_NUM_BUCKETS));
      return bucket - 1 + sp_prng_range(prng, 0, 2);
    }
    case SIZE_LARGE: return sp_prng_range(prng, SP_MEM_HEAP_MAX_SMALL + 1, 4 * SP_MEM_HEAP_SPAN_SIZE);
    case SIZE_HUGE: return sp_prng_range(prng, 4 * SP_MEM_HEAP_SPAN_SIZE, 2 * SP_MEM_HEAP_SEGMENT_SIZE);
    case SIZE_COUNT: break;
  }
  sp_unreachable_return(0);
}

u8 gen_fill(state_t* state) {
  return (u8)sp_prng_range(&state->prng, 1, 255);
}

slot_t* get_live_slot(state_t* state) {
  u32 pick = (u32)sp_prng_below(&state->prng, state->live);
  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) continue;
    if (!pick) return &state->slots[it];
    pick--;
  }
  sp_unreachable_return(SP_NULLPTR);
}
