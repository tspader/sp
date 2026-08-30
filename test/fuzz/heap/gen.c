#include "fuzz.h"

sp_prng_profile_t* profile_new(sp_mem_t mem) {
  return sp_prng_profile_new(mem, (sp_prng_profile_desc_t) {
    .entries = {
      {
        .kind = SP_PRNG_KIND_SWARM,
        .name = "ops",
        .swarm = {
          .count = OP_COUNT,
          .bind = sp_prng_bind(profile_t, ops),
        },
      },
      {
        .kind = SP_PRNG_KIND_SWARM,
        .name = "sizes",
        .swarm = {
          .count = SIZE_COUNT,
          .bind = sp_prng_bind(profile_t, sizes),
        },
      },
      {
        .kind = SP_PRNG_KIND_CHANCE,
        .name = "big",
        .chance = {
          .numerator = 1,
          .denominator = 8,
          .bind = sp_prng_bind(profile_t, big),
        },
      },
      {
        .kind = SP_PRNG_KIND_RANGE,
        .name = "steps",
        .range = {
          .min = 1,
          .max = 64,
          .bind = sp_prng_bind(profile_t, steps),
        },
      },
      {
        .kind = SP_PRNG_KIND_RANGE,
        .name = "big_steps",
        .range = {
          .min = 128,
          .max = MAX_STEPS,
          .bind = sp_prng_bind(profile_t, big_steps),
        },
      },
      {
        .kind = SP_PRNG_KIND_RANGE,
        .name = "max_live",
        .range = {
          .min = 1,
          .max = 16,
          .bind = sp_prng_bind(profile_t, max_live),
        },
      },
      {
        .kind = SP_PRNG_KIND_RANGE,
        .name = "big_max_live",
        .range = {
          .min = 16,
          .max = MAX_SLOTS,
          .bind = sp_prng_bind(profile_t, big_max_live),
        },
      },
    }
  });
}

static bool is_allocating(op_kind_t op) {
  switch (op) {
    case OP_ALLOC:
    case OP_ALLOC_UNINIT: return true;
    case OP_FREE:
    case OP_REALLOC:
    case OP_REALLOC_UNINIT: return false;
    case OP_COUNT: break;
  }
  sp_unreachable_return(false);
}

static op_kind_t sample_op(sp_prng_t* prng, const profile_t* profile, u64 num_live, u64 max_live) {
  if (!num_live) {
    static const op_kind_t allocs [] = { OP_ALLOC, OP_ALLOC_UNINIT };
    u64 weights [] = { profile->ops[allocs[0]], profile->ops[allocs[1]] };
    return allocs[sp_prng_weighted(prng, weights, sp_carr_len(allocs))];
  }

  op_kind_t op = (op_kind_t)sp_prng_weighted(prng, profile->ops, OP_COUNT);
  if (num_live >= max_live && is_allocating(op)) return OP_FREE;
  return op;
}

static u64 sample_size(sp_prng_t* prng, const profile_t* profile) {
  switch ((size_class_t)sp_prng_weighted(prng, profile->sizes, SIZE_COUNT)) {
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

static u8 sample_fill(sp_prng_t* prng) {
  return (u8)sp_prng_range(prng, 1, 255);
}

static u32 sample_live(sp_prng_t* prng, const bool* live, u64 num_live) {
  u64 pick = sp_prng_below(prng, num_live);
  sp_for(it, MAX_SLOTS) {
    if (!live[it]) continue;
    if (!pick) return it;
    pick--;
  }
  sp_unreachable_return(0);
}

static u32 first_dead(const bool* live) {
  sp_for(it, MAX_SLOTS) {
    if (!live[it]) return it;
  }
  sp_unreachable_return(0);
}

trace_t gen_trace(sp_prng_t* prng, const sp_prng_profile_t* profile) {
  trace_t trace = sp_zero;
  sp_prng_generate(prng, profile, &trace.profile);
  u64 steps = trace.profile.big ? trace.profile.big_steps : trace.profile.steps;
  u64 max_live = trace.profile.big ? trace.profile.big_max_live : trace.profile.max_live;

  bool live [MAX_SLOTS] = sp_zero;
  u64 num_live = 0;

  sp_for(it, steps) {
    op_t op = { .kind = sample_op(prng, &trace.profile, num_live, max_live) };
    switch (op.kind) {
      case OP_ALLOC:
      case OP_ALLOC_UNINIT: {
        op.slot = first_dead(live);
        op.size = sample_size(prng, &trace.profile);
        op.fill = sample_fill(prng);
        live[op.slot] = true;
        num_live++;
        break;
      }
      case OP_FREE: {
        op.slot = sample_live(prng, live, num_live);
        live[op.slot] = false;
        num_live--;
        break;
      }
      case OP_REALLOC:
      case OP_REALLOC_UNINIT: {
        op.slot = sample_live(prng, live, num_live);
        op.size = sample_size(prng, &trace.profile);
        op.fill = sample_fill(prng);
        if (!op.size) {
          live[op.slot] = false;
          num_live--;
        }
        break;
      }
      case OP_COUNT: break;
    }
    trace.ops[trace.count++] = op;
  }
  return trace;
}

trace_t flip_trace(trace_t trace) {
  sp_for(it, trace.count) {
    op_t* op = &trace.ops[it];
    switch (op->kind) {
      case OP_ALLOC: op->kind = OP_ALLOC_UNINIT; break;
      case OP_ALLOC_UNINIT: op->kind = OP_ALLOC; break;
      case OP_REALLOC: op->kind = OP_REALLOC_UNINIT; break;
      case OP_REALLOC_UNINIT: op->kind = OP_REALLOC; break;
      case OP_FREE: break;
      case OP_COUNT: break;
    }
  }
  return trace;
}
