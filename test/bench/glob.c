#include "ubench.h"

#include "sp/sp_glob.h"

#define GLOB_BENCH_MAX_PATTERNS 16

typedef struct {
  bool match;
} glob_bench_expect_t;

typedef struct {
  const c8* patterns[GLOB_BENCH_MAX_PATTERNS];
  const c8* path;
  glob_bench_expect_t expect;
} glob_bench_t;

static void run_glob_bench(ubench_run_state_t* ubench_run_state, glob_bench_t bench) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_glob_t* glob = sp_glob_new(scratch.mem, bench.patterns[0]);
  sp_str_t path = sp_str_view(bench.path);

  SP_ASSERT(glob != SP_NULLPTR);
  SP_ASSERT(sp_glob_match(glob, path) == bench.expect.match);

  UBENCH_DO_BENCHMARK() {
    UBENCH_LOOP {
      bool matched = sp_glob_match(glob, path);
      UBENCH_DO_NOT_OPTIMIZE(matched);
    }
  }

  sp_mem_end_scratch(scratch);
}

static void run_glob_set_bench(ubench_run_state_t* ubench_run_state, glob_bench_t bench) {
  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_glob_set_t* set = sp_glob_set_new(scratch.mem);
  sp_carr_for(bench.patterns, it) {
    if (!bench.patterns[it]) break;
    sp_glob_set_add(set, bench.patterns[it]);
  }
  sp_glob_set_build(set);
  sp_str_t path = sp_str_view(bench.path);

  SP_ASSERT(sp_glob_set_match(set, path) == bench.expect.match);

  UBENCH_DO_BENCHMARK() {
    UBENCH_LOOP {
      bool matched = sp_glob_set_match(set, path);
      UBENCH_DO_NOT_OPTIMIZE(matched);
    }
  }

  sp_mem_end_scratch(scratch);
}

UBENCH_EX(glob, ext) {
  run_glob_bench(ubench_run_state, (glob_bench_t) {
    .patterns = { "*.txt" },
    .path = "some/a/bigger/path/to/the/crazy/needle.txt",
    .expect = { .match = true },
  });
}

UBENCH_EX(glob, short) {
  run_glob_bench(ubench_run_state, (glob_bench_t) {
    .patterns = { "some/**/needle.txt" },
    .path = "some/needle.txt",
    .expect = { .match = true },
  });
}

UBENCH_EX(glob, long) {
  run_glob_bench(ubench_run_state, (glob_bench_t) {
    .patterns = { "some/**/needle.txt" },
    .path = "some/a/bigger/path/to/the/crazy/needle.txt",
    .expect = { .match = true },
  });
}

UBENCH_EX(globset, ext) {
  run_glob_set_bench(ubench_run_state, (glob_bench_t) {
    .patterns = { "*.txt" },
    .path = "some/a/bigger/path/to/the/crazy/needle.txt",
    .expect = { .match = true },
  });
}

UBENCH_EX(globset, short) {
  run_glob_set_bench(ubench_run_state, (glob_bench_t) {
    .patterns = { "some/**/needle.txt" },
    .path = "some/needle.txt",
    .expect = { .match = true },
  });
}

UBENCH_EX(globset, long) {
  run_glob_set_bench(ubench_run_state, (glob_bench_t) {
    .patterns = { "some/**/needle.txt" },
    .path = "some/a/bigger/path/to/the/crazy/needle.txt",
    .expect = { .match = true },
  });
}

UBENCH_EX(globset, many_short) {
  run_glob_set_bench(ubench_run_state, (glob_bench_t) {
    .patterns = {
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
    },
    .path = "98m-blah.csv.idx",
    .expect = { .match = true },
  });
}

UBENCH_MAIN()
