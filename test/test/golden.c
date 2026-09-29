#include "sp/sp_test.h"

typedef struct {
  const c8* status;
  s32 exit_code;
  const c8* golden;
  const c8* actual;
} expect_t;

typedef struct {
  const c8* name;
  const c8* golden;
  const c8* cwd;
  const c8* root;
  const c8* env;
  bool update;
  bool absolute;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "match",
    .golden = "A",
    .root = ".",
    .expect = { .status = "ok", .golden = "A" },
  },
  {
    .name = "mismatch",
    .golden = "B",
    .root = ".",
    .expect = { .status = "failed", .exit_code = 1, .golden = "B", .actual = "A" },
  },
  {
    .name = "missing",
    .root = ".",
    .expect = { .status = "failed", .exit_code = 1 },
  },
  {
    .name = "update",
    .root = ".",
    .update = true,
    .expect = { .status = "updated", .golden = "A" },
  },
  {
    .name = "update_clears_actual",
    .golden = "B",
    .root = ".",
    .update = true,
    .expect = { .status = "updated", .golden = "A" },
  },
  {
    .name = "env_root",
    .golden = "A",
    .env = ".",
    .expect = { .status = "ok", .golden = "A" },
  },
  {
    .name = "flag_over_env",
    .golden = "A",
    .cwd = "D",
    .env = "nowhere",
    .root = "..",
    .expect = { .status = "ok", .golden = "A" },
  },
  {
    .name = "search_from_cwd",
    .golden = "A",
    .cwd = "D",
    .expect = { .status = "ok", .golden = "A" },
  },
  {
    .name = "search_walks_up",
    .golden = "A",
    .cwd = "D/E/F",
    .expect = { .status = "ok", .golden = "A" },
  },
  {
    .name = "absolute_file",
    .golden = "A",
    .cwd = "D",
    .absolute = true,
    .expect = { .status = "ok", .golden = "A" },
  },
};

sp_test_each(runner, golden, test_t, tests) {
  sp_test_skip_on_wasm();
  sp_test_skip_on_freestanding();

  if (it->absolute) sp_test_skip_without_absolute_names();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_str_t exe = sp_fs_get_exe_path(mem);

  sp_path_t src = sp_path_join(mem, sandbox, sp_str_lit("S/main.c"));
  sp_path_t golden = sp_path_join(mem, sandbox, sp_str_lit("S/G"));
  sp_path_t actual = sp_path_join(mem, sandbox, sp_str_lit("S/G.actual"));
  sp_must_ok(t, sp_fs_create_dir_at(sp_path_parent(mem, src)));
  sp_must_ok(t, sp_fs_create_file_at(src));
  sp_str_t file = it->absolute ? sp_fs_canonicalize_path_at(mem, src) : sp_str_lit("S/main.c");
  if (it->golden) sp_must_ok(t, sp_fs_create_file_cstr_at(golden, it->golden));
  if (it->update) sp_must_ok(t, sp_fs_create_file_cstr_at(actual, "stale"));

  sp_str_t cwd = it->cwd ? sp_fs_join_path(mem, sandbox.sub, sp_cstr_as_str(it->cwd)) : sandbox.sub;
  sp_must_ok(t, sp_fs_create_dir_at(sp_path_at(sandbox.dir, cwd)));

  sp_ps_config_t config = {
    .command = exe,
    .args = { sp_str_lit("child"), sp_str_lit("--filter"), sp_str_lit("child.golden"), sp_str_lit("--dir"), sp_str_lit("T") },
    .cwd = cwd,
    .env = {
      .extra = {
        { .key = sp_str_lit("SP_TEST_GOLDEN_ROOT"), .value = sp_cstr_as_str(it->env ? it->env : "") },
        { .key = sp_str_lit("SP_TEST_CHILD_FILE"), .value = file },
      },
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
  };
  if (it->root) {
    sp_ps_config_add_arg(mem, &config, sp_str_lit("--golden-root"));
    sp_ps_config_add_arg(mem, &config, sp_cstr_as_str(it->root));
  }
  if (it->update) sp_ps_config_add_arg(mem, &config, sp_str_lit("--update"));

  sp_ps_output_t out = sp_ps_run(mem, config);
  sp_test_kv(t, "stdout", out.out);
  sp_must_ok(t, out.error);
  sp_expect_eq(t, out.status.exit_code, it->expect.exit_code);
  sp_expect(t, sp_str_contains(out.out, sp_test_format(t, "\nchild.golden {} ", sp_fmt_cstr(it->expect.status))));

  sp_str_t content = sp_zero;
  sp_expect_eq(t, sp_fs_exists_at(golden), it->expect.golden != SP_NULLPTR);
  if (it->expect.golden) {
    sp_expect_ok(t, sp_io_read_file_at(mem, golden, &content));
    sp_expect_str_eq_c(t, content, it->expect.golden);
  }
  sp_expect_eq(t, sp_fs_exists_at(actual), it->expect.actual != SP_NULLPTR);
  if (it->expect.actual) {
    sp_expect_ok(t, sp_io_read_file_at(mem, actual, &content));
    sp_expect_str_eq_c(t, content, it->expect.actual);
  }
  return SP_OK;
}
