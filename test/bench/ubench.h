
#ifndef SP_BENCH_H
#define SP_BENCH_H

#if defined(UBENCH_ENABLE_SQLITE) && defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#ifndef SP_PRIVATE_HEADER
#define SP_PRIVATE_HEADER
#endif

#include "sp.h"
SP_BEGIN_EXTERN_C()

typedef u64 ubench_size_t;

////////////
// MACROS //
////////////
#if defined(SP_CPP)
  #define UBENCH_C_FUNC extern "C"
#else
  #define UBENCH_C_FUNC
#endif

#if defined(SP_MSVC)
  #define UBENCH_UNUSED
#else
  #define UBENCH_UNUSED SP_ATTRIBUTE(unused)
#endif

#if defined(SP_CPP)
  #define UBENCH_EXTERN extern "C"
#else
  #define UBENCH_EXTERN extern
#endif

#if defined(SP_MSVC)
  #define UBENCH_DO_NOT_OPTIMIZE(x)                                              \
    do {                                                                         \
      _ReadWriteBarrier();                                                       \
      ubench_do_nothing((void *)&(x));                                           \
    } while (0)
  #define UBENCH_CLOBBER_MEMORY() _ReadWriteBarrier()

#else
  #define UBENCH_CLOBBER_MEMORY() __asm__ volatile("" : : : "memory")

  #if defined(SP_CLANG)
    #define UBENCH_DO_NOT_OPTIMIZE(x)                                              \
      __asm__ volatile("" : "+r,m"(x) : : "memory")
  #else
    #define UBENCH_DO_NOT_OPTIMIZE(x)                                              \
      __asm__ volatile("" : "+m,r"(x) : : "memory")
  #endif
#endif


typedef struct ubench_run_state_s {
  s64 *ns;
  s64 *pause_ns;
  s64 size;
  s64 sample;
  s64 paused_ns;
  s64 pause_start;
  s64 bytes_processed;
  s64 items_processed;
  /* Auto-tuned per-sample batch size: each clock-bracketed sample executes
     `batch` body invocations when the body uses UBENCH_LOOP. The runner
     amortizes clock-call overhead so micro-bodies (sub-µs) become measurable.
     batch_consumed is set non-zero by UBENCH_LOOP to signal that the body
     opted into batching, so the runner knows whether to tune. */
  s64 batch;
  s64 batch_consumed;
} ubench_run_state_t;

struct ubench_benchmark_state_s;

typedef void (*ubench_body_t)(void *fixture, struct ubench_run_state_s *ubs);
typedef void (*ubench_setup_t)(void *fixture);
typedef void (*ubench_teardown_t)(void *fixture);

typedef struct ubench_fixture_ops_s {
  ubench_setup_t setup;
  ubench_teardown_t teardown;
  size_t size;
} ubench_fixture_ops_t;

typedef void (*ubench_dispatch_t)(struct ubench_benchmark_state_s *b,
                                  struct ubench_run_state_s *ubs);

typedef struct ubench_benchmark_state_s {
  sp_str_t name;
  ubench_body_t body;
  const struct ubench_fixture_ops_s *ops;
  ubench_dispatch_t dispatch;
} ubench_benchmark_state_t;

typedef struct ubench_state_s {
  ubench_benchmark_state_t* benchmarks; // @spader make this sp_da, get rid of len
  ubench_size_t benchmarks_length;
  f64 confidence;
  sp_mem_t mem;
} ubench_state_t;

typedef struct unbench_benchmark_config_s {
  const c8* name;
  ubench_body_t body;
  const ubench_fixture_ops_t* ops;
  ubench_dispatch_t dispatch;
} ubench_benchmark_config_t;

SP_API struct ubench_state_s ubench_state;
SP_API void ubench_run_lifecycle(ubench_benchmark_state_t* b, ubench_run_state_t* run);
SP_API void ubench_invoke(ubench_benchmark_state_t* b, ubench_run_state_t* run);
SP_API s32 ubench_do_benchmark(ubench_run_state_t* const run);
SP_API void ubench_register_benchmark(sp_str_t name, ubench_body_t body, const ubench_fixture_ops_t* ops, ubench_dispatch_t dispatch);
SP_API void ubench_register_benchmark_s(ubench_benchmark_config_t config);
SP_API SP_INLINE void ubench_pause(ubench_run_state_t* const run);
SP_API SP_INLINE void ubench_resume(ubench_run_state_t* const run);
SP_API SP_INLINE void ubench_do_nothing(void* ptr);
SP_API sp_str_t sp_cpu_get_model_a(sp_mem_t mem);
SP_API u32      sp_cpu_get_thread_count(void);

#define UBENCH_STATE()                                                         \
  struct ubench_state_s ubench_state = {                                       \
      .benchmarks = SP_NULLPTR,                                                \
      .benchmarks_length = 0,                                                  \
      .confidence = 2.5,                                                       \
      .mem = {sp_mem_os_on_alloc, SP_NULLPTR}}

#define UBENCH_MAIN()                                                          \
  UBENCH_STATE();                                                              \
  s32 main(s32 argc, const c8* const argv[]) {                                 \
    return ubench_main(argc, argv);                                            \
  }


typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef long long int sqlite3_int64;
typedef void (*sqlite3_destructor_type)(void*);

#define SQLITE_OK 0
#define SQLITE_ROW  100
#define SQLITE_DONE 101
#define SQLITE_STATIC      ((sqlite3_destructor_type)0)
#define SQLITE_TRANSIENT   ((sqlite3_destructor_type)-1)

int sqlite3_open(const char *filename, sqlite3 **ppDb);
int sqlite3_close(sqlite3*);
int sqlite3_exec(sqlite3*, const char *sql, int (*callback)(void*,int,char**,char**), void*, char **errmsg);
const char *sqlite3_errmsg(sqlite3*);
int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
int sqlite3_step(sqlite3_stmt*);
int sqlite3_reset(sqlite3_stmt *pStmt);
int sqlite3_finalize(sqlite3_stmt *pStmt);
sqlite3_int64 sqlite3_last_insert_rowid(sqlite3*);
int sqlite3_bind_null(sqlite3_stmt*, int);
int sqlite3_bind_int(sqlite3_stmt*, int, int);
int sqlite3_bind_int64(sqlite3_stmt*, int, sqlite3_int64);
int sqlite3_bind_double(sqlite3_stmt*, int, double);
int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int, void(*)(void*));
sqlite3_int64 sqlite3_column_int64(sqlite3_stmt*, int iCol);


SP_IMP SP_INLINE s32 ubench_should_filter(const c8 *filter, const c8 *benchmark);
SP_IMP SP_INLINE s32 ubench_int64_cmp(const void* a, const void* b);
SP_IMP SP_INLINE f32 sp_sys_sqrtf(f32 x);
SP_IMP void ubench_fmt_tty_green(sp_io_writer_t *io, sp_fmt_arg_t *arg, sp_fmt_arg_t *params);
SP_IMP void ubench_fmt_tty_red(sp_io_writer_t *io, sp_fmt_arg_t *arg, sp_fmt_arg_t *params);
SP_IMP void ubench_fmt_tty_reset(sp_io_writer_t *io, sp_fmt_arg_t *arg, sp_fmt_arg_t *params);

#define BENCH_STORE_SCHEMA                                                     \
  "CREATE TABLE IF NOT EXISTS machines ("                                      \
  " id INTEGER PRIMARY KEY,"                                                   \
  " fingerprint TEXT NOT NULL UNIQUE,"                                         \
  " hostname TEXT,"                                                            \
  " os_name TEXT, os_version TEXT, arch TEXT,"                                 \
  " cpu_model TEXT, cpu_cores INTEGER, cpu_threads INTEGER,"                   \
  " memory_bytes INTEGER);"                                                    \
  "CREATE TABLE IF NOT EXISTS runs ("                                          \
  " id INTEGER PRIMARY KEY,"                                                   \
  " machine_id INTEGER NOT NULL REFERENCES machines(id),"                      \
  " started_at TEXT NOT NULL,"                                                 \
  " finished_at TEXT,"                                                         \
  " executable_path TEXT,"                                                     \
  " executable_size_bytes INTEGER,"                                            \
  " executable_mtime TEXT,"                                                    \
  " confidence_threshold REAL,"                                                \
  " filter TEXT,"                                                              \
  " has_perf_counters INTEGER,"                                                \
  " label TEXT,"                                                               \
  " framework TEXT,"                                                           \
  " metadata TEXT);"                                                           \
  "CREATE TABLE IF NOT EXISTS benchmarks ("                                    \
  " id INTEGER PRIMARY KEY,"                                                   \
  " name TEXT NOT NULL UNIQUE);"                                               \
  "CREATE TABLE IF NOT EXISTS results ("                                       \
  " id INTEGER PRIMARY KEY,"                                                   \
  " run_id INTEGER NOT NULL REFERENCES runs(id),"                              \
  " benchmark_id INTEGER NOT NULL REFERENCES benchmarks(id),"                  \
  " iterations INTEGER NOT NULL,"                                              \
  " mean_ns REAL NOT NULL,"                                                    \
  " median_ns REAL NOT NULL,"                                                  \
  " min_ns REAL NOT NULL,"                                                     \
  " max_ns REAL NOT NULL,"                                                     \
  " stddev_ns REAL,"                                                           \
  " stddev_pct REAL,"                                                          \
  " ci_low_ns REAL,"                                                           \
  " ci_high_ns REAL,"                                                          \
  " ci_level_pct REAL,"                                                        \
  " confidence_pct REAL,"                                                      \
  " bytes_processed INTEGER,"                                                  \
  " items_processed INTEGER,"                                                  \
  " cycles_per_iter INTEGER,"                                                  \
  " instructions_per_iter INTEGER,"                                            \
  " UNIQUE(run_id, benchmark_id));"                                            \
  "CREATE INDEX IF NOT EXISTS idx_results_bench_run"                           \
  " ON results(benchmark_id, run_id);"

#define BENCH_UNSET_I64 ((s64)-1)
#define BENCH_UNSET_F64 (-1.0)

typedef struct bench_store bench_store;

typedef struct {
  c8  hostname[256];
  c8  os_name[64];
  c8  os_version[128];
  c8  arch[64];
  c8  cpu_model[256];
  s32 cpu_cores;
  s32 cpu_threads;
  s64 memory_bytes;
} bench_machine_info;

typedef struct {
  const c8 *executable_path;
  s64       executable_size_bytes;
  const c8 *executable_mtime;       /* ISO 8601 UTC. */
  const c8 *filter;
  const c8 *label;
  const c8 *framework;
  const c8 *metadata_json;
  f64       confidence_threshold;   /* < 0 => NULL. */
  s32       has_perf_counters;      /* < 0 => NULL. */
} bench_run_info;

typedef struct {
  s64 iterations;
  f64 mean_ns;
  f64 median_ns;
  f64 min_ns;
  f64 max_ns;
  f64 stddev_ns;
  f64 stddev_pct;
  f64 ci_low_ns;
  f64 ci_high_ns;
  f64 ci_level_pct;
  f64 confidence_pct;
  s64 bytes_processed;
  s64 items_processed;
  s64 cycles_per_iter;
  s64 instructions_per_iter;
} bench_result;

SP_API bench_store* bench_store_open(const c8 *path);
SP_API void         bench_store_close(bench_store* s);
SP_API s32          bench_collect_machine_info(bench_machine_info* out);
SP_API s64          bench_store_begin_run(bench_store* s, const bench_machine_info* mi, const bench_run_info* ri);
SP_API s32          bench_store_record(bench_store* s, s64 run_id, const c8 *bench_name, const bench_result* r);
SP_API s32          bench_store_end_run(bench_store* s, s64 run_id);
SP_API s64 bench_simple_begin_run(
  bench_store* s,
  const c8* framework, const c8* label,
  const c8* executable_path, s64 executable_size_bytes, const c8* executable_mtime,
  f64 confidence_threshold,
  s32 has_perf_counters
);
SP_API s32 bench_simple_record(
  bench_store* s, s64 run_id,
  const c8* name,
  s64 iterations,
  f64 mean_ns, f64 median_ns,
  f64 min_ns, f64 max_ns,
  f64 stddev_ns, f64 stddev_pct,
  f64 ci_low_ns, f64 ci_high_ns, f64 ci_level_pct,
  f64 confidence_pct,
  s64 bytes_processed, s64 items_processed,
  s64 cycles_per_iter, s64 instructions_per_iter
);

SP_IMP s32 bench__get_or_insert_machine(sqlite3* db, const bench_machine_info* m, sqlite3_int64* out_id);
SP_IMP void bench__read_first_field(const c8* path, const c8* prefix, c8* dst, u32 dst_size);
SP_IMP void bench__make_fingerprint(const bench_machine_info* m, c8* dst, u32 dst_size);
SP_IMP s32 bench__get_or_insert_benchmark(sqlite3* db, const c8* name, sqlite3_int64* out_id);
SP_IMP void bench__bind_text_or_null(sqlite3_stmt *stmt, int idx, const c8 *s);
SP_IMP void bench__bind_i64_or_null(sqlite3_stmt *stmt, int idx, s64 v);
SP_IMP void bench__bind_f64_or_null(sqlite3_stmt *stmt, int idx, f64 v);

///////////////////////
// UBENCH_DO_NOTHING //
///////////////////////
#if defined(SP_MSVC)
  UBENCH_C_FUNC void _ReadWriteBarrier(void);

  void ubench_do_nothing(void *ptr) {
    (void)ptr;
    _ReadWriteBarrier();
  }
#elif defined(SP_CLANG)
  void ubench_do_nothing(void *ptr) {
    _Pragma("clang diagnostic push")
        _Pragma("clang diagnostic ignored \"-Wlanguage-extension-token\"");
    __asm__ volatile("" : : "r"(ptr), "m"(ptr) : "memory");
    _Pragma("clang diagnostic pop");
  }
#else
  void ubench_do_nothing(void *ptr) {
    __asm__ volatile("" : : "r"(ptr), "m"(ptr) : "memory");
  }
#endif


////////////////////////
// UBENCH_INITIALIZER //
////////////////////////
#if defined(SP_CPP)
  #if defined(SP_CLANG)
    #define UBENCH_INITIALIZER_BEGIN_DISABLE_WARNINGS                              \
      _Pragma("clang diagnostic push")                                             \
          _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"")

    #define UBENCH_INITIALIZER_END_DISABLE_WARNINGS _Pragma("clang diagnostic pop")
  #else
    #define UBENCH_INITIALIZER_BEGIN_DISABLE_WARNINGS
    #define UBENCH_INITIALIZER_END_DISABLE_WARNINGS
  #endif

  #define UBENCH_INITIALIZER(f)                                                  \
    struct f##_cpp_struct {                                                      \
      f##_cpp_struct();                                                          \
    };                                                                           \
    UBENCH_INITIALIZER_BEGIN_DISABLE_WARNINGS static f##_cpp_struct              \
        f##_cpp_global UBENCH_INITIALIZER_END_DISABLE_WARNINGS;                  \
    f##_cpp_struct::f##_cpp_struct()

#elif defined(SP_MSVC)
  #define UBENCH_SYMBOL_PREFIX

  #if defined(SP_CLANG)
    #define UBENCH_INITIALIZER_BEGIN_DISABLE_WARNINGS                              \
      _Pragma("clang diagnostic push")                                             \
          _Pragma("clang diagnostic ignored \"-Wmissing-variable-declarations\"")

    #define UBENCH_INITIALIZER_END_DISABLE_WARNINGS _Pragma("clang diagnostic pop")
  #else
    #define UBENCH_INITIALIZER_BEGIN_DISABLE_WARNINGS
    #define UBENCH_INITIALIZER_END_DISABLE_WARNINGS
  #endif

  #pragma section(".CRT$XCU", read)
  #define UBENCH_INITIALIZER(f)                                                  \
    static void __cdecl f(void);                                                 \
    UBENCH_INITIALIZER_BEGIN_DISABLE_WARNINGS __pragma(                          \
        comment(linker, "/include:" UBENCH_SYMBOL_PREFIX #f "_")) UBENCH_C_FUNC  \
        __declspec(allocate(".CRT$XCU")) void(__cdecl * f##_)(void) = f;         \
    UBENCH_INITIALIZER_END_DISABLE_WARNINGS static void __cdecl f(void)
#else
  #define UBENCH_INITIALIZER(f)                                                  \
    static void f(void) SP_ATTRIBUTE(constructor);                           \
    static void f(void)
#endif

//////////////////////////////
// UBENCH_SURPRESS_WARNINGS //
//////////////////////////////
#if defined(SP_CLANG)
#if __has_warning("-Wunsafe-buffer-usage")
#define UBENCH_SURPRESS_WARNINGS_BEGIN                                         \
  _Pragma("clang diagnostic push")                                             \
      _Pragma("clang diagnostic ignored \"-Wunsafe-buffer-usage\"")
#define UBENCH_SURPRESS_WARNINGS_END _Pragma("clang diagnostic pop")
#else
#define UBENCH_SURPRESS_WARNINGS_BEGIN
#define UBENCH_SURPRESS_WARNINGS_END
#endif
#elif defined(SP_GNUC) && __GNUC__ >= 8 && defined(SP_CPP)
#define UBENCH_SURPRESS_WARNINGS_BEGIN                                         \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wclass-memaccess\"")
#define UBENCH_SURPRESS_WARNINGS_END _Pragma("GCC diagnostic pop")
#else
#define UBENCH_SURPRESS_WARNINGS_BEGIN
#define UBENCH_SURPRESS_WARNINGS_END
#endif

#define UBENCH_DO_BENCHMARK() \
  while (ubench_do_benchmark(ubench_run_state) > 0)

#define UBENCH_DO_NOTHING(x) \
  ubench_do_nothing(x)

#define UBENCH_EX(SET, NAME)                                                   \
  UBENCH_SURPRESS_WARNINGS_BEGIN                                               \
  static void ubench_##SET##_##NAME(void *,                                    \
                                    struct ubench_run_state_s *);              \
  UBENCH_INITIALIZER(ubench_register_##SET##_##NAME) {                         \
    ubench_register_benchmark(                         \
      sp_str_lit(#SET "." #NAME),                      \
      &ubench_##SET##_##NAME,                          \
      SP_NULLPTR, SP_NULLPTR                           \
    );                                                 \
  }                                                                            \
  UBENCH_SURPRESS_WARNINGS_END                                                 \
  void ubench_##SET##_##NAME(void *ubench_fixture_unused UBENCH_UNUSED,        \
                             struct ubench_run_state_s *ubench_run_state)

/* The user body receives `ubench_run_state` as a parameter so that
   UBENCH_LOOP, UBENCH_PAUSE, UBENCH_RESUME, UBENCH_SET_BYTES_PROCESSED, etc.
   can resolve the symbol from inside a UBENCH(...) body. The parameter has
   a fixed name and is unused by callers that don't need it, so this is a
   silent extension to the macro contract. */
#define UBENCH(SET, NAME)                                                      \
  static void ubench_run_##SET##_##NAME(struct ubench_run_state_s *);          \
  UBENCH_EX(SET, NAME) {                                                       \
    UBENCH_DO_BENCHMARK() { ubench_run_##SET##_##NAME(ubench_run_state); }     \
  }                                                                            \
  void ubench_run_##SET##_##NAME(                                              \
      struct ubench_run_state_s *ubench_run_state UBENCH_UNUSED)

#define UBENCH_F_SETUP(FIXTURE)                                                \
  static void ubench_f_setup_impl_##FIXTURE(struct FIXTURE *ubench_fixture);   \
  static void ubench_f_setup_##FIXTURE(void *ubench_fixture_void) {            \
    ubench_f_setup_impl_##FIXTURE((struct FIXTURE *)ubench_fixture_void);      \
  }                                                                            \
  static void ubench_f_setup_impl_##FIXTURE(struct FIXTURE *ubench_fixture)

#define UBENCH_F_TEARDOWN(FIXTURE)                                             \
  static void ubench_f_teardown_impl_##FIXTURE(struct FIXTURE *ubench_fixture);\
  static void ubench_f_teardown_##FIXTURE(void *ubench_fixture_void) {         \
    ubench_f_teardown_impl_##FIXTURE((struct FIXTURE *)ubench_fixture_void);   \
  }                                                                            \
  static void ubench_f_teardown_impl_##FIXTURE(struct FIXTURE *ubench_fixture)

#define UBENCH_EX_F(FIXTURE, NAME)                                             \
  UBENCH_SURPRESS_WARNINGS_BEGIN                                               \
  static void ubench_f_setup_##FIXTURE(void *);                                \
  static void ubench_f_teardown_##FIXTURE(void *);                             \
  static void ubench_run_ex_##FIXTURE##_##NAME(struct FIXTURE *,               \
                                               struct ubench_run_state_s *);   \
  static void ubench_f_##FIXTURE##_##NAME(                                     \
      void *ubench_fixture_void,                                               \
      struct ubench_run_state_s *ubench_run_state) {                           \
    ubench_run_ex_##FIXTURE##_##NAME((struct FIXTURE *)ubench_fixture_void,    \
                                     ubench_run_state);                        \
  }                                                                            \
  UBENCH_INITIALIZER(ubench_register_##FIXTURE##_##NAME) {                     \
    static const struct ubench_fixture_ops_s ubench_ops_##FIXTURE##_##NAME = { \
        .setup = &ubench_f_setup_##FIXTURE,                                    \
        .teardown = &ubench_f_teardown_##FIXTURE,                              \
        .size = sizeof(struct FIXTURE)};                                       \
    ubench_register_benchmark(sp_str_lit(#FIXTURE "." #NAME),                  \
                              &ubench_f_##FIXTURE##_##NAME,                    \
                              &ubench_ops_##FIXTURE##_##NAME,                  \
                              SP_NULLPTR);                                     \
  }                                                                            \
  UBENCH_SURPRESS_WARNINGS_END                                                 \
  void ubench_run_ex_##FIXTURE##_##NAME(                                       \
      struct FIXTURE *ubench_fixture,                                          \
      struct ubench_run_state_s *ubench_run_state)

#define UBENCH_F(FIXTURE, NAME)                                                \
  static void ubench_run_##FIXTURE##_##NAME(struct FIXTURE *,                  \
                                            struct ubench_run_state_s *);     \
  UBENCH_EX_F(FIXTURE, NAME) {                                                 \
    UBENCH_DO_BENCHMARK() {                                                    \
      ubench_run_##FIXTURE##_##NAME(ubench_fixture, ubench_run_state);         \
    }                                                                          \
  }                                                                            \
  void ubench_run_##FIXTURE##_##NAME(                                          \
      struct FIXTURE *ubench_fixture,                                          \
      struct ubench_run_state_s *ubench_run_state UBENCH_UNUSED)

// Prevent 64-bit integer overflow when computing a timestamp by using a trick
// from Sokol:
// https://github.com/floooh/sokol/blob/189843bf4f86969ca4cc4b6d94e793a37c5128a7/sokol_time.h#L204
SP_IMP SP_INLINE s64 ubench_mul_div(const s64 value, const s64 numer, const s64 denom) {
  const s64 q = value / denom;
  const s64 r = value % denom;
  return q * numer + r * numer / denom;
}

static SP_INLINE s64 ubench_ns(void) {
#if defined(SP_WIN32)
  /* QPC frequency is constant for the lifetime of the process; query once. */
  static s64 qpc_freq = 0;
  LARGE_INTEGER counter;
  if (qpc_freq == 0) {
    LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    qpc_freq = f.QuadPart;
  }
  QueryPerformanceCounter(&counter);
  return ubench_mul_div(counter.QuadPart, 1000000000, qpc_freq);
#elif defined(SP_LINUX)
  /* Use a monotonic clock so NTP slew/step cannot corrupt deltas. Prefer
     CLOCK_MONOTONIC_RAW where available (Linux >= 2.6.28) since it is also
     immune to adjtimex frequency steering. */
  struct timespec ts;
#if defined(CLOCK_MONOTONIC_RAW)
  const clockid_t cid = CLOCK_MONOTONIC_RAW;
#else
  const clockid_t cid = CLOCK_MONOTONIC;
#endif
  clock_gettime(cid, &ts);
  return sp_cast(s64, ts.tv_sec) * 1000 * 1000 * 1000 +
         ts.tv_nsec;
#elif defined(SP_MACOS)
  return sp_cast(s64, clock_gettime_nsec_np(CLOCK_UPTIME_RAW));
#else
#error Unsupported platform!
#endif
}

void ubench_run_lifecycle(ubench_benchmark_state_t* b, ubench_run_state_t* run) {
  if (b->ops != SP_NULLPTR) {
    sp_mem_arena_marker_t scratch = sp_mem_begin_scratch_for(ubench_state.mem);
    void *fixture = sp_alloc(scratch.mem, b->ops->size);
    b->ops->setup(fixture);
    b->body(fixture, run);
    b->ops->teardown(fixture);
    sp_mem_end_scratch(scratch);
  } else {
    b->body(SP_NULLPTR, run);
  }
}

void ubench_invoke(ubench_benchmark_state_t* b, ubench_run_state_t* run) {
  if (b->dispatch != SP_NULLPTR) {
    b->dispatch(b, run);
  } else {
    ubench_run_lifecycle(b, run);
  }
}

// @spader C string in the public API
void ubench_register_benchmark(sp_str_t name, ubench_body_t body, const ubench_fixture_ops_t* ops, ubench_dispatch_t dispatch) {
  const ubench_size_t i = ubench_state.benchmarks_length++;
  ubench_state.benchmarks = sp_ptr_cast(
    ubench_benchmark_state_t*,
    sp_realloc(ubench_state.mem, ubench_state.benchmarks, sizeof(ubench_benchmark_state_t) * ubench_state.benchmarks_length)
  );

  ubench_state.benchmarks[i].name = name;
  ubench_state.benchmarks[i].body = body;
  ubench_state.benchmarks[i].ops = ops;
  ubench_state.benchmarks[i].dispatch = dispatch;
}

void ubench_register_benchmark_s(ubench_benchmark_config_t config) {
  ubench_register_benchmark(sp_cstr_as_str(config.name), config.body, config.ops, config.dispatch);
}


s32 ubench_do_benchmark(ubench_run_state_t* const run) {
  const s64 curr_sample = run->sample++;
  if (curr_sample > 0) {
    run->pause_ns[curr_sample - 1] = run->paused_ns;
  }
  run->paused_ns = 0;
  run->ns[curr_sample] = ubench_ns();
  return curr_sample < run->size ? 1 : 0;
}

void ubench_pause(ubench_run_state_t* const run) {
  run->pause_start = ubench_ns();
}

void ubench_resume(ubench_run_state_t* const run) {
  run->paused_ns += ubench_ns() - run->pause_start;
}

#define UBENCH_PAUSE() \
  ubench_pause(ubench_run_state)

#define UBENCH_RESUME() \
  ubench_resume(ubench_run_state)

#define UBENCH_SET_BYTES_PROCESSED(N)                                          \
  (ubench_run_state->bytes_processed = (s64)(N))

#define UBENCH_SET_ITEMS_PROCESSED(N)                                          \
  (ubench_run_state->items_processed = (s64)(N))

#define UBENCH_LOOP                                                            \
  for (                                                                \
    s64 ubench_loop_i_ = ((ubench_run_state->batch_consumed = 1),      \
    ubench_run_state->batch);                                          \
    ubench_loop_i_ > 0; ubench_loop_i_--                               \
  )

s32 ubench_should_filter(const c8 *filter, const c8 *benchmark) {
  if (filter) {
    const c8 *filter_cur = filter;
    const c8 *benchmark_cur = benchmark;
    const c8 *filter_wildcard = SP_NULLPTR;

    while (('\0' != *filter_cur) && ('\0' != *benchmark_cur)) {
      if ('*' == *filter_cur) {
        /* store the position of the wildcard */
        filter_wildcard = filter_cur;

        /* skip the wildcard character */
        filter_cur++;

        while (('\0' != *filter_cur) && ('\0' != *benchmark_cur)) {
          if ('*' == *filter_cur) {
            /*
               we found another wildcard (filter is something like *foo*) so we
               exit the current loop, and return to the parent loop to handle
               the wildcard case
            */
            break;
          } else if (*filter_cur != *benchmark_cur) {
            /* otherwise our filter didn't match, so reset it */
            filter_cur = filter_wildcard;
          }

          /* move benchmark along */
          benchmark_cur++;

          /* move filter along */
          filter_cur++;
        }

        if (('\0' == *filter_cur) && ('\0' == *benchmark_cur)) {
          return 0;
        }

        /* if the benchmarks have been exhausted, we don't have a match! */
        if ('\0' == *benchmark_cur) {
          return 1;
        }
      } else {
        if (*benchmark_cur != *filter_cur) {
          /* benchmark doesn't match filter */
          return 1;
        } else {
          /* move our filter and benchmark forward */
          benchmark_cur++;
          filter_cur++;
        }
      }
    }

    if (('\0' != *filter_cur) ||
        (('\0' != *benchmark_cur) &&
         ((filter == filter_cur) || ('*' != filter_cur[-1])))) {
      /* we have a mismatch! */
      return 1;
    }
  }

  return 0;
}

s32 ubench_int64_cmp(const void *a, const void *b) {
  const s64 aa = *sp_ptr_cast(const s64 *, a);
  const s64 bb = *sp_ptr_cast(const s64 *, b);
  return aa < bb ? -1 : (aa > bb ? 1 : 0);
}

f32 sp_sys_sqrtf(f32 x) {
  if (x < 0) return 0;
  if (x == 0) return 0;
  f32 guess = x / 2.0f;
  for (s32 i = 0; i < 10; i++) {
    guess = (guess + x / guess) / 2.0f;
  }
  return guess;
}

void ubench_fmt_tty_green(sp_io_writer_t *io, sp_fmt_arg_t *arg, sp_fmt_arg_t *params) {
  sp_unused(arg); sp_unused(params);
  if (sp_os_is_tty(sp_sys_stdout)) {
    sp_io_write_cstr(io, SP_ANSI_FG_GREEN, SP_NULLPTR);
  }
}

void ubench_fmt_tty_red(sp_io_writer_t *io, sp_fmt_arg_t *arg, sp_fmt_arg_t *params) {
  sp_unused(arg); sp_unused(params);
  if (sp_os_is_tty(sp_sys_stdout)) {
    sp_io_write_cstr(io, SP_ANSI_FG_RED, SP_NULLPTR);
  }
}

void ubench_fmt_tty_reset(sp_io_writer_t *io, sp_fmt_arg_t *arg, sp_fmt_arg_t *params) {
  sp_unused(arg); sp_unused(params);
  if (sp_os_is_tty(sp_sys_stdout)) {
    sp_io_write_cstr(io, SP_ANSI_RESET, SP_NULLPTR);
  }
}

#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
#include <linux/perf_event.h>

struct ubench_perf_s {
  s32 group_fd;
  s32 instr_fd;
  /* Per-pair overhead introduced by the ioctl(RESET)+ioctl(ENABLE) ...
     ioctl(DISABLE)+read() sequence itself, measured at startup with an empty
     body. Subtracted from every measurement so reported counts approximate
     just the user code. */
  u64 overhead_cycles;
  u64 overhead_instructions;
};

static s32 ubench_perf_open_event(s32 leader, u32 config) {
  struct perf_event_attr pea;
  sp_mem_zero(&pea, sizeof(pea));
  pea.type = PERF_TYPE_HARDWARE;
  pea.size = sizeof(pea);
  pea.config = config;
  pea.disabled = (leader == -1) ? 1 : 0;
  pea.exclude_kernel = 1;
  pea.exclude_hv = 1;
  pea.read_format = PERF_FORMAT_GROUP;
  return sp_cast(s32,
                     sp_syscall(SP_SYSCALL_NUM_PERF_EVENT_OPEN, &pea, 0, -1, leader, 0));
}

static void ubench_perf_start(struct ubench_perf_s *p) {
  if (p->group_fd < 0) {
    return;
  }
  sp_syscall(SP_SYSCALL_NUM_IOCTL, p->group_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  sp_syscall(SP_SYSCALL_NUM_IOCTL, p->group_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

static void ubench_perf_stop_raw(struct ubench_perf_s *p,
                                 u64 *cycles,
                                 u64 *instructions) {
  struct {
    u64 nr;
    u64 values[2];
  } buf;
  *cycles = 0;
  *instructions = 0;
  if (p->group_fd < 0) {
    return;
  }
  sp_syscall(SP_SYSCALL_NUM_IOCTL, p->group_fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
  if (sp_syscall(SP_SYSCALL_NUM_READ, p->group_fd, &buf, sizeof(buf)) ==
          sp_cast(s64, sizeof(buf)) &&
      buf.nr == 2) {
    *cycles = buf.values[0];
    *instructions = buf.values[1];
  }
}

static void ubench_perf_stop(struct ubench_perf_s *p, u64 *cycles,
                             u64 *instructions) {
  ubench_perf_stop_raw(p, cycles, instructions);
  /* Subtract per-pair overhead measured at init. Saturate to zero rather than
     wrap around if a single ultra-cheap measurement happens to undercount. */
  *cycles = (*cycles > p->overhead_cycles) ? *cycles - p->overhead_cycles : 0;
  *instructions = (*instructions > p->overhead_instructions)
                      ? *instructions - p->overhead_instructions
                      : 0;
}

static void ubench_perf_init(struct ubench_perf_s *p) {
  p->instr_fd = -1;
  p->overhead_cycles = 0;
  p->overhead_instructions = 0;
  p->group_fd = ubench_perf_open_event(-1, PERF_COUNT_HW_CPU_CYCLES);
  if (p->group_fd < 0) {
    return;
  }
  p->instr_fd =
      ubench_perf_open_event(p->group_fd, PERF_COUNT_HW_INSTRUCTIONS);
  if (p->instr_fd < 0) {
    sp_syscall(SP_SYSCALL_NUM_CLOSE, p->group_fd);
    p->group_fd = -1;
    return;
  }

  /* Calibrate per-pair start/stop overhead: take the minimum of N empty
     start/stop pairs, mirroring nanobench's mCalibratedOverhead. */
  {
    u64 best_cycles = (u64)-1;
    u64 best_instructions = (u64)-1;
    s32 trial;
    for (trial = 0; trial < 32; trial++) {
      u64 c = 0, i = 0;
      ubench_perf_start(p);
      ubench_perf_stop_raw(p, &c, &i);
      if (c < best_cycles) {
        best_cycles = c;
      }
      if (i < best_instructions) {
        best_instructions = i;
      }
    }
    if (best_cycles == (u64)-1) {
      best_cycles = 0;
    }
    if (best_instructions == (u64)-1) {
      best_instructions = 0;
    }
    p->overhead_cycles = best_cycles;
    p->overhead_instructions = best_instructions;
  }
}

static void ubench_perf_close(struct ubench_perf_s *p) {
  if (p->group_fd >= 0) {
    sp_syscall(SP_SYSCALL_NUM_CLOSE, p->instr_fd);
    sp_syscall(SP_SYSCALL_NUM_CLOSE, p->group_fd);
    p->group_fd = -1;
    p->instr_fd = -1;
  }
}
#endif

SP_END_EXTERN_C()

#endif // SP_BENCH_H



#if defined SP_IMPLEMENTATION && !defined(SP_BENCH_IMPLEMENTATION)
  #define SP_BENCH_IMPLEMENTATION
#endif

#if defined(SP_BENCH_IMPLEMENTATION)

#include <sys/stat.h>
#include <time.h>

#if defined(SP_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

SP_BEGIN_EXTERN_C()

#if defined(UBENCH_ENABLE_SQLITE)
struct bench_store {
  sqlite3 * db;
  sqlite3_stmt* result_stmt;
  sp_mem_arena_t* arena;
  sp_mem_t mem;
};

void bench__read_first_field(const c8* path, const c8* prefix, c8* dst, u32 dst_size) {
  FILE *f = fopen(path, "r");
  c8 line[1024];
  u32 plen = (u32)strlen(prefix);
  dst[0] = '\0';
  if (!f) return;
  while (fgets(line, sizeof(line), f)) {
    if (strncmp(line, prefix, plen) == 0) {
      const c8 *p = line + plen;
      u32 i = 0;
      while (*p == ' ' || *p == '\t' || *p == ':') p++;
      while (*p && *p != '\n' && i + 1 < dst_size) dst[i++] = *p++;
      dst[i] = '\0';
      break;
    }
  }
  fclose(f);
}

void bench__make_fingerprint(const bench_machine_info* m, c8* dst, u32 dst_size) {
  s64 mem_gb = (m->memory_bytes + (1LL << 30) - 1) / (1LL << 30);
  snprintf(dst, dst_size, "%s|%s|%s|%d|%lld",
           m->os_name[0]   ? m->os_name   : "Unknown",
           m->arch[0]      ? m->arch      : "unknown",
           m->cpu_model[0] ? m->cpu_model : "unknown",
           (int)m->cpu_threads,
           (long long)mem_gb);
}


#if defined(SP_LINUX)

static sp_str_t ubench_cpu_read_file_a(sp_mem_t mem, const c8 *path) {
  sp_sys_fd_t fd = (sp_sys_fd_t)sp_syscall(
      SP_SYSCALL_NUM_OPENAT, SP_AT_FDCWD, path, SP_O_RDONLY, 0);
  if (fd < 0) return sp_zero_s(sp_str_t);

  u64 cap = 4096;
  c8 *buf = sp_alloc_n(mem, c8, cap);
  u64 len = 0;
  for (;;) {
    if (len == cap) {
      u64 new_cap = cap * 2;
      c8 *grown = sp_alloc_n(mem, c8, new_cap);
      sp_mem_copy(grown, buf, len);
      buf = grown;
      cap = new_cap;
    }
    s64 n = sp_syscall(SP_SYSCALL_NUM_READ, fd, buf + len, cap - len);
    if (n <= 0) break;
    len += (u64)n;
  }
  sp_syscall(SP_SYSCALL_NUM_CLOSE, fd);
  return (sp_str_t) { .data = buf, .len = (u32)len };
}

sp_str_t sp_cpu_get_model_a(sp_mem_t mem) {
  sp_mem_arena_marker_t s = sp_mem_begin_scratch_for(mem);
  sp_str_t cpuinfo = ubench_cpu_read_file_a(s.mem, "/proc/cpuinfo");
  sp_str_t result = sp_zero_s(sp_str_t);

  sp_da(sp_str_t) lines = sp_str_split_c8(s.mem, cpuinfo, '\n');
  sp_da_for(lines, i) {
    sp_str_t line = lines[i];
    if (!sp_str_starts_with(line, sp_str_lit("model name")) &&
        !sp_str_starts_with(line, sp_str_lit("Hardware"))) {
      continue;
    }
    s32 colon = sp_str_find_c8(line, ':');
    if (colon < 0) continue;
    sp_str_t value = sp_str_sub(line, colon + 1, line.len - colon - 1);
    result = sp_str_copy(mem, sp_str_trim(value));
    break;
  }

  sp_mem_end_scratch(s);
  return result;
}

u32 sp_cpu_get_thread_count(void) {
  /* glibc cpu_set_t is 1024 bits; the kernel pads to 8-byte multiples. */
  u8 mask[128] = sp_zero;
  s64 rc = sp_syscall(SP_SYSCALL_NUM_SCHED_GETAFFINITY, 0, sizeof(mask), mask);
  if (rc <= 0) return 1;
  u32 count = 0;
  for (u64 i = 0; i < (u64)rc; i++) {
    u8 b = mask[i];
    while (b) { count += b & 1; b >>= 1; }
  }
  return count ? count : 1;
}

#elif defined(SP_MACOS)

sp_str_t sp_cpu_get_model_a(sp_mem_t mem) {
  c8     buf[256] = sp_zero;
  size_t len      = sizeof(buf);
  if (sysctlbyname("machdep.cpu.brand_string", buf, &len, NULL, 0) != 0) {
    return sp_zero_s(sp_str_t);
  }
  /* sysctlbyname returns len including the trailing NUL on success. */
  if (len > 0 && buf[len - 1] == '\0') len--;
  return sp_str_copy(mem, (sp_str_t){ .data = buf, .len = (u32)len });
}

u32 sp_cpu_get_thread_count(void) {
  int    v   = 0;
  size_t len = sizeof(v);
  if (sysctlbyname("hw.logicalcpu", &v, &len, NULL, 0) == 0 && v > 0) {
    return (u32)v;
  }
  return 1;
}

#elif defined(SP_WIN32)

sp_str_t sp_cpu_get_model_a(sp_mem_t mem) {
  HKEY  key;
  c8    buf[256] = sp_zero;
  DWORD len      = (DWORD)sizeof(buf);
  LONG  rc;
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                    "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                    0, KEY_READ, &key) != ERROR_SUCCESS) {
    return sp_zero_s(sp_str_t);
  }
  rc = RegQueryValueExA(key, "ProcessorNameString", NULL, NULL,
                        (LPBYTE)buf, &len);
  RegCloseKey(key);
  if (rc != ERROR_SUCCESS) return sp_zero_s(sp_str_t);
  /* RegQueryValueExA returns len including the trailing NUL for REG_SZ. */
  if (len > 0 && buf[len - 1] == '\0') len--;
  return sp_str_copy(mem, (sp_str_t){ .data = buf, .len = (u32)len });
}

u32 sp_cpu_get_thread_count(void) {
  SYSTEM_INFO si;
  GetNativeSystemInfo(&si);
  return si.dwNumberOfProcessors > 0 ? (u32)si.dwNumberOfProcessors : 1;
}

#else
  #error "ubench: sp_cpu_* impl missing for this platform"
#endif

SP_API s32 bench_collect_machine_info(bench_machine_info *m) {
  memset(m, 0, sizeof(*m));
#if defined(SP_WIN32)
  {
    DWORD n = (DWORD)sizeof(m->hostname);
    SYSTEM_INFO si;
    MEMORYSTATUSEX mem;
    if (!GetComputerNameA(m->hostname, &n)) m->hostname[0] = '\0';
    snprintf(m->os_name, sizeof(m->os_name), "%s", "Windows");
    /* os_version is intentionally left blank: GetVersionExA is deprecated and
       lies about the running OS, and RtlGetVersion needs a runtime ntdll
       resolve. The fingerprint is fine without it. */
    GetNativeSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
      case PROCESSOR_ARCHITECTURE_AMD64: snprintf(m->arch, sizeof(m->arch), "x86_64"); break;
      case PROCESSOR_ARCHITECTURE_ARM64: snprintf(m->arch, sizeof(m->arch), "aarch64"); break;
      case PROCESSOR_ARCHITECTURE_INTEL: snprintf(m->arch, sizeof(m->arch), "x86"); break;
      default: snprintf(m->arch, sizeof(m->arch), "unknown");
    }
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) m->memory_bytes = (s64)mem.ullTotalPhys;
  }
#else
  {
    struct utsname u;
    if (uname(&u) == 0) {
      snprintf(m->os_name,    sizeof(m->os_name),    "%s", u.sysname);
      snprintf(m->os_version, sizeof(m->os_version), "%s", u.release);
      snprintf(m->arch,       sizeof(m->arch),       "%s", u.machine);
    }
    if (gethostname(m->hostname, sizeof(m->hostname)) != 0) m->hostname[0] = '\0';
    m->hostname[sizeof(m->hostname) - 1] = '\0';
  }
#endif
#if defined(__linux__)
  {
    c8 buf[64];
    bench__read_first_field("/proc/meminfo", "MemTotal", buf, sizeof(buf));
    m->memory_bytes = (s64)atoll(buf) * 1024;
  }
#elif defined(__APPLE__)
  {
    s64 bytes = 0;
    size_t len = sizeof(bytes);
    if (sysctlbyname("hw.memsize", &bytes, &len, NULL, 0) == 0) m->memory_bytes = bytes;
  }
#endif

  {
    sp_mem_arena_marker_t s = sp_mem_begin_scratch();
    sp_str_t model = sp_cpu_get_model_a(s.mem);
    sp_cstr_copy_to_n(model.data, model.len,
                      m->cpu_model, sizeof(m->cpu_model));
    sp_mem_end_scratch(s);
  }
  m->cpu_threads = (s32)sp_cpu_get_thread_count();
  m->cpu_cores   = m->cpu_threads;
  return 0;
}

SP_IMP s32 bench__get_or_insert_machine(sqlite3 *db, const bench_machine_info *m, sqlite3_int64 *out_id) {
  c8            fingerprint[512];
  sqlite3_stmt *stmt = NULL;
  s32           rc;

  bench__make_fingerprint(m, fingerprint, sizeof(fingerprint));

  rc = sqlite3_prepare_v2(db, "SELECT id FROM machines WHERE fingerprint=?",
                          -1, &stmt, NULL);
  if (rc != SQLITE_OK) goto fail;
  sqlite3_bind_text(stmt, 1, fingerprint, -1, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    *out_id = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return 0;
  }
  sqlite3_finalize(stmt);

  rc = sqlite3_prepare_v2(
      db,
      "INSERT INTO machines (fingerprint, hostname, os_name, os_version, "
      "arch, cpu_model, cpu_cores, cpu_threads, memory_bytes) "
      "VALUES (?,?,?,?,?,?,?,?,?)",
      -1, &stmt, NULL);
  if (rc != SQLITE_OK) goto fail;
  sqlite3_bind_text (stmt, 1, fingerprint,  -1, SQLITE_TRANSIENT);
  sqlite3_bind_text (stmt, 2, m->hostname,  -1, SQLITE_TRANSIENT);
  sqlite3_bind_text (stmt, 3, m->os_name,   -1, SQLITE_TRANSIENT);
  sqlite3_bind_text (stmt, 4, m->os_version,-1, SQLITE_TRANSIENT);
  sqlite3_bind_text (stmt, 5, m->arch,      -1, SQLITE_TRANSIENT);
  sqlite3_bind_text (stmt, 6, m->cpu_model, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int  (stmt, 7, m->cpu_cores);
  sqlite3_bind_int  (stmt, 8, m->cpu_threads);
  sqlite3_bind_int64(stmt, 9, m->memory_bytes);
  if (sqlite3_step(stmt) != SQLITE_DONE) goto fail;
  *out_id = sqlite3_last_insert_rowid(db);
  sqlite3_finalize(stmt);
  return 0;
fail:
  fprintf(stderr, "bench machine: %s\n", sqlite3_errmsg(db));
  if (stmt) sqlite3_finalize(stmt);
  return -1;
}

s32 bench__get_or_insert_benchmark(sqlite3* db, const c8* name, sqlite3_int64* out_id) {
  sqlite3_stmt *stmt = NULL;
  s32 rc = sqlite3_prepare_v2(db, "SELECT id FROM benchmarks WHERE name=?",
                              -1, &stmt, NULL);
  if (rc != SQLITE_OK) goto fail;
  sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    *out_id = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return 0;
  }
  sqlite3_finalize(stmt);

  rc = sqlite3_prepare_v2(db, "INSERT INTO benchmarks (name) VALUES (?)",
                          -1, &stmt, NULL);
  if (rc != SQLITE_OK) goto fail;
  sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt) != SQLITE_DONE) goto fail;
  *out_id = sqlite3_last_insert_rowid(db);
  sqlite3_finalize(stmt);
  return 0;
fail:
  fprintf(stderr, "bench benchmark: %s\n", sqlite3_errmsg(db));
  if (stmt) sqlite3_finalize(stmt);
  return -1;
}

void bench__bind_text_or_null(sqlite3_stmt *stmt, int idx, const c8 *s) {
  if (s) {
    sqlite3_bind_text(stmt, idx, s, -1, SQLITE_TRANSIENT);
  } else {
    sqlite3_bind_null(stmt, idx);
  }
}

void bench__bind_i64_or_null(sqlite3_stmt *stmt, int idx, s64 v) {
  if (v == BENCH_UNSET_I64) {
    sqlite3_bind_null(stmt, idx);
  } else {
    sqlite3_bind_int64(stmt, idx, v);
  }
}

void bench__bind_f64_or_null(sqlite3_stmt *stmt, int idx, f64 v) {
  if (v <= BENCH_UNSET_F64) {
    sqlite3_bind_null(stmt, idx);
  } else {
    sqlite3_bind_double(stmt, idx, v);
  }
}

bench_store* bench_store_open(const c8 *path) {
  sp_mem_t mem = sp_mem_os_new();

  bench_store* s = sp_alloc_type(mem, bench_store);
  s->arena = sp_mem_arena_new(mem);
  s->mem = sp_mem_arena_as_allocator(s->arena);

  if (sqlite3_open(path, &s->db) != SQLITE_OK) {
    fprintf(stderr, "bench open: %s\n", sqlite3_errmsg(s->db));
    sqlite3_close(s->db);
    goto error;
  }
  sqlite3_exec(s->db, "PRAGMA journal_mode=WAL", 0, 0, 0);
  sqlite3_exec(s->db, "PRAGMA foreign_keys=ON", 0, 0, 0);
  if (sqlite3_exec(s->db, BENCH_STORE_SCHEMA, 0, 0, 0) != SQLITE_OK) {
    fprintf(stderr, "bench schema: %s\n", sqlite3_errmsg(s->db));
    sqlite3_close(s->db);
    goto error;
  }

  return s;

error:
  if (s) sp_mem_allocator_free(mem, s);
  return SP_NULLPTR;
}

void bench_store_close(bench_store* s) {
  if (!s) return;
  if (s->result_stmt) sqlite3_finalize(s->result_stmt);
  if (s->db)          sqlite3_close(s->db);

  sp_mem_t mem = sp_mem_os_new();
  sp_mem_allocator_free(mem, s);
}

s64 bench_store_begin_run(bench_store* s, const bench_machine_info* mi, const bench_run_info* ri) {
  sqlite3_int64 machine_id = 0, run_id = 0;
  sqlite3_stmt *stmt = NULL;
  c8            started_at[32];

  if (!s || !s->db || !mi || !ri) return -1;
  if (bench__get_or_insert_machine(s->db, mi, &machine_id) != 0) return -1;

  //bench__iso_time_now(started_at, sizeof(started_at));
  sp_mem_fixed_t mem = sp_mem_fixed(started_at, sizeof(started_at));
  sp_tm_epoch_to_iso8601(sp_mem_fixed_as_allocator(&mem), sp_tm_now_epoch());

  if (sqlite3_prepare_v2(
          s->db,
          "INSERT INTO runs (machine_id, started_at, executable_path, "
          "executable_size_bytes, executable_mtime, confidence_threshold, "
          "filter, has_perf_counters, label, framework, metadata) "
          "VALUES (?,?,?,?,?,?,?,?,?,?,?)",
          -1, &stmt, NULL) != SQLITE_OK) goto fail;

  sqlite3_bind_int64(stmt, 1, machine_id);
  sqlite3_bind_text (stmt, 2, started_at, -1, SQLITE_TRANSIENT);
  bench__bind_text_or_null(stmt, 3, ri->executable_path);
  if (ri->executable_size_bytes > 0)
    sqlite3_bind_int64(stmt, 4, ri->executable_size_bytes);
  else
    sqlite3_bind_null(stmt, 4);
  bench__bind_text_or_null(stmt, 5, ri->executable_mtime);
  if (ri->confidence_threshold >= 0)
    sqlite3_bind_double(stmt, 6, ri->confidence_threshold);
  else
    sqlite3_bind_null(stmt, 6);
  bench__bind_text_or_null(stmt, 7, ri->filter);
  if (ri->has_perf_counters >= 0) sqlite3_bind_int (stmt, 8, ri->has_perf_counters);
  else                            sqlite3_bind_null(stmt, 8);
  bench__bind_text_or_null(stmt,  9, ri->label);
  bench__bind_text_or_null(stmt, 10, ri->framework);
  bench__bind_text_or_null(stmt, 11, ri->metadata_json);

  if (sqlite3_step(stmt) != SQLITE_DONE) goto fail;
  run_id = sqlite3_last_insert_rowid(s->db);
  sqlite3_finalize(stmt);

  /* Batch all subsequent INSERTs into one transaction; bench_store_end_run
     commits. The prepared INSERT is reused for every record. */
  sqlite3_exec(s->db, "BEGIN TRANSACTION", 0, 0, 0);
  if (sqlite3_prepare_v2(
          s->db,
          "INSERT INTO results (run_id, benchmark_id, iterations, mean_ns, "
          "median_ns, min_ns, max_ns, stddev_ns, stddev_pct, ci_low_ns, "
          "ci_high_ns, ci_level_pct, confidence_pct, bytes_processed, "
          "items_processed, cycles_per_iter, instructions_per_iter) "
          "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
          -1, &s->result_stmt, NULL) != SQLITE_OK) goto fail;

  return (s64)run_id;
fail:
  fprintf(stderr, "bench begin_run: %s\n", sqlite3_errmsg(s->db));
  if (stmt) sqlite3_finalize(stmt);
  return -1;
}

s32 bench_store_record(bench_store* s, s64 run_id, const c8* bench_name, const bench_result* r) {
  sqlite3_int64 bench_id = 0;
  if (!s || !s->result_stmt || !bench_name || !r) return -1;
  if (bench__get_or_insert_benchmark(s->db, bench_name, &bench_id) != 0) return -1;

  sqlite3_reset(s->result_stmt);
  sqlite3_bind_int64 (s->result_stmt,  1, run_id);
  sqlite3_bind_int64 (s->result_stmt,  2, bench_id);
  sqlite3_bind_int64 (s->result_stmt,  3, r->iterations);
  sqlite3_bind_double(s->result_stmt,  4, r->mean_ns);
  sqlite3_bind_double(s->result_stmt,  5, r->median_ns);
  sqlite3_bind_double(s->result_stmt,  6, r->min_ns);
  sqlite3_bind_double(s->result_stmt,  7, r->max_ns);
  bench__bind_f64_or_null(s->result_stmt,  8, r->stddev_ns);
  bench__bind_f64_or_null(s->result_stmt,  9, r->stddev_pct);
  bench__bind_f64_or_null(s->result_stmt, 10, r->ci_low_ns);
  bench__bind_f64_or_null(s->result_stmt, 11, r->ci_high_ns);
  bench__bind_f64_or_null(s->result_stmt, 12, r->ci_level_pct);
  bench__bind_f64_or_null(s->result_stmt, 13, r->confidence_pct);
  bench__bind_i64_or_null(s->result_stmt, 14, r->bytes_processed);
  bench__bind_i64_or_null(s->result_stmt, 15, r->items_processed);
  bench__bind_i64_or_null(s->result_stmt, 16, r->cycles_per_iter);
  bench__bind_i64_or_null(s->result_stmt, 17, r->instructions_per_iter);

  if (sqlite3_step(s->result_stmt) != SQLITE_DONE) {
    fprintf(stderr, "bench record: %s\n", sqlite3_errmsg(s->db));
    return -1;
  }
  return 0;
}

s32 bench_store_end_run(bench_store* s, s64 run_id) {
  c8 finished_at[32];
  sqlite3_stmt *stmt = NULL;
  if (!s || !s->db) return -1;

  if (s->result_stmt) {
    sqlite3_finalize(s->result_stmt);
    s->result_stmt = NULL;
    sqlite3_exec(s->db, "COMMIT TRANSACTION", 0, 0, 0);
  }

  sp_mem_fixed_t mem = sp_mem_fixed(finished_at, sizeof(finished_at));
  sp_tm_epoch_to_iso8601(sp_mem_fixed_as_allocator(&mem), sp_tm_now_epoch());
  sqlite3_prepare_v2(s->db, "UPDATE runs SET finished_at=? WHERE id=?",
                     -1, &stmt, NULL);
  sqlite3_bind_text (stmt, 1, finished_at, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 2, run_id);
  sqlite3_step    (stmt);
  sqlite3_finalize(stmt);
  return 0;
}

s64 bench_simple_begin_run(
  bench_store* s,
  const c8* framework, const c8* label,
  const c8* executable_path, s64 executable_size_bytes, const c8* executable_mtime,
  f64 confidence_threshold,
  s32 has_perf_counters
) {
  bench_machine_info mi;
  bench_run_info     ri;
  bench_collect_machine_info(&mi);
  memset(&ri, 0, sizeof(ri));
  ri.framework             = framework;
  ri.label                 = label;
  ri.executable_path       = executable_path;
  ri.executable_size_bytes = executable_size_bytes;
  ri.executable_mtime      = executable_mtime;
  ri.confidence_threshold  = confidence_threshold;
  ri.has_perf_counters     = has_perf_counters;
  return bench_store_begin_run(s, &mi, &ri);
}

s32 bench_simple_record(
  bench_store* s, s64 run_id,
  const c8* name,
  s64 iterations,
  f64 mean_ns, f64 median_ns,
  f64 min_ns, f64 max_ns,
  f64 stddev_ns, f64 stddev_pct,
  f64 ci_low_ns, f64 ci_high_ns, f64 ci_level_pct,
  f64 confidence_pct,
  s64 bytes_processed, s64 items_processed,
  s64 cycles_per_iter, s64 instructions_per_iter
) {
  bench_result r;
  r.iterations            = iterations;
  r.mean_ns               = mean_ns;
  r.median_ns             = median_ns;
  r.min_ns                = min_ns;
  r.max_ns                = max_ns;
  r.stddev_ns             = stddev_ns;
  r.stddev_pct            = stddev_pct;
  r.ci_low_ns             = ci_low_ns;
  r.ci_high_ns            = ci_high_ns;
  r.ci_level_pct          = ci_level_pct;
  r.confidence_pct        = confidence_pct;
  r.bytes_processed       = bytes_processed;
  r.items_processed       = items_processed;
  r.cycles_per_iter       = cycles_per_iter;
  r.instructions_per_iter = instructions_per_iter;
  return bench_store_record(s, run_id, name, &r);
}
#endif

SP_END_EXTERN_C()

static SP_INLINE s32 ubench_main(s32 argc, const c8 *const argv[]);
s32 ubench_main(s32 argc, const c8 *const argv[]) {
  u64 failed = 0;
  ubench_size_t index = 0;
  ubench_size_t *failed_benchmarks = SP_NULLPTR;
  ubench_size_t failed_benchmarks_length = 0;
  const c8 *filter = SP_NULLPTR;
  u64 ran_benchmarks = 0;

  sp_fmt_register_decorator("green", ubench_fmt_tty_green, ubench_fmt_tty_reset);
  sp_fmt_register_decorator("red", ubench_fmt_tty_red, ubench_fmt_tty_reset);

#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
  struct ubench_perf_s perf;
  ubench_perf_init(&perf);
#endif
#if defined(UBENCH_ENABLE_SQLITE)
  const c8 *db_path = "./ubench.db";
  bench_store *store = SP_NULLPTR;
  s64 run_id = 0;
#endif

  /* loop through all arguments looking for our options */
  for (index = 1; index < sp_cast(ubench_size_t, argc); index++) {
    /* Informational switches */
    const sp_str_t help_str = sp_str_lit("--help");
    const sp_str_t list_str = sp_str_lit("--list-benchmarks");
    /* Benchmark config switches */
    const sp_str_t filter_str = sp_str_lit("--filter=");
#if defined(UBENCH_ENABLE_SQLITE)
    const sp_str_t output_str = sp_str_lit("--output=");
#endif
    const sp_str_t confidence_str = sp_str_lit("--confidence=");
    const sp_str_t arg = sp_cstr_as_str(argv[index]);

    if (sp_str_starts_with(arg, help_str)) {
      sp_log("ubench.h - the single file benchmarking solution for C/C++!");
      sp_log("Command line Options:");
      sp_log("  --help                    Show this message and exit.");
      sp_log("  --filter=<filter>         Filter the benchmarks to run (EG. "
               "MyBench*.a would run MyBenchmark.a but not MyBenchmark.b).");
      sp_log("  --list-benchmarks         List benchmarks, one per line. "
               "Output names can be passed to --filter.");
#if defined(UBENCH_ENABLE_SQLITE)
      sp_log("  --output=<path|none>      SQLite database to write results to "
               "(default ./ubench.db, 'none' disables).");
#endif
      sp_log("  --confidence=<percent>    MdAPE (median absolute percent "
               "error) cut-off above which a benchmark is reported as failed. "
               "Defaults to 2.5%");
      goto cleanup;
    } else if (sp_str_starts_with(arg, filter_str)) {
      /* user wants to filter what benchmarks run! */
      filter = argv[index] + filter_str.len;
#if defined(UBENCH_ENABLE_SQLITE)
    } else if (sp_str_starts_with(arg, output_str)) {
      const c8 *value = argv[index] + output_str.len;
      if (sp_cstr_equal(value, "none")) {
        db_path = SP_NULLPTR;
      } else {
        db_path = value;
      }
#endif
    } else if (sp_str_starts_with(arg, list_str)) {
      for (index = 0; index < ubench_state.benchmarks_length; index++) {
        sp_log("{}", sp_fmt_str(ubench_state.benchmarks[index].name));
      }

      /* when printing the benchmark list, don't actually run the benchmarks */
      goto cleanup;
    } else if (sp_str_starts_with(arg, confidence_str)) {
      /* user wants to specify a different confidence */
      ubench_state.confidence =
          sp_parse_f64(sp_cstr_as_str(argv[index] + confidence_str.len));

      /* must be between 0 and 100 */
      if ((ubench_state.confidence < 0) || (ubench_state.confidence > 100)) {
        sp_print_err(
            "Confidence must be in the range [0..100] (you specified {})\n",
            sp_fmt_float(ubench_state.confidence));
        goto cleanup;
      }
    }
  }

  for (index = 0; index < ubench_state.benchmarks_length; index++) {
    if (ubench_should_filter(filter, ubench_state.benchmarks[index].name.data)) {
      continue;
    }

    ran_benchmarks++;
  }

  sp_log("{.green} Running {} benchmarks.", sp_fmt_cstr("[==========]"), sp_fmt_uint(ran_benchmarks));

#if defined(UBENCH_ENABLE_SQLITE)
  if (db_path) {
    store = bench_store_open(db_path);
    if (store) {
      bench_machine_info mi;
      bench_run_info     ri;
      c8                 exe_path_buf[4096];
      c8                 exe_mtime_buf[32];
      s64                exe_size = 0;
      s32                has_perf = 0;
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
      has_perf = perf.group_fd >= 0 ? 1 : 0;
#endif
      exe_path_buf[0]  = '\0';
      exe_mtime_buf[0] = '\0';

      bench_collect_machine_info(&mi);

      /* Collect executable info via sp_fs_*; bench_run_info just holds
         pointers into these stack buffers (live until bench_store_begin_run
         returns, which is what binds the strings into SQLite). */
      {
        sp_mem_arena_marker_t s = sp_mem_begin_scratch();
        sp_str_t exe = sp_fs_get_exe_path(s.mem);
        sp_cstr_copy_to_n(exe.data, exe.len,
                          exe_path_buf, sizeof(exe_path_buf));
        if (exe_path_buf[0]) {
          struct stat st;
          if (stat(exe_path_buf, &st) == 0) {
            exe_size = (s64)st.st_size;
            sp_str_t mtime = sp_tm_epoch_to_iso8601(
                s.mem, sp_fs_get_mod_time(exe));
            sp_cstr_copy_to_n(mtime.data, mtime.len,
                              exe_mtime_buf, sizeof(exe_mtime_buf));
          }
        }
        sp_mem_end_scratch(s);
      }

      sp_mem_zero(&ri, sizeof(ri));
      ri.executable_path       = exe_path_buf[0]    ? exe_path_buf  : SP_NULLPTR;
      ri.executable_size_bytes = exe_size > 0       ? exe_size      : BENCH_UNSET_I64;
      ri.executable_mtime      = exe_mtime_buf[0]   ? exe_mtime_buf : SP_NULLPTR;
      ri.filter                = filter;
      ri.framework             = "ubench";
      ri.confidence_threshold  = ubench_state.confidence;
      ri.has_perf_counters     = has_perf;

      run_id = bench_store_begin_run(store, &mi, &ri);
      if (run_id < 0) {
        bench_store_close(store);
        store  = SP_NULLPTR;
        run_id = 0;
      }
    }
  }
#endif

  for (index = 0; index < ubench_state.benchmarks_length; index++) {
    s32 result = 0;
    s64 kndex = 0;
    s64 cal_ns = 0;
    s64 epochs = 0;
    /* Per-body times stored as doubles end-to-end so sub-nanosecond bodies
       (UBENCH_LOOP-batched microbenchmarks) survive both the display path
       and the SQL bind. The unit is nanoseconds. */
    f64 best_avg_ns = 0;
    f64 best_min_ns = 0;
    f64 best_median_ns = 0;
    f64 best_max_ns = 0;
    f64 best_deviation = 0;
    f64 best_confidence = 0;
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
    u64 best_cycles = 0;
    u64 best_instructions = 0;
    u64 pass_cycles = 0;
    u64 pass_instructions = 0;
#endif
    struct ubench_run_state_s ubs;

#define UBENCH_MIN_EPOCHS 16
#define UBENCH_MAX_EPOCHS 500
    const s64 max_epochs = UBENCH_MAX_EPOCHS;
    const s64 min_epochs = UBENCH_MIN_EPOCHS;
    /* Add one extra timestamp slot: each sample stores the timestamp at its
       start, plus one final timestamp after the last sample exits. */
    s64 ns[UBENCH_MAX_EPOCHS + 1];
    s64 pause_ns[UBENCH_MAX_EPOCHS + 1];
    /* Scratch for MdAPE computation (relative deviations in ppm). */
    s64 mdape_scratch[UBENCH_MAX_EPOCHS];
#undef UBENCH_MAX_EPOCHS
#undef UBENCH_MIN_EPOCHS

    if (ubench_should_filter(filter, ubench_state.benchmarks[index].name.data)) {
      continue;
    }

    sp_str_t name = ubench_state.benchmarks[index].name;
    sp_log("[ {:<9 .green}] {}", sp_fmt_cstr("RUN"), sp_fmt_str(name));

    ubs.ns = ns;
    ubs.pause_ns = pause_ns;
    ubs.size = 1;
    ubs.sample = 0;
    ubs.paused_ns = 0;
    ubs.pause_start = 0;
    ubs.bytes_processed = 0;
    ubs.items_processed = 0;
    ubs.batch = 1;
    ubs.batch_consumed = 0;

    /* CALIBRATION: one body invocation, batch=1, to estimate single-body
       cost. The body announces UBENCH_LOOP usage by setting batch_consumed. */
    ubench_invoke(&ubench_state.benchmarks[index], &ubs);
    cal_ns = ns[1] - ns[0] - pause_ns[0];
    if (cal_ns <= 0) {
      cal_ns = 1;
    }

    /* Auto-tune batch only if the body opted in via UBENCH_LOOP. The clock-
       bracketed window is sized to ~1 ms regardless of body cost, giving
       (1 ms / clock_overhead) ~= 10^4..10^6 amortization for sub-µs bodies. */
    ubs.batch = 1;
    if (ubs.batch_consumed) {
      const s64 target_batch_ns = 1 * 1000 * 1000;
      s64 b = target_batch_ns / cal_ns;
      if (b < 1) {
        b = 1;
      }
      ubs.batch = b;
    }

    /* Choose epoch count: target ~100 ms total wall-time across all samples,
       but never fewer than min_epochs (so the median has a meaningful base). */
    {
      const s64 target_total_ns = 100 * 1000 * 1000;
      const s64 per_sample_ns = ubs.batch * cal_ns;
      epochs = target_total_ns / (per_sample_ns > 0 ? per_sample_ns : 1);
      if (epochs < min_epochs) {
        epochs = min_epochs;
      }
      if (epochs > max_epochs) {
        epochs = max_epochs;
      }
    }

    /* WARMUP: one throwaway sample at the chosen batch, to prime caches,
       branch predictor, TLB, and demand-page any lazy allocations. */
    ubs.size = 1;
    ubs.sample = 0;
    ubs.paused_ns = 0;
    ubs.pause_start = 0;
    ubench_invoke(&ubench_state.benchmarks[index], &ubs);

    /* MEASUREMENT */
    ubs.size = epochs;
    ubs.sample = 0;
    ubs.paused_ns = 0;
    ubs.pause_start = 0;
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
    ubench_perf_start(&perf);
#endif
    ubench_invoke(&ubench_state.benchmarks[index], &ubs);
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
    ubench_perf_stop(&perf, &pass_cycles, &pass_instructions);
#endif

    /* Convert raw timestamps to per-batch deltas (in ns) in place. The /batch
       split is deferred to f64-precision arithmetic below so that sub-ns
       per-body times don't get truncated to zero. */
    for (kndex = 0; kndex < epochs; kndex++) {
      s64 d = ns[kndex + 1] - ns[kndex] - pause_ns[kndex];
      if (d < 0) {
        d = 0;
      }
      ns[kndex] = d;
    }

    /* Mean per body in f64. */
    {
      const f64 batch_d = sp_cast(f64, ubs.batch);
      f64 sum = 0;
      for (kndex = 0; kndex < epochs; kndex++) {
        sum += sp_cast(f64, ns[kndex]);
      }
      best_avg_ns = sum / (sp_cast(f64, epochs) * batch_d);

      /* Sample stddev (kept for legacy reporting). */
      {
        f64 var = 0;
        for (kndex = 0; kndex < epochs; kndex++) {
          const f64 v =
              sp_cast(f64, ns[kndex]) / batch_d - best_avg_ns;
          var += v * v;
        }
        var /= sp_cast(f64, epochs);
        best_deviation =
            (best_avg_ns > 0)
                ? ((f64)sp_sys_sqrtf((f32)var) / best_avg_ns) * 100.0
                : 0.0;
      }
    }

    /* Sort raw per-batch samples to derive median, min, max. MdAPE is
       scale-invariant ((x-med)/med), so it works on raw samples without /batch
       — the batch factor cancels in the ratio. */
    sp_os_qsort(ns, sp_cast(ubench_size_t, epochs), sizeof(*ns), ubench_int64_cmp);
    {
      const s64 raw_median = ns[epochs / 2];
      const f64 batch_d = sp_cast(f64, ubs.batch);
      best_min_ns = sp_cast(f64, ns[0]) / batch_d;
      best_median_ns = sp_cast(f64, raw_median) / batch_d;
      best_max_ns = sp_cast(f64, ns[epochs - 1]) / batch_d;

      /* MdAPE = median of |x - median| / median, in percent. Robust against
         the heavy-tailed one-sided noise typical of microbenchmark
         distributions (preemption, page faults, IRQs, frequency steps).
         Replaces the prior Gaussian CI, which was structurally wrong here. */
      best_confidence = 0.0;
      if (raw_median > 0) {
        for (kndex = 0; kndex < epochs; kndex++) {
          s64 v = ns[kndex] - raw_median;
          if (v < 0) {
            v = -v;
          }
          mdape_scratch[kndex] = (v * 1000000) / raw_median;
        }
        sp_os_qsort(mdape_scratch, sp_cast(ubench_size_t, epochs), sizeof(*mdape_scratch), ubench_int64_cmp);
        {
          const s64 mid = epochs / 2;
          best_confidence =
              sp_cast(f64, mdape_scratch[mid]) / 10000.0;
        }
      }
    }
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
    {
      const u64 total_bodies =
          sp_cast(u64, epochs) *
          sp_cast(u64, ubs.batch);
      if (total_bodies > 0) {
        best_cycles = pass_cycles / total_bodies;
        best_instructions = pass_instructions / total_bodies;
      }
    }
#endif

    /* Flag the benchmark as failed if MdAPE exceeds the user threshold. */
    result = best_confidence > ubench_state.confidence;

    if (result) {
      sp_log("MdAPE {}% exceeds maximum permitted {}%",
               sp_fmt_float(best_confidence),
               sp_fmt_float(ubench_state.confidence));
    }

    {
      const f64 bps = (ubs.bytes_processed > 0 && best_avg_ns > 0)
          ? sp_cast(f64, ubs.bytes_processed) * 1e9 /
                sp_cast(f64, best_avg_ns)
          : 0.0;
      const f64 ips = (ubs.items_processed > 0 && best_avg_ns > 0)
          ? sp_cast(f64, ubs.items_processed) * 1e9 /
                sp_cast(f64, best_avg_ns)
          : 0.0;

#if defined(UBENCH_ENABLE_SQLITE)
      if (store) {
        bench_result br;
        sp_mem_zero(&br, sizeof(br));
        br.iterations            = epochs;
        br.mean_ns               = best_avg_ns;
        br.median_ns             = best_median_ns;
        br.min_ns                = best_min_ns;
        br.max_ns                = best_max_ns;
        br.stddev_ns             = best_deviation * best_avg_ns / 100.0;
        br.stddev_pct            = best_deviation;
        br.ci_low_ns             = BENCH_UNSET_F64;
        br.ci_high_ns            = BENCH_UNSET_F64;
        br.ci_level_pct          = BENCH_UNSET_F64;
        br.confidence_pct        = best_confidence;
        br.bytes_processed       = ubs.bytes_processed > 0 ? ubs.bytes_processed
                                                           : BENCH_UNSET_I64;
        br.items_processed       = ubs.items_processed > 0 ? ubs.items_processed
                                                           : BENCH_UNSET_I64;
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
        br.cycles_per_iter       = perf.group_fd >= 0 ? (s64)best_cycles
                                                      : BENCH_UNSET_I64;
        br.instructions_per_iter = perf.group_fd >= 0 ? (s64)best_instructions
                                                      : BENCH_UNSET_I64;
#else
        br.cycles_per_iter       = BENCH_UNSET_I64;
        br.instructions_per_iter = BENCH_UNSET_I64;
#endif
        bench_store_record(store, run_id,
                           ubench_state.benchmarks[index].name.data, &br);
      }
#endif

      {
        const c8 *unit = "us";
        f64 scale_div = 1.0;

        if (0 != result) {
          const ubench_size_t failed_benchmark_index = failed_benchmarks_length++;
          failed_benchmarks = sp_ptr_cast(
              ubench_size_t *,
              sp_realloc(ubench_state.mem,
                           sp_ptr_cast(void *, failed_benchmarks),
                           sizeof(ubench_size_t) * failed_benchmarks_length));
          failed_benchmarks[failed_benchmark_index] = index;
          failed++;
        }

        if (0 != result) {
          sp_print("[{:^10 .red}] ", sp_fmt_cstr("FAILED"));
        } else {
          sp_print("[{:>9 .green} ] ", sp_fmt_cstr("OK"));
        }
        sp_print("{} (mean ", sp_fmt_str(ubench_state.benchmarks[index].name));

        /* Auto-scale display: pick a unit so the mean prints in [1, 1000). */
        if (best_avg_ns >= 1e9) {
          unit = "s";
          scale_div = 1e9;
        } else if (best_avg_ns >= 1e6) {
          unit = "ms";
          scale_div = 1e6;
        } else if (best_avg_ns >= 1e3) {
          unit = "us";
          scale_div = 1e3;
        } else if (best_avg_ns >= 1.0) {
          unit = "ns";
          scale_div = 1.0;
        } else {
          unit = "ps";
          scale_div = 1e-3;
        }
        sp_print("{:.3}{}, median {:.3}{}, min {:.3}{}, MdAPE {}%",
                   sp_fmt_float(best_avg_ns / scale_div), sp_fmt_cstr(unit),
                   sp_fmt_float(best_median_ns / scale_div), sp_fmt_cstr(unit),
                   sp_fmt_float(best_min_ns / scale_div), sp_fmt_cstr(unit),
                   sp_fmt_float(best_confidence));

        if (bps > 0.0) {
          const c8 *bps_unit;
          f64 bps_scaled;
          if (bps >= 1e9)      { bps_unit = "GB/s"; bps_scaled = bps / 1e9; }
          else if (bps >= 1e6) { bps_unit = "MB/s"; bps_scaled = bps / 1e6; }
          else if (bps >= 1e3) { bps_unit = "KB/s"; bps_scaled = bps / 1e3; }
          else                 { bps_unit = "B/s";  bps_scaled = bps; }
          sp_print(", {:.3} {}", sp_fmt_float(bps_scaled), sp_fmt_cstr(bps_unit));
        }
        if (ips > 0.0) {
          const c8 *ips_unit;
          f64 ips_scaled;
          if (ips >= 1e9)      { ips_unit = "G items/s"; ips_scaled = ips / 1e9; }
          else if (ips >= 1e6) { ips_unit = "M items/s"; ips_scaled = ips / 1e6; }
          else if (ips >= 1e3) { ips_unit = "K items/s"; ips_scaled = ips / 1e3; }
          else                 { ips_unit = "items/s";   ips_scaled = ips; }
          sp_print(", {:.3} {}", sp_fmt_float(ips_scaled), sp_fmt_cstr(ips_unit));
        }
#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
        if (perf.group_fd >= 0) {
          sp_print(", {} cycles, {} instructions",
                     sp_fmt_uint(best_cycles), sp_fmt_uint(best_instructions));
        }
#endif
        sp_log(")");
      }
    }
  }

  sp_log("{.green} {} benchmarks ran.",
           sp_fmt_cstr("[==========]"), sp_fmt_uint(ran_benchmarks));
  sp_log("[{:^10 .green}] {} benchmarks.",
           sp_fmt_cstr("PASSED"), sp_fmt_uint(ran_benchmarks - failed));

  if (0 != failed) {
    sp_log("[{:^10 .red}] {} benchmarks, listed below:",
             sp_fmt_cstr("FAILED"), sp_fmt_uint(failed));
    for (index = 0; index < failed_benchmarks_length; index++) {
      sp_log("[{:^10 .red}] {}",
               sp_fmt_cstr("FAILED"),
               sp_fmt_str(ubench_state.benchmarks[failed_benchmarks[index]].name));
    }
  }

cleanup:
  sp_free(ubench_state.mem, sp_ptr_cast(void *, failed_benchmarks));
  sp_free(ubench_state.mem, sp_ptr_cast(void *, ubench_state.benchmarks));

#if defined(UBENCH_ENABLE_PERF_COUNTERS) && defined(SP_LINUX)
  ubench_perf_close(&perf);
#endif
#if defined(UBENCH_ENABLE_SQLITE)
  if (store) {
    if (run_id > 0) bench_store_end_run(store, run_id);
    bench_store_close(store);
  }
#endif

  return sp_cast(s32, failed);
}

#endif // SP_BENCH_C
