#include "sp.h"

#define SP_TABLE_IMPLEMENTATION
#include "table.h"

#if defined(__GLIBC__)
  #include <malloc.h>
#endif

#define BENCH_MAX_SLOTS 262144
#define BENCH_MAX_BACKENDS 3
#define BENCH_MAX_PHASES 3

typedef enum {
  DIST_FIXED,
  DIST_UNIFORM,
  DIST_LOG,
} bench_dist_t;

typedef enum {
  WORK_CHURN,
  WORK_RAMP_LIFO,
  WORK_RAMP_FIFO,
  WORK_PIN,
  WORK_REALLOC,
} bench_work_t;

typedef struct {
  const c8* name;
  bench_work_t kind;
  bench_dist_t dist;
  u64 lo;
  u64 hi;
  u32 slots;
  u32 ops;
  u32 survive_pct;
} bench_workload_t;

typedef struct {
  const c8* name;
  void* (*create)();
  void (*destroy)(void* ctx);
  sp_mem_t (*as_mem)(void* ctx);
  bool (*sample)(void* ctx, u64* used, u64* reserved);
} bench_backend_t;

typedef struct {
  void* ptrs [BENCH_MAX_SLOTS];
  u64 sizes [BENCH_MAX_SLOTS];
  u64 live_req;
  u64 rng;
  sp_mem_t mem;
} bench_state_t;

static bench_state_t state = sp_zero;

static u64 bench_rng_next() {
  u64 x = state.rng;
  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  state.rng = x;
  return x * 0x2545F4914F6CDD1DULL;
}

static u64 bench_log2(u64 v) {
  u64 r = 0;
  while (v >>= 1) r++;
  return r;
}

static u64 bench_dist_size(const bench_workload_t* w) {
  switch (w->dist) {
    case DIST_FIXED: return w->lo;
    case DIST_UNIFORM: return w->lo + (bench_rng_next() % (w->hi - w->lo + 1));
    case DIST_LOG: {
      u64 lo_exp = bench_log2(w->lo);
      u64 hi_exp = bench_log2(w->hi);
      u64 e = lo_exp + (bench_rng_next() % (hi_exp - lo_exp + 1));
      u64 size = ((u64)1 << e) + (bench_rng_next() % ((u64)1 << e));
      return sp_min(sp_max(size, w->lo), w->hi);
    }
  }
  return w->lo;
}

static void bench_touch(void* ptr, u64 size) {
  u8* bytes = (u8*)ptr;
  u64 head = size & ~(u64)7;
  for (u64 i = 0; i < head; i += 8) {
    *(u64*)(bytes + i) = 0x5050505050505050ULL;
  }
  for (u64 i = head; i < size; i++) {
    bytes[i] = 0x50;
  }
}

static void bench_alloc_slot(const bench_workload_t* w, u32 slot) {
  u64 size = bench_dist_size(w);
  state.ptrs[slot] = sp_alloc(state.mem, size);
  SP_ASSERT(state.ptrs[slot]);
  bench_touch(state.ptrs[slot], size);
  state.sizes[slot] = size;
  state.live_req += size;
}

static void bench_free_slot(u32 slot) {
  sp_free(state.mem, state.ptrs[slot], state.sizes[slot]);
  state.live_req -= state.sizes[slot];
  state.ptrs[slot] = SP_NULLPTR;
  state.sizes[slot] = 0;
}

static void bench_realloc_slot(u32 slot, u64 size) {
  state.ptrs[slot] = sp_realloc(state.mem, state.ptrs[slot], state.sizes[slot], size);
  SP_ASSERT(state.ptrs[slot]);
  bench_touch(state.ptrs[slot], size);
  state.live_req -= state.sizes[slot];
  state.live_req += size;
  state.sizes[slot] = size;
}

static void* bench_heap_create() {
  return sp_mem_heap_new();
}

static void bench_heap_destroy(void* ctx) {
  sp_mem_heap_destroy((sp_mem_heap_t*)ctx);
}

static sp_mem_t bench_heap_as_mem(void* ctx) {
  return sp_mem_heap_as_allocator((sp_mem_heap_t*)ctx);
}

static bool bench_heap_sample(void* ctx, u64* used, u64* reserved) {
  sp_mem_heap_t* heap = (sp_mem_heap_t*)ctx;
  *used = heap->bytes_used;
  *reserved = heap->bytes_reserved;
  return true;
}

#if !defined(SP_FREESTANDING)
static void* bench_malloc_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr, u64 old_size) {
  sp_unused(user_data);
  sp_unused(old_size);
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: return malloc(size);
    case SP_ALLOCATOR_MODE_RESIZE: return realloc(ptr, size);
    case SP_ALLOCATOR_MODE_FREE: free(ptr); return SP_NULLPTR;
  }
  return SP_NULLPTR;
}

static void* bench_malloc_create() {
  return SP_NULLPTR;
}

static void bench_malloc_destroy(void* ctx) {
  sp_unused(ctx);
}

static sp_mem_t bench_malloc_as_mem(void* ctx) {
  sp_unused(ctx);
  return (sp_mem_t) {
    .on_alloc = bench_malloc_on_alloc
  };
}

static bool bench_malloc_sample(void* ctx, u64* used, u64* reserved) {
  sp_unused(ctx);
  #if defined(__GLIBC__)
    struct mallinfo2 info = mallinfo2();
    *used = info.uordblks;
    *reserved = info.arena + info.hblkhd;
    return true;
  #else
    *used = 0;
    *reserved = 0;
    return false;
  #endif
}
#endif

typedef struct {
  u64 used;
  u64 reserved;
  u64 peak_reserved;
} bench_os_counters_t;

static bench_os_counters_t bench_os_counters = sp_zero;

static u64 bench_os_reservation(u64 size) {
  return sp_align_offset(size, 4096);
}

static void* bench_os_on_alloc(void* user_data, sp_mem_alloc_mode_t mode, u64 size, void* ptr, u64 old_size) {
  bench_os_counters_t* counters = (bench_os_counters_t*)user_data;
  switch (mode) {
    case SP_ALLOCATOR_MODE_ALLOC: {
      void* p = sp_mem_os_alloc(size);
      if (p) {
        counters->used += size;
        counters->reserved += bench_os_reservation(size);
        counters->peak_reserved = sp_max(counters->peak_reserved, counters->reserved);
      }
      return p;
    }
    case SP_ALLOCATOR_MODE_RESIZE: {
      if (!ptr) return bench_os_on_alloc(user_data, SP_ALLOCATOR_MODE_ALLOC, size, SP_NULLPTR, 0);
      void* p = sp_mem_os_realloc(ptr, old_size, size);
      if (p) {
        counters->used -= old_size;
        counters->used += size;
        counters->reserved -= bench_os_reservation(old_size);
        counters->reserved += bench_os_reservation(size);
        counters->peak_reserved = sp_max(counters->peak_reserved, counters->reserved);
      }
      return p;
    }
    case SP_ALLOCATOR_MODE_FREE: {
      if (ptr) {
        counters->used -= old_size;
        counters->reserved -= bench_os_reservation(old_size);
        sp_mem_os_free(ptr, old_size);
      }
      return SP_NULLPTR;
    }
  }
  return SP_NULLPTR;
}

static void* bench_os_create() {
  bench_os_counters = sp_zero_s(bench_os_counters_t);
  return &bench_os_counters;
}

static void bench_os_destroy(void* ctx) {
  sp_unused(ctx);
}

static sp_mem_t bench_os_as_mem(void* ctx) {
  return (sp_mem_t) {
    .on_alloc = bench_os_on_alloc,
    .user_data = ctx
  };
}

static bool bench_os_sample(void* ctx, u64* used, u64* reserved) {
  bench_os_counters_t* counters = (bench_os_counters_t*)ctx;
  *used = counters->used;
  *reserved = counters->reserved;
  return true;
}

static const bench_backend_t backends [] = {
  { "sp_heap", bench_heap_create, bench_heap_destroy, bench_heap_as_mem, bench_heap_sample },
  #if !defined(SP_FREESTANDING)
  { "malloc", bench_malloc_create, bench_malloc_destroy, bench_malloc_as_mem, bench_malloc_sample },
  #endif
  // { "sp_os", bench_os_create, bench_os_destroy, bench_os_as_mem, bench_os_sample },
};

static const bench_workload_t workloads [] = {
  { .name = "fixed_16",      .kind = WORK_CHURN,     .dist = DIST_FIXED,   .lo = 16,                 .slots = 65536,  .ops = 400000 },
  { .name = "fixed_64",      .kind = WORK_CHURN,     .dist = DIST_FIXED,   .lo = 64,                 .slots = 65536,  .ops = 400000 },
  { .name = "fixed_512",     .kind = WORK_CHURN,     .dist = DIST_FIXED,   .lo = 512,                .slots = 16384,  .ops = 100000 },
  { .name = "uniform_small", .kind = WORK_CHURN,     .dist = DIST_UNIFORM, .lo = 1,    .hi = 1024,   .slots = 32768,  .ops = 200000 },
  { .name = "log_mixed",     .kind = WORK_CHURN,     .dist = DIST_LOG,     .lo = 16,   .hi = 16384,  .slots = 8192,   .ops = 100000 },
  { .name = "large",         .kind = WORK_CHURN,     .dist = DIST_UNIFORM, .lo = 4096, .hi = 65536,  .slots = 1024,   .ops = 20000 },
  { .name = "ramp_lifo",     .kind = WORK_RAMP_LIFO, .dist = DIST_UNIFORM, .lo = 16,   .hi = 512,    .slots = 200000 },
  { .name = "ramp_fifo",     .kind = WORK_RAMP_FIFO, .dist = DIST_UNIFORM, .lo = 16,   .hi = 512,    .slots = 200000 },
  { .name = "pin_5pct",      .kind = WORK_PIN,       .dist = DIST_UNIFORM, .lo = 16,   .hi = 256,    .slots = 200000, .survive_pct = 5 },
  { .name = "realloc_grow",  .kind = WORK_REALLOC,   .dist = DIST_FIXED,   .lo = 16,   .hi = 65536,  .slots = 2048,   .ops = 100000 },
};

typedef struct {
  const c8* phase;
  u64 ns_per_op;
  void* ctx;
  const bench_backend_t* backend;
} bench_report_t;

typedef struct {
  const c8* phase;
  const c8* backend;
  u64 ns_per_op;
  u64 req;
  u64 used;
  u64 reserved;
  u64 util;
  bool exact;
} bench_result_t;

typedef struct {
  bench_result_t rows [BENCH_MAX_BACKENDS][BENCH_MAX_PHASES];
  u32 num_phases [BENCH_MAX_BACKENDS];
  u32 num_backends;
} bench_results_t;

static bench_results_t results = sp_zero;

static void bench_report(bench_report_t r) {
  u64 used = 0;
  u64 reserved = 0;
  bool exact = r.backend->sample(r.ctx, &used, &reserved);

  SP_ASSERT(results.num_backends < BENCH_MAX_BACKENDS);
  SP_ASSERT(results.num_phases[results.num_backends] < BENCH_MAX_PHASES);
  bench_result_t* result = &results.rows[results.num_backends][results.num_phases[results.num_backends]++];
  *result = (bench_result_t) {
    .phase = r.phase,
    .backend = r.backend->name,
    .ns_per_op = r.ns_per_op,
    .req = state.live_req,
    .used = used,
    .reserved = reserved,
    .util = reserved ? (state.live_req * 100) / reserved : 0,
    .exact = exact,
  };
}

static void bench_write_ratio(sp_table_writer_t* table, u64 value, u64 best, bool valid) {
  if (!valid) {
    sp_table_write_cstr(table, "?");
    return;
  }
  if (value == best) sp_table_color(table, SP_ANSI_FG_GREEN);
  if (!best) {
    if (value) sp_table_write_cstr(table, "-");
    else       sp_table_write_f64(table, 1.0);
    return;
  }
  sp_table_write_f64(table, (f64)value / (f64)best);
}

static void bench_render_results() {
  if (!results.num_backends) return;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_table_writer_t table = sp_zero;
  sp_table_init(&table, scratch.mem);
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("phase") });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("backend") });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("ns/op"), .align = SP_FMT_ALIGN_RIGHT });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("req"), .fmt = "{.bytes}", .align = SP_FMT_ALIGN_RIGHT });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("used"), .fmt = "{.bytes}", .align = SP_FMT_ALIGN_RIGHT });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("rsvd"), .fmt = "{.bytes}", .align = SP_FMT_ALIGN_RIGHT });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("util"), .fmt = "{}%", .align = SP_FMT_ALIGN_RIGHT });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("ns/best"), .fmt = "{:.2}x", .align = SP_FMT_ALIGN_RIGHT });
  sp_table_add_col(&table, (sp_table_col_t) { .header = sp_str_lit("rsvd/best"), .fmt = "{:.2}x", .align = SP_FMT_ALIGN_RIGHT });

  u32 num_phases = results.num_phases[0];
  sp_for(phase, num_phases) {
    u64 best_ns = 0;
    u64 best_rsvd = 0;
    bool have_rsvd = false;
    sp_for(b, results.num_backends) {
      bench_result_t* result = &results.rows[b][phase];
      if (!b || result->ns_per_op < best_ns) best_ns = result->ns_per_op;
      if (result->exact && (!have_rsvd || result->reserved < best_rsvd)) {
        best_rsvd = result->reserved;
        have_rsvd = true;
      }
    }

    sp_for(b, results.num_backends) {
      SP_ASSERT(results.num_phases[b] == num_phases);
      bench_result_t* result = &results.rows[b][phase];
      sp_table_begin(&table);
      sp_table_write_cstr(&table, result->phase);
      sp_table_write_cstr(&table, result->backend);
      sp_table_write_u64(&table, result->ns_per_op);
      sp_table_write_u64(&table, result->req);
      if (result->exact) {
        sp_table_write_u64(&table, result->used);
        sp_table_write_u64(&table, result->reserved);
        sp_table_write_u64(&table, result->util);
      }
      else {
        sp_table_write_cstr(&table, "?");
        sp_table_write_cstr(&table, "?");
        sp_table_write_cstr(&table, "?");
      }
      bench_write_ratio(&table, result->ns_per_op, best_ns, true);
      bench_write_ratio(&table, result->reserved, best_rsvd, result->exact && have_rsvd);
    }
  }
  sp_table_log(&table);
  sp_mem_end_scratch(scratch);
}

static void bench_run(const bench_workload_t* w, const bench_backend_t* backend) {
  void* ctx = backend->create();
  state.mem = backend->as_mem(ctx);
  state.live_req = 0;
  state.rng = 0x5EED5EED5EED5EEDULL;
  sp_mem_zero(state.ptrs, w->slots * sizeof(void*));
  sp_mem_zero(state.sizes, w->slots * sizeof(u64));


  sp_tm_timer_t timer = sp_tm_start_timer();
  sp_for(it, w->slots) {
    bench_alloc_slot(w, it);
  }
  bench_report((bench_report_t) {
    .phase = "fill",
    .ns_per_op = sp_tm_read_timer(&timer) / w->slots,
    .ctx = ctx,
    .backend = backend,
  });

  switch (w->kind) {
    case WORK_CHURN: {
      sp_tm_reset_timer(&timer);
      sp_for(it, w->ops) {
        u32 slot = (u32)(bench_rng_next() % w->slots);
        if (state.ptrs[slot]) bench_free_slot(slot);
        else bench_alloc_slot(w, slot);
      }
      bench_report((bench_report_t) {
        .phase = "churn",
        .ns_per_op = sp_tm_read_timer(&timer) / w->ops,
        .ctx = ctx,
        .backend = backend,
      });
      break;
    }
    case WORK_RAMP_LIFO: {
      sp_tm_reset_timer(&timer);
      for (u32 it = w->slots; it > 0; it--) {
        bench_free_slot(it - 1);
      }
      bench_report((bench_report_t) {
        .phase = "free",
        .ns_per_op = sp_tm_read_timer(&timer) / w->slots,
        .ctx = ctx,
        .backend = backend,
      });
      break;
    }
    case WORK_RAMP_FIFO: {
      sp_tm_reset_timer(&timer);
      sp_for(it, w->slots) {
        bench_free_slot(it);
      }
      bench_report((bench_report_t) {
        .phase = "free",
        .ns_per_op = sp_tm_read_timer(&timer) / w->slots,
        .ctx = ctx,
        .backend = backend,
      });
      break;
    }
    case WORK_PIN: {
      sp_tm_reset_timer(&timer);
      u32 freed = 0;
      sp_for(it, w->slots) {
        if (bench_rng_next() % 100 >= w->survive_pct) {
          bench_free_slot(it);
          freed++;
        }
      }
      bench_report((bench_report_t) {
        .phase = "pinned",
        .ns_per_op = sp_tm_read_timer(&timer) / sp_max(freed, 1),
        .ctx = ctx,
        .backend = backend,
      });
      break;
    }
    case WORK_REALLOC: {
      sp_tm_reset_timer(&timer);
      sp_for(it, w->ops) {
        u32 slot = (u32)(bench_rng_next() % w->slots);
        if (!state.ptrs[slot]) {
          bench_alloc_slot(w, slot);
        }
        else if (state.sizes[slot] * 2 > w->hi) {
          bench_free_slot(slot);
        }
        else {
          bench_realloc_slot(slot, state.sizes[slot] * 2);
        }
      }
      bench_report((bench_report_t) {
        .phase = "grow",
        .ns_per_op = sp_tm_read_timer(&timer) / w->ops,
        .ctx = ctx,
        .backend = backend,
      });
      break;
    }
  }

  sp_tm_reset_timer(&timer);
  u32 drained = 0;
  sp_for(it, w->slots) {
    if (state.ptrs[it]) {
      bench_free_slot(it);
      drained++;
    }
  }
  if (drained) {
    bench_report((bench_report_t) {
      .phase = "drained",
      .ns_per_op = sp_tm_read_timer(&timer) / drained,
      .ctx = ctx,
      .backend = backend,
    });
  }

  backend->destroy(ctx);
  results.num_backends++;
}

s32 main(s32 argc, const c8** argv) {
  sp_str_t workload_filter = argc > 1 ? sp_str_view(argv[1]) : sp_str_lit("");
  sp_str_t backend_filter = argc > 2 ? sp_str_view(argv[2]) : sp_str_lit("");

  sp_mem_zero(state.ptrs, sizeof(state.ptrs));
  sp_mem_zero(state.sizes, sizeof(state.sizes));

  sp_carr_for(workloads, w) {
    const bench_workload_t* workload = &workloads[w];
    if (!sp_str_empty(workload_filter)) {
      if (!sp_str_equal_cstr(workload_filter, workload->name)) {
        continue;
      }
    }

    sp_log("> {.yellow}", sp_fmt_cstr(workload->name));
    sp_log(
      "min={.cyan} max={.cyan} slots={.cyan} ops={.cyan}",
      sp_fmt_uint(workload->lo),
      sp_fmt_uint(sp_max(workload->lo, workload->hi)),
      sp_fmt_uint(workload->slots),
      sp_fmt_uint(workload->ops)
    );

    results = sp_zero_s(bench_results_t);
    sp_carr_for(backends, b) {
      const bench_backend_t* backend = &backends[b];
      if (!sp_str_empty(backend_filter)) {
        if (!sp_str_equal(sp_str_view(backend->name), backend_filter)) {
          continue;
        }
      }
      bench_run(workload, backend);
    }
    bench_render_results();
    sp_log("");
  }

  return 0;
}
