#include "sp/sp_test.h"

typedef struct {
  const c8* status;
  bool kept;
  s32 exit_code;
} keep_expect_t;

typedef struct {
  const c8* name;
  const c8* test;
  const c8* keep;
  const c8* env;
  keep_expect_t expect;
} keep_case_t;

static const keep_case_t keep_cases [] = {
  {
    .name = "default_pass",
    .test = "child.pass",
    .expect = { .status = "ok" },
  },
  {
    .name = "default_fail",
    .test = "child.fail",
    .expect = { .status = "failed", .kept = true, .exit_code = 1 },
  },
  {
    .name = "never_fail",
    .test = "child.fail",
    .keep = "never",
    .expect = { .status = "failed", .exit_code = 1 },
  },
  {
    .name = "failed_pass",
    .test = "child.pass",
    .keep = "failed",
    .expect = { .status = "ok" },
  },
  {
    .name = "failed_fail",
    .test = "child.fail",
    .keep = "failed",
    .expect = { .status = "failed", .kept = true, .exit_code = 1 },
  },
  {
    .name = "failed_leak",
    .test = "child.leak",
    .keep = "failed",
    .expect = { .status = "failed", .kept = true, .exit_code = 1 },
  },
  {
    .name = "failed_skip",
    .test = "child.skip",
    .keep = "failed",
    .expect = { .status = "skipped" },
  },
  {
    .name = "always_pass",
    .test = "child.pass",
    .keep = "always",
    .expect = { .status = "ok", .kept = true },
  },
  {
    .name = "always_skip",
    .test = "child.skip",
    .keep = "always",
    .expect = { .status = "skipped", .kept = true },
  },
  {
    .name = "declared_default",
    .test = "child.declared",
    .expect = { .status = "ok", .kept = true },
  },
  {
    .name = "declared_each",
    .test = "child.each.R",
    .expect = { .status = "ok", .kept = true },
  },
  {
    .name = "declared_failed",
    .test = "child.declared",
    .keep = "failed",
    .expect = { .status = "ok", .kept = true },
  },
  {
    .name = "declared_never",
    .test = "child.declared",
    .keep = "never",
    .expect = { .status = "ok" },
  },
  {
    .name = "declared_env_never",
    .test = "child.declared",
    .env = "never",
    .expect = { .status = "ok" },
  },
  {
    .name = "env",
    .test = "child.pass",
    .env = "always",
    .expect = { .status = "ok", .kept = true },
  },
  {
    .name = "env_overridden",
    .test = "child.pass",
    .env = "always",
    .keep = "never",
    .expect = { .status = "ok" },
  },
};

sp_test_each(runner, keep, keep_case_t, keep_cases) {
  sp_test_skip_on_wasm();
  sp_test_skip_on_freestanding();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_str_t exe = sp_zero;
  sp_must_ok(t, sp_fs_get_exe_path(mem, &exe));

  sp_ps_config_t config = {
    .command = exe,
    .args = { sp_str_lit("child"), sp_str_lit("--filter"), sp_cstr_as_str(it->test), sp_str_lit("--dir"), sp_str_lit("T") },
    .cwd = sandbox.sub,
    .env = {
      .extra = {
        { .key = sp_str_lit("SP_TEST_KEEP"), .value = sp_cstr_as_str(it->env ? it->env : "") },
      },
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
  };
  if (it->keep) {
    sp_ps_config_add_arg(mem, &config, sp_str_lit("--keep"));
    sp_ps_config_add_arg(mem, &config, sp_cstr_as_str(it->keep));
  }

  sp_ps_output_t out = sp_ps_run(mem, config);
  sp_test_kv(t, "stdout", out.out);
  sp_must_ok(t, out.error);
  sp_expect_eq(t, out.status.exit_code, it->expect.exit_code);
  sp_expect(t, sp_str_contains(out.out, sp_test_format(t, "\n{} {} ", sp_fmt_cstr(it->test), sp_fmt_cstr(it->expect.status))));

  sp_str_t runs = sp_fs_join_path(mem, sp_str_lit("T"), sp_fs_get_stem(exe));
  sp_da(sp_str_t) names = sp_da_new(mem, sp_str_t);
  sp_sys_dir_t runs_dir = sp_zero;
  if (!sp_fs_open_dir_at(sp_path_join(mem, sandbox, runs), &runs_dir)) {
    sp_fs_dir_it_t iter = sp_fs_dir_it(runs_dir);
    SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
    sp_fs_dir_entry_t entry = sp_zero;
    while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) sp_da_push(names, sp_str_copy(mem, entry.name));
    sp_fs_dir_it_close(&iter);
    sp_fs_close_dir(runs_dir);
  }

  if (!it->expect.kept) {
    sp_expect_eq(t, sp_da_size(names), (u64)0);
    sp_expect(t, !sp_str_contains(out.out, sp_str_lit("\n  dir ")));
    return SP_OK;
  }

  sp_must_eq(t, sp_da_size(names), (u64)1);
  sp_str_t dir = sp_fs_join_path(mem, sp_fs_join_path(mem, runs, names[0]), sp_cstr_as_str(it->test));
  sp_sys_file_meta_t meta = sp_zero;
  sp_expect_ok(t, sp_fs_get_link_metadata_at(sp_path_join(mem, sandbox, sp_fs_join_path(mem, dir, sp_str_lit("A"))), &meta));
  sp_expect_eq(t, meta.kind, SP_FS_KIND_FILE);
  sp_expect(t, sp_str_contains(out.out, sp_test_format(t, "\n  dir {}\n", sp_fmt_str(dir))));
  return SP_OK;
}
