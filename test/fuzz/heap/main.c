#define SP_IMPLEMENTATION
#include "fuzz.h"
#include "sp/sp_cli.h"

static sp_str_t err_str(err_t err) {
  switch (err) {
    case ERR_OK: return sp_str_lit("ok");
    case ERR_NULL: return sp_str_lit("allocation unexpectedly returned null");
    case ERR_ZERO: return sp_str_lit("realloc to zero returned a pointer");
    case ERR_ALIGN: return sp_str_lit("misaligned pointer");
    case ERR_DIRTY: return sp_str_lit("fresh bytes are not zeroed");
    case ERR_TAIL: return sp_str_lit("chunk tail bytes are not zeroed");
    case ERR_BYTES: return sp_str_lit("live allocation bytes diverged from the model");
    case ERR_OVERLAP: return sp_str_lit("allocation overlaps a live allocation");
    case ERR_IDENTITY: return sp_str_lit("allocation does not resolve to its span or large");
    case ERR_ACCOUNTING: return sp_str_lit("bytes_used diverged from the model");
    case ERR_RESERVED: return sp_str_lit("bytes_reserved diverged from the model");
    case ERR_PEAK: return sp_str_lit("peak_reserved dropped below an observed watermark");
    case ERR_LEAK: return sp_str_lit("spans leaked from every list");
    case ERR_SPAN: return sp_str_lit("span invariant violated");
    case ERR_LARGE: return sp_str_lit("large list invariant violated");
    case ERR_DRAIN: return sp_str_lit("heap did not drain after freeing everything");
    case ERR_FLIP: return sp_str_lit("heap shape diverged under the zeroing flip");
    case ERR_COUNT: break;
  }
  sp_unreachable_return(sp_str_lit("unknown"));
}

static bool shape_eq(shape_t a, shape_t b) {
  return a.segments == b.segments &&
         a.spans == b.spans &&
         a.recycled == b.recycled &&
         a.larges == b.larges &&
         a.reserved == b.reserved &&
         a.peak == b.peak;
}

static err_t run_iteration(const sp_prng_profile_t* profile, sp_prng_t base, u64 iter) {
  sp_prng_t prng = sp_prng_iter(base, iter);
  trace_t trace = gen_trace(&prng, profile);
  trace_t flipped = flip_trace(trace);

  shape_t shape = sp_zero;
  shape_t flip_shape = sp_zero;
  try(run_trace(&trace, &shape));
  try(run_trace(&flipped, &flip_shape));
  must(shape_eq(shape, flip_shape), ERR_FLIP);
  return ERR_OK;
}

typedef struct {
  u64 iters;
  s64 iter;
  sp_str_t seed;
  bool keep_going;
} cli_t;

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
  sp_log("--seed 0x{:x}", sp_fmt_uint(seed));

  sp_prng_t prng = sp_prng_new(seed);
  sp_prng_profile_t* profile = profile_new(sp_mem_os_new());

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
