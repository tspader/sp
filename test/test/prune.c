#include "sp/sp_test.h"

#define PRUNE_MAX 8

typedef struct {
  const c8* id;
  u32 age_s;
  bool file;
} prune_seed_t;

typedef struct {
  const c8* name;
  prune_seed_t seeds [PRUNE_MAX];
  const c8* expect [PRUNE_MAX];
} prune_case_t;

static const prune_case_t prune_cases [] = {
  {
    .name = "stale",
    .seeds = {
      { .id = "A", .age_s = 10800 },
      { .id = "B", .age_s = 7200 },
      { .id = "C", .age_s = 5400 },
      { .id = "D", .age_s = 1800 },
      { .id = "E", .age_s = 600 },
    },
    .expect = { "C", "D", "E" },
  },
  {
    .name = "grace",
    .seeds = {
      { .id = "A", .age_s = 1800 },
      { .id = "B", .age_s = 1500 },
      { .id = "C", .age_s = 1200 },
      { .id = "D", .age_s = 900 },
      { .id = "E", .age_s = 600 },
    },
    .expect = { "A", "B", "C", "D", "E" },
  },
  {
    .name = "retained",
    .seeds = {
      { .id = "A", .age_s = 10800 },
      { .id = "B", .age_s = 7200 },
      { .id = "C", .age_s = 5400 },
    },
    .expect = { "A", "B", "C" },
  },
  {
    .name = "file",
    .seeds = {
      { .id = "A", .age_s = 10800, .file = true },
      { .id = "B", .age_s = 7200 },
      { .id = "C", .age_s = 5400 },
      { .id = "D", .age_s = 1800 },
      { .id = "E", .age_s = 600 },
    },
    .expect = { "A", "C", "D", "E" },
  },
};

static sp_str_t prune_run_name(sp_mem_t mem, sp_tm_epoch_t now, u32 age_s) {
  sp_tm_epoch_t time = { .s = now.s - age_s, .ns = now.ns };
  return sp_str_replace_c8(mem, sp_tm_epoch_to_iso8601(mem, time), ':', '-');
}

sp_test_each(runner, prune, prune_case_t, prune_cases) {
  sp_test_skip_on_wasm();
  sp_test_skip_on_freestanding();

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  sp_str_t exe = sp_fs_get_exe_path(mem);
  sp_str_t runs = sp_fs_join_path(mem, sandbox, sp_fs_get_stem(exe));
  sp_tm_epoch_t now = sp_tm_now_epoch();

  sp_must_ok(t, sp_fs_create_dir(runs));
  sp_str_t paths [PRUNE_MAX] = sp_zero;
  sp_carr_for(it->seeds, at) {
    const prune_seed_t* seed = &it->seeds[at];
    if (!seed->id) break;
    paths[at] = sp_fs_join_path(mem, runs, prune_run_name(mem, now, seed->age_s));
    if (seed->file) {
      sp_must_ok(t, sp_fs_create_file_cstr(paths[at], "A"));
    }
    else {
      sp_must_ok(t, sp_fs_create_dir(paths[at]));
    }
  }

  sp_ps_output_t out = sp_ps_run(mem, (sp_ps_config_t) {
    .command = exe,
    .args = { sp_str_lit("child"), sp_str_lit("--filter"), sp_str_lit("child.pass"), sp_str_lit("--dir"), sandbox },
    .env = {
      .extra = {
        { .key = sp_str_lit("SP_TEST_KEEP"), .value = sp_str_lit("never") },
      },
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
  });
  sp_test_kv(t, "stdout", out.out);
  sp_must_ok(t, out.error);
  sp_must_eq(t, out.status.exit_code, 0);

  sp_da(sp_str_t) survivors = sp_da_new(mem, sp_str_t);
  sp_carr_for(it->seeds, at) {
    if (!it->seeds[at].id) break;
    if (sp_fs_exists(paths[at])) sp_da_push(survivors, sp_cstr_as_str(it->seeds[at].id));
  }
  sp_expect_strs_eq(t, survivors, sp_da_size(survivors), it->expect);

  sp_da(sp_fs_entry_t) entries = SP_NULLPTR;
  sp_fs_collect(mem, runs, &entries);
  sp_expect_eq(t, sp_da_size(entries), sp_da_size(survivors));
  return SP_OK;
}
