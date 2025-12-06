#define SP_APP
#include "sp.h"

#include "glob.h"

#define BENCH_ITERATIONS 1000000

typedef struct {
  const c8* pattern;
  const c8* path;
} bench_case_t;

typedef struct {
  sp_str_t name;
  f64 ns_per_op;
} bench_result_t;

// From original rust globset benchmarks
static bench_case_t bench_cases[] = {
  {.pattern = "*.txt",              .path = "some/a/bigger/path/to/the/crazy/needle.txt"},
  {.pattern = "some/**/needle.txt", .path = "some/needle.txt"},
  {.pattern = "some/**/needle.txt", .path = "some/a/bigger/path/to/the/crazy/needle.txt"},
};

static const c8* case_names[] = {
  "ext", "short", "long",
};

// From original rust globset benchmarks
static const c8* many_short_patterns[] = {
  ".*.swp",
  "tags",
  "target",
  "*.lock",
  "tmp",
  "*.csv",
  "*.fst",
  "*-got",
  "*.csv.idx",
  "words",
  "98m*",
  "dict",
  "test",
  "months",
};

static const c8* many_short_path = "98m-blah.csv.idx";

static f64 run_glob_bench(sp_glob_t* g, sp_str_t path) {
  for (u32 i = 0; i < 1000; i++) {
    sp_glob_match(g, path);
  }

  volatile bool result;
  sp_tm_point_t start = sp_tm_now_point();
  for (u32 i = 0; i < BENCH_ITERATIONS; i++) {
    result = sp_glob_match(g, path);
  }
  sp_tm_point_t end = sp_tm_now_point();
  (void)result;

  return (f64)sp_tm_point_diff(end, start) / (f64)BENCH_ITERATIONS;
}

static f64 run_glob_set_bench(sp_glob_set_t* set, sp_str_t path) {
  for (u32 i = 0; i < 1000; i++) {
    sp_glob_set_match(set, path);
  }

  volatile bool result;
  sp_tm_point_t start = sp_tm_now_point();
  for (u32 i = 0; i < BENCH_ITERATIONS; i++) {
    result = sp_glob_set_match(set, path);
  }
  sp_tm_point_t end = sp_tm_now_point();
  (void)result;

  return (f64)sp_tm_point_diff(end, start) / (f64)BENCH_ITERATIONS;
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  sp_mem_arena_t* arena = sp_mem_arena_new(4 * 1024 * 1024);
  sp_allocator_t allocator = sp_mem_arena_as_allocator(arena);
  sp_context_push_allocator(allocator);

  u32 num_cases = sizeof(bench_cases) / sizeof(bench_cases[0]);
  sp_da(bench_result_t) results = SP_NULLPTR;

  // Pre-compile all globs
  sp_da(sp_glob_t*) globs = SP_NULLPTR;
  sp_da(sp_glob_set_t*) globsets = SP_NULLPTR;
  sp_carr_for(bench_cases, i) {
    sp_glob_t* g = sp_glob_new(sp_str_view(bench_cases[i].pattern));
    SP_ASSERT(g != SP_NULLPTR);
    sp_dyn_array_push(globs, g);

    sp_glob_set_t* set = sp_glob_set_new();
    sp_glob_set_add(set, sp_glob_new(sp_str_view(bench_cases[i].pattern)));
    sp_glob_set_build(set);
    sp_dyn_array_push(globsets, set);
  }

  // Pre-compile many_short globset
  sp_glob_set_t* many_short_set = sp_glob_set_new();
  sp_carr_for(many_short_patterns, i) {
    sp_glob_set_add(many_short_set, sp_glob_new(sp_str_view(many_short_patterns[i])));
  }
  sp_glob_set_build(many_short_set);

  // Single glob benchmarks
  for (u32 i = 0; i < num_cases; i++) {
    sp_str_t path = sp_str_view(bench_cases[i].path);
    SP_ASSERT(sp_glob_match(globs[i], path));
    f64 ns = run_glob_bench(globs[i], path);
    sp_str_t name = sp_format("{}_glob", SP_FMT_CSTR(case_names[i]));
    sp_dyn_array_push(results, ((bench_result_t){.name = name, .ns_per_op = ns}));
  }

  // GlobSet single pattern benchmarks
  for (u32 i = 0; i < num_cases; i++) {
    sp_str_t path = sp_str_view(bench_cases[i].path);
    SP_ASSERT(sp_glob_set_match(globsets[i], path));
    f64 ns = run_glob_set_bench(globsets[i], path);
    sp_str_t name = sp_format("{}_globset", SP_FMT_CSTR(case_names[i]));
    sp_dyn_array_push(results, ((bench_result_t){.name = name, .ns_per_op = ns}));
  }

  // many_short benchmark (14 patterns, 2 matches expected)
  {
    sp_str_t path = sp_str_view(many_short_path);
    f64 ns = run_glob_set_bench(many_short_set, path);
    sp_dyn_array_push(results, ((bench_result_t){.name = SP_LIT("many_short_globset"), .ns_per_op = ns}));
  }

  // Print space-separated pairs
  sp_dyn_array_for(results, i) {
    SP_LOG("{} {}", SP_FMT_STR(results[i].name), SP_FMT_F64(results[i].ns_per_op));
  }

  return 0;
}
