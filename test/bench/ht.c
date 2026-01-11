#define SP_APP
#include "sp.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct {
  s32 key;
  u64 value;
} kvp_t;

#define BENCH_WARMUP_RUNS 2
#define BENCH_TIMED_RUNS  5

#define SP_COMPILER_BARRIER() __asm__ volatile("" ::: "memory")

static u32 xorshift32(u32* state) {
  u32 x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

static sp_str_t rgb_fg(u8 r, u8 g, u8 b) {
  return sp_format("\x1b[38;2;{};{};{}m", SP_FMT_U8(r), SP_FMT_U8(g), SP_FMT_U8(b));
}

static sp_str_t color_for_ratio(f64 ratio) {
  f64 hue = ratio < 1.0 ? 120.0f : 0.0f;
  f64 distance = ratio < 1.0 ? (1.0 / ratio) - 1.0 : ratio - 1.0;
  f64 saturation = sp_clamp(distance / 3.0, 0.0, 1.0) * 100.0f;
  sp_color_t hsv = { .r = (f32)hue, .g = (f32)saturation, .b = 60.0f, .a = 1.0f };
  sp_color_t rgb = sp_color_hsv_to_rgb(hsv);
  return rgb_fg((u8)(rgb.r * 255), (u8)(rgb.g * 255), (u8)(rgb.b * 255));
}

typedef enum {
  BENCH_LIB_SP,
  BENCH_LIB_STB,
} bench_lib_t;

typedef struct {
  u32 n;
  bench_lib_t lib;
} bench_params_t;

typedef struct {
  sp_ht(s32, u64) sp;
  kvp_t* stb;
  s32* random_keys;
  u32 random_keys_n;
} bench_data_t;

SP_TYPEDEF_FN(void, bench_kernel_t, bench_params_t, bench_data_t*);

typedef struct {
  struct {
    bench_kernel_t init;
    bench_kernel_t deinit;
    bench_kernel_t test;
  } kernels;
  u32 iterations[8];
  sp_str_t name;
} bench_t;

typedef struct {
  sp_str_t name;
  u32 n;
  u64 sp_time_ns;
  u64 stb_time_ns;
} bench_result_pair_t;

static s32 compare_result_pairs(const void* a, const void* b) {
  const bench_result_pair_t* ra = (const bench_result_pair_t*)a;
  const bench_result_pair_t* rb = (const bench_result_pair_t*)b;
  s32 cmp = sp_str_compare_alphabetical(ra->name, rb->name);
  if (cmp != SP_QSORT_EQUAL) return cmp;
  if (ra->n < rb->n) return SP_QSORT_A_FIRST;
  if (ra->n > rb->n) return SP_QSORT_B_FIRST;
  return SP_QSORT_EQUAL;
}

static u64 run_single_bench(bench_t* bench, bench_params_t params, bench_data_t* data) {
  sp_for(w, BENCH_WARMUP_RUNS) {
    if (bench->kernels.init) bench->kernels.init(params, data);
    bench->kernels.test(params, data);
    if (bench->kernels.deinit) bench->kernels.deinit(params, data);
  }

  u64 best_time = UINT64_MAX;
  sp_for(r, BENCH_TIMED_RUNS) {
    if (bench->kernels.init) bench->kernels.init(params, data);

    sp_tm_timer_t timer = sp_tm_start_timer();
    bench->kernels.test(params, data);
    u64 elapsed = sp_tm_read_timer(&timer);

    if (elapsed < best_time) best_time = elapsed;

    if (bench->kernels.deinit) bench->kernels.deinit(params, data);
  }

  return best_time;
}

static void run_benchmarks(bench_t* benches, u32 num_benches) {
  sp_da(bench_result_pair_t) results = SP_NULLPTR;
  bench_data_t data = SP_ZERO_INITIALIZE();

  sp_for(bi, num_benches) {
    bench_t* bench = &benches[bi];
    SP_ASSERT(bench->kernels.test);

    sp_for(it, 8) {
      u32 n = bench->iterations[it];
      if (!n) break;

      u64 sp_time = run_single_bench(bench, (bench_params_t){ .n = n, .lib = BENCH_LIB_SP }, &data);
      u64 stb_time = run_single_bench(bench, (bench_params_t){ .n = n, .lib = BENCH_LIB_STB }, &data);

      sp_dyn_array_push(results, ((bench_result_pair_t){
        .name = bench->name,
        .n = n,
        .sp_time_ns = sp_time,
        .stb_time_ns = stb_time,
      }));
    }
  }

  qsort(results, sp_dyn_array_size(results), sizeof(bench_result_pair_t), compare_result_pairs);

  u32 max_name = 4;
  u32 max_n_width = 1;
  u32 time_width = 12;
  u32 ratio_width = 6;
  sp_dyn_array_for(results, i) {
    if (results[i].name.len > max_name) max_name = results[i].name.len;
    sp_str_t n_str = sp_format("{}", SP_FMT_U32(results[i].n));
    if (n_str.len > max_n_width) max_n_width = n_str.len;
  }

  sp_str_builder_t sb = SP_ZERO_INITIALIZE();
  sp_str_builder_append_fmt(&sb, "{}{} {} {} {} {}{}\n",
    SP_FMT_CSTR(SP_ANSI_FG_BRIGHT_BLACK),
    SP_FMT_STR(sp_str_pad(SP_LIT("test"), max_name)),
    SP_FMT_STR(sp_str_pad(SP_LIT("n"), max_n_width)),
    SP_FMT_STR(sp_str_pad(SP_LIT("sp_ht"), time_width)),
    SP_FMT_STR(sp_str_pad(SP_LIT("stb_ds"), time_width)),
    SP_FMT_STR(sp_str_pad(SP_LIT("ratio"), ratio_width)),
    SP_FMT_CSTR(SP_ANSI_RESET));

  sp_dyn_array_for(results, i) {
    bench_result_pair_t* r = &results[i];

    sp_str_t n_str = sp_format("{}", SP_FMT_U32(r->n));
    f64 sp_ms = sp_tm_ns_to_ms_f((f64)r->sp_time_ns);
    f64 stb_ms = sp_tm_ns_to_ms_f((f64)r->stb_time_ns);
    f64 ratio = sp_ms / stb_ms;

    sp_str_t sp_time_str = sp_str_pad(sp_format("{}ms", SP_FMT_F64(sp_ms)), time_width);
    sp_str_t stb_time_str = sp_str_pad(sp_format("{}ms", SP_FMT_F64(stb_ms)), time_width);
    sp_str_t ratio_color = color_for_ratio(ratio);
    sp_str_t ratio_str = sp_str_pad(sp_format("{}x", SP_FMT_F64(ratio)), ratio_width);

    sp_str_builder_append_fmt(&sb, "{} {} {} {} {}{}{}\n",
      SP_FMT_STR(sp_str_pad(r->name, max_name)),
      SP_FMT_STR(sp_str_pad(n_str, max_n_width)),
      SP_FMT_STR(sp_time_str),
      SP_FMT_STR(stb_time_str),
      SP_FMT_STR(ratio_color),
      SP_FMT_STR(ratio_str),
      SP_FMT_CSTR(SP_ANSI_RESET));
  }

  sp_str_t output = sp_str_builder_to_str(&sb);
  sp_os_print(output);
}

static void kernel_deinit(bench_params_t p, bench_data_t* data) {
  (void)p;
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_ht_free(data->sp);
      data->sp = SP_NULLPTR;
      break;
    }
    case BENCH_LIB_STB: {
      hmfree(data->stb);
      data->stb = SP_NULLPTR;
      break;
    }
  }
}

static void kernel_seq_insert(bench_params_t p, bench_data_t* data) {
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_insert(data->sp, i, i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmput(data->stb, i, i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
  }
}

static void kernel_seq_lookup_init(bench_params_t p, bench_data_t* data) {
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_insert(data->sp, (s32)i, (u64)i);
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmput(data->stb, (s32)i, (u64)i);
      }
      break;
    }
  }
}

static void kernel_seq_lookup(bench_params_t p, bench_data_t* data) {
  u64 sum = 0;
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        u64* v = sp_ht_getp(data->sp, (s32)i);
        if (v) sum += *v;
        SP_COMPILER_BARRIER();
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        sum += hmget(data->stb, (s32)i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
  }
  SP_COMPILER_BARRIER();
  (void)sum;
}

static void kernel_rnd_init(bench_params_t p, bench_data_t* data) {
  if (data->random_keys_n >= p.n) return;
  data->random_keys = sp_alloc(sizeof(s32) * p.n);
  u32 state = 12345;
  sp_for(i, p.n) {
    data->random_keys[i] = (s32)xorshift32(&state);
  }
  data->random_keys_n = p.n;
}

static void kernel_rnd_insert(bench_params_t p, bench_data_t* data) {
  kernel_rnd_init(p, data);
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_insert(data->sp, data->random_keys[i], (u64)i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmput(data->stb, data->random_keys[i], (u64)i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
  }
}

static void kernel_rnd_lookup_init(bench_params_t p, bench_data_t* data) {
  kernel_rnd_init(p, data);
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_insert(data->sp, data->random_keys[i], (u64)i);
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmput(data->stb, data->random_keys[i], (u64)i);
      }
      break;
    }
  }
}

static void kernel_rnd_lookup(bench_params_t p, bench_data_t* data) {
  u64 sum = 0;
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        u64* v = sp_ht_getp(data->sp, data->random_keys[i]);
        if (v) sum += *v;
        SP_COMPILER_BARRIER();
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        sum += hmget(data->stb, data->random_keys[i]);
        SP_COMPILER_BARRIER();
      }
      break;
    }
  }
  SP_COMPILER_BARRIER();
  (void)sum;
}

static void kernel_delete_init(bench_params_t p, bench_data_t* data) {
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_insert(data->sp, (s32)i, (u64)i);
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmput(data->stb, (s32)i, (u64)i);
      }
      break;
    }
  }
}

static void kernel_delete(bench_params_t p, bench_data_t* data) {
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_erase(data->sp, (s32)i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmdel(data->stb, (s32)i);
        SP_COMPILER_BARRIER();
      }
      break;
    }
  }
}

static void kernel_mixed(bench_params_t p, bench_data_t* data) {
  u64 sum = 0;
  switch (p.lib) {
    case BENCH_LIB_SP: {
      sp_for(i, p.n) {
        sp_ht_insert(data->sp, (s32)i, (u64)i);
        if (i > 0) {
          u64* v = sp_ht_getp(data->sp, (s32)(i / 2));
          if (v) sum += *v;
        }
        SP_COMPILER_BARRIER();
      }
      break;
    }
    case BENCH_LIB_STB: {
      sp_for(i, p.n) {
        hmput(data->stb, (s32)i, (u64)i);
        if (i > 0) {
          s32 index = (s32)(i / 2);
          sum += hmget(data->stb, index);
        }
        SP_COMPILER_BARRIER();
      }
      break;
    }
  }
  SP_COMPILER_BARRIER();
  (void)sum;
}

#define DEFAULT_ITERS 10, 100, 1000, 100000

s32 main(s32 argc, const c8** argv) {
  (void)argc;
  (void)argv;

  bench_t benches[] = {
    {
      .kernels = { .deinit = kernel_deinit, .test = kernel_seq_insert },
      .iterations = { DEFAULT_ITERS },
      .name = SP_LIT("seq_insert"),
    },
    {
      .kernels = { .init = kernel_seq_lookup_init, .deinit = kernel_deinit, .test = kernel_seq_lookup },
      .iterations = { DEFAULT_ITERS },
      .name = SP_LIT("seq_lookup"),
    },
    {
      .kernels = { .deinit = kernel_deinit, .test = kernel_rnd_insert },
      .iterations = { DEFAULT_ITERS },
      .name = SP_LIT("rnd_insert"),
    },
    {
      .kernels = { .init = kernel_rnd_lookup_init, .deinit = kernel_deinit, .test = kernel_rnd_lookup },
      .iterations = { DEFAULT_ITERS },
      .name = SP_LIT("rnd_lookup"),
    },
    {
      .kernels = { .init = kernel_delete_init, .deinit = kernel_deinit, .test = kernel_delete },
      .iterations = { DEFAULT_ITERS },
      .name = SP_LIT("delete"),
    },
    {
      .kernels = { .deinit = kernel_deinit, .test = kernel_mixed },
      .iterations = { DEFAULT_ITERS },
      .name = SP_LIT("mixed"),
    },
  };
  run_benchmarks(benches, sp_carr_len(benches));

  return 0;
}
