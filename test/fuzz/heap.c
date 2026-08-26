#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_cli.h"
#include "sp/sp_prng.h"

#define MAX_SLOTS 64

typedef enum {
  ERR_OK,
  ERR_NULL,
  ERR_ZERO,
  ERR_ALIGN,
  ERR_DIRTY,
  ERR_BYTES,
  ERR_ACCOUNTING,
  ERR_SPAN,
  ERR_LARGE,
  ERR_DRAIN,
  ERR_COUNT,
} err_t;

typedef enum {
  OP_ALLOC,
  OP_FREE,
  OP_REALLOC,
  OP_COUNT,
} op_t;

typedef enum {
  SIZE_TINY,
  SIZE_SMALL,
  SIZE_EDGE,
  SIZE_LARGE,
  SIZE_HUGE,
  SIZE_COUNT,
} size_class_t;

typedef struct {
  u64 ops [OP_COUNT];
  u64 sizes [SIZE_COUNT];
  bool big;
  u64 steps;
  u64 big_steps;
  u64 max_live;
  u64 big_max_live;
} profile_t;

typedef struct {
  u8* ptr;
  u64 size;
  u8 fill;
  bool live;
} slot_t;

typedef struct {
  sp_prng_t prng;
  profile_t profile;
  u64 steps;
  u32 max_live;
  sp_mem_heap_t* heap;
  slot_t slots [MAX_SLOTS];
  u32 live;
} state_t;

static sp_str_t err_str(err_t err) {
  switch (err) {
    case ERR_OK: return sp_str_lit("ok");
    case ERR_NULL: return sp_str_lit("allocation unexpectedly returned null");
    case ERR_ZERO: return sp_str_lit("realloc to zero returned a pointer");
    case ERR_ALIGN: return sp_str_lit("misaligned pointer");
    case ERR_DIRTY: return sp_str_lit("fresh bytes are not zeroed");
    case ERR_BYTES: return sp_str_lit("live allocation bytes diverged from the model");
    case ERR_ACCOUNTING: return sp_str_lit("bytes_used diverged from the model");
    case ERR_SPAN: return sp_str_lit("span invariant violated");
    case ERR_LARGE: return sp_str_lit("large list invariant violated");
    case ERR_DRAIN: return sp_str_lit("heap did not drain after freeing everything");
    case ERR_COUNT: break;
  }
  sp_unreachable_return(sp_str_lit("unknown"));
}

static u64 gen_size(sp_prng_t* prng, const profile_t* profile) {
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

static bool bytes_are(const u8* bytes, u64 len, u8 want) {
  sp_for(it, len) {
    if (bytes[it] != want) return false;
  }
  return true;
}

static u64 expected_bytes(state_t* state) {
  u64 total = 0;
  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) continue;
    u64 size = state->slots[it].size;
    u32 bucket = sp_mem_heap_bucket_of(size);
    total += bucket < SP_MEM_HEAP_NUM_BUCKETS ? sp_mem_heap_bucket_size(bucket) : size;
  }
  return total;
}

static err_t check_span(sp_mem_heap_t* heap, sp_mem_heap_span_t* span, u32 bucket, bool full) {
  if (span->magic != SP_MEM_HEAP_SPAN_MAGIC) return ERR_SPAN;
  if (span->bucket != bucket) return ERR_SPAN;
  if (span->heap != heap) return ERR_SPAN;
  if (!span->in_use) return ERR_SPAN;
  if (full && span->free_head) return ERR_SPAN;
  if (!full && !span->free_head) return ERR_SPAN;

  u64 bucket_size = sp_mem_heap_bucket_size(bucket);
  u8* base = (u8*)span + sizeof(sp_mem_heap_span_t);
  u8* end = (u8*)span + SP_MEM_HEAP_SPAN_SIZE;
  u32 num_chunks = (u32)((u64)(end - base) / bucket_size);
  if (span->in_use > num_chunks) return ERR_SPAN;

  u32 free_chunks = 0;
  for (void* chunk = span->free_head; chunk; chunk = *(void**)chunk) {
    if ((u8*)chunk < base || (u8*)chunk >= end) return ERR_SPAN;
    if ((u64)((u8*)chunk - base) % bucket_size) return ERR_SPAN;
    if (++free_chunks > num_chunks) return ERR_SPAN;
  }
  if (span->in_use + free_chunks != num_chunks) return ERR_SPAN;
  return ERR_OK;
}

static err_t check_heap(state_t* state) {
  sp_mem_heap_t* heap = state->heap;

  u64 used = 0;
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    sp_mem_heap_bucket_t* lists = &heap->buckets[bucket];
    if (lists->partial && lists->partial->prev) return ERR_SPAN;
    if (lists->full && lists->full->prev) return ERR_SPAN;

    for (sp_mem_heap_span_t* span = lists->partial; span; span = span->next) {
      err_t err = check_span(heap, span, bucket, false);
      if (err) return err;
      if (span->next && span->next->prev != span) return ERR_SPAN;
      used += (u64)span->in_use * sp_mem_heap_bucket_size(bucket);
    }
    for (sp_mem_heap_span_t* span = lists->full; span; span = span->next) {
      err_t err = check_span(heap, span, bucket, true);
      if (err) return err;
      if (span->next && span->next->prev != span) return ERR_SPAN;
      used += (u64)span->in_use * sp_mem_heap_bucket_size(bucket);
    }
  }

  if (heap->recycled && heap->recycled->prev) return ERR_SPAN;
  for (sp_mem_heap_span_t* span = heap->recycled; span; span = span->next) {
    if (span->magic == SP_MEM_HEAP_SPAN_MAGIC) return ERR_SPAN;
  }

  if (heap->larges && heap->larges->prev) return ERR_LARGE;
  for (sp_mem_heap_large_t* large = heap->larges; large; large = large->next) {
    if (large->magic != SP_MEM_HEAP_LARGE_MAGIC) return ERR_LARGE;
    if (large->heap != heap) return ERR_LARGE;
    if (large->next && large->next->prev != large) return ERR_LARGE;
    used += large->size;
  }

  if (used != heap->bytes_used) return ERR_ACCOUNTING;
  if (used != expected_bytes(state)) return ERR_ACCOUNTING;
  return ERR_OK;
}

static u32 pick_live(state_t* state) {
  u32 pick = (u32)sp_prng_below(&state->prng, state->live);
  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) continue;
    if (!pick) return it;
    pick--;
  }
  sp_unreachable_return(0);
}

static u32 pick_dead(state_t* state) {
  sp_carr_for(state->slots, it) {
    if (!state->slots[it].live) return it;
  }
  sp_unreachable_return(0);
}

static err_t op_alloc(state_t* state) {
  u64 size = gen_size(&state->prng, &state->profile);
  bool uninitialized = sp_prng_chance(&state->prng, 1, 8);

  u8* ptr = (u8*)(uninitialized
    ? sp_mem_heap_alloc_uninitialized(state->heap, size)
    : sp_mem_heap_alloc(state->heap, size));
  if (!ptr) return ERR_NULL;
  if (sp_align_up(ptr, SP_MEM_ALIGNMENT) != sp_uptr(ptr)) return ERR_ALIGN;
  if (!uninitialized && !bytes_are(ptr, size, 0)) return ERR_DIRTY;

  u8 fill = (u8)sp_prng_range(&state->prng, 1, 255);
  sp_mem_fill_u8(ptr, size, fill);
  state->slots[pick_dead(state)] = (slot_t) { .ptr = ptr, .size = size, .fill = fill, .live = true };
  state->live++;
  return ERR_OK;
}

static err_t op_free(state_t* state) {
  slot_t* slot = &state->slots[pick_live(state)];
  if (!bytes_are(slot->ptr, slot->size, slot->fill)) return ERR_BYTES;
  sp_mem_heap_free(state->heap, slot->ptr);
  *slot = sp_zero_s(slot_t);
  state->live--;
  return ERR_OK;
}

static err_t op_realloc(state_t* state) {
  slot_t* slot = &state->slots[pick_live(state)];
  u64 size = gen_size(&state->prng, &state->profile);
  bool uninitialized = sp_prng_chance(&state->prng, 1, 8);

  if (!bytes_are(slot->ptr, slot->size, slot->fill)) return ERR_BYTES;

  u8* ptr = (u8*)(uninitialized
    ? sp_mem_heap_realloc_uninitialized(state->heap, slot->ptr, size)
    : sp_mem_heap_realloc(state->heap, slot->ptr, size));

  if (!size) {
    if (ptr) return ERR_ZERO;
    *slot = sp_zero_s(slot_t);
    state->live--;
    return ERR_OK;
  }

  if (!ptr) return ERR_NULL;
  if (sp_align_up(ptr, SP_MEM_ALIGNMENT) != sp_uptr(ptr)) return ERR_ALIGN;
  if (!bytes_are(ptr, sp_min(slot->size, size), slot->fill)) return ERR_BYTES;
  if (!uninitialized && size > slot->size && !bytes_are(ptr + slot->size, size - slot->size, 0)) return ERR_DIRTY;

  u8 fill = (u8)sp_prng_range(&state->prng, 1, 255);
  sp_mem_fill_u8(ptr, size, fill);
  *slot = (slot_t) { .ptr = ptr, .size = size, .fill = fill, .live = true };
  return ERR_OK;
}

static err_t step(state_t* state) {
  op_t op = (op_t)sp_prng_weighted(&state->prng, state->profile.ops, OP_COUNT);
  if (!state->live) op = OP_ALLOC;
  else if (state->live >= state->max_live && op == OP_ALLOC) op = OP_FREE;

  switch (op) {
    case OP_ALLOC: return op_alloc(state);
    case OP_FREE: return op_free(state);
    case OP_REALLOC: return op_realloc(state);
    case OP_COUNT: break;
  }
  sp_unreachable_return(ERR_OK);
}

static err_t drain(state_t* state) {
  sp_carr_for(state->slots, it) {
    slot_t* slot = &state->slots[it];
    if (!slot->live) continue;
    if (!bytes_are(slot->ptr, slot->size, slot->fill)) return ERR_BYTES;
    sp_mem_heap_free(state->heap, slot->ptr);
    *slot = sp_zero_s(slot_t);
    state->live--;
  }

  if (state->heap->bytes_used) return ERR_DRAIN;
  if (state->heap->larges) return ERR_DRAIN;
  sp_for(bucket, SP_MEM_HEAP_NUM_BUCKETS) {
    if (state->heap->buckets[bucket].partial) return ERR_DRAIN;
    if (state->heap->buckets[bucket].full) return ERR_DRAIN;
  }
  return ERR_OK;
}

static err_t run_iteration(const sp_prng_profile_t* profile, sp_prng_t base, u64 iter) {
  state_t state = sp_zero;
  state.prng = sp_prng_iter(base, iter);
  sp_prng_generate(&state.prng, profile, &state.profile);

  state.steps = state.profile.big ? state.profile.big_steps : state.profile.steps;
  state.max_live = (u32)(state.profile.big ? state.profile.big_max_live : state.profile.max_live);
  state.heap = sp_mem_heap_new();

  err_t err = ERR_OK;
  for (u64 it = 0; it < state.steps && !err; it++) {
    err = step(&state);
    if (!err) err = check_heap(&state);
  }
  if (!err) err = drain(&state);
  if (!err) err = check_heap(&state);

  sp_mem_heap_destroy(state.heap);
  return err;
}

typedef struct {
  u64 iters;
  s64 iter;
  sp_str_t seed;
  bool keep_going;
} cli_t;

static sp_cli_result_t cli_handler(sp_cli_t* cli) {
  sp_unused(cli);
  return SP_CLI_CONTINUE;
}

static sp_err_t fmt_hex(sp_io_writer_t* io, sp_fmt_arg_t* arg) {
  return sp_fmt_write_u64_ex(io, arg->value.u, SP_FMT_RADIX_HEX);
}

static s32 entry(s32 num_args, const c8** args) {
  cli_t opts = {
    .iters = 512,
    .iter = -1,
    .seed = sp_str_lit("")
  };

  sp_cli_cmd_t cmd = {
    .name = "fuzz_heap",
    .summary = "deterministic fuzzer for the heap allocator",
    .opts = {
      {
        .brief = 'n',
        .name = "iters",
        .kind = SP_CLI_OPT_U64,
        .summary = "number of iterations to run",
        .placeholder = "n",
        .ptr = &opts.iters,
      },
      {
        .brief = 'i',
        .name = "iter",
        .kind = SP_CLI_OPT_S64,
        .summary = "run a single iteration",
        .placeholder = "iter",
        .ptr = &opts.iter,
      },
      {
        .brief = 's',
        .name = "seed",
        .kind = SP_CLI_OPT_STR,
        .summary = "seed as decimal or 0x hex; random when unset",
        .placeholder = "seed",
        .ptr = &opts.seed,
      },
      {
        .brief = 'k',
        .name = "keep-going",
        .kind = SP_CLI_OPT_BOOLEAN,
        .summary = "run every iteration instead of stopping at the first failure",
        .ptr = &opts.keep_going,
      },
    },
    .handler = cli_handler,
  };

  switch (sp_cli_run((sp_cli_desc_t) { .root = &cmd, .args = args, .num_args = num_args })) {
    case SP_CLI_OK:
    case SP_CLI_CONTINUE: break;
    case SP_CLI_HELP: return 0;
    case SP_CLI_ERR: return 1;
  }

  u64 seed = 0;
  if (sp_str_empty(opts.seed)) {
    seed = sp_prng_entropy();
  }
  else if (!sp_prng_parse_seed(opts.seed, &seed)) {
    sp_log("fuzz: bad seed {}", sp_fmt_str(opts.seed));
    return 1;
  }
  sp_log("--seed 0x{}", sp_fmt_u64_custom(seed, fmt_hex));

  sp_mem_heap_t* heap = sp_mem_heap_new();
  sp_mem_t mem = sp_mem_heap_as_allocator(heap);
  sp_prng_t prng = sp_prng_new(seed);

  sp_prng_profile_t* profile = sp_prng_profile_new(mem, (sp_prng_profile_desc_t) {
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
          .max = 2048,
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
    },
  });

  u64 first = opts.iter >= 0 ? (u64)opts.iter : 0;
  u64 last = opts.iter >= 0 ? (u64)opts.iter + 1 : opts.iters;
  u32 failures = 0;
  err_t status = ERR_OK;

  for (u64 it = first; it < last; it++) {
    err_t err = run_iteration(profile, prng, it);
    if (!err) continue;

    failures++;
    if (!status) status = err;
    sp_log("fuzz: {} (repro: --iter {})", sp_fmt_str(err_str(err)), sp_fmt_uint(it));
    if (!opts.keep_going) break;
  }

  if (failures) {
    sp_log("fuzz: {} of {} iterations failed", sp_fmt_uint(failures), sp_fmt_uint(last - first));
  }
  return opts.keep_going && failures ? 1 : (s32)status;
}

SP_MAIN(entry)
