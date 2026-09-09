#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* env;
  const c8* flag;
  const c8* expect;
} dir_case_t;

static const dir_case_t dir_cases [] = {
  {
    .name = "default",
    .expect = ".sp/test",
  },
  {
    .name = "env",
    .env = "E",
    .expect = "E",
  },
  {
    .name = "flag",
    .flag = "F",
    .expect = "F",
  },
  {
    .name = "flag_over_env",
    .env = "E",
    .flag = "F",
    .expect = "F",
  },
};

sp_test_each(runner, dir, dir_case_t, dir_cases) {
  sp_test_skip_on_wasm();
  sp_test_skip_on_freestanding();

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  sp_str_t exe = sp_fs_get_exe_path(mem);

  sp_ps_config_t config = {
    .command = exe,
    .args = { sp_str_lit("child"), sp_str_lit("--filter"), sp_str_lit("child.pass"), sp_str_lit("--keep"), sp_str_lit("always") },
    .cwd = sandbox,
    .env = {
      .extra = {
        { .key = sp_str_lit("SP_TEST_DIR"), .value = sp_cstr_as_str(it->env ? it->env : "") },
      },
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
  };
  if (it->flag) {
    sp_ps_config_add_arg(mem, &config, sp_str_lit("--dir"));
    sp_ps_config_add_arg(mem, &config, sp_fs_join_path(mem, sandbox, sp_cstr_as_str(it->flag)));
  }

  sp_ps_output_t out = sp_ps_run(mem, config);
  sp_test_kv(t, "stdout", out.out);
  sp_must_ok(t, out.error);
  sp_must_eq(t, out.status.exit_code, 0);

  sp_str_t root = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(it->expect));
  sp_da(sp_fs_entry_t) runs = SP_NULLPTR;
  sp_fs_collect(mem, sp_fs_join_path(mem, root, sp_fs_get_stem(exe)), &runs);
  sp_must_eq(t, sp_da_size(runs), (u64)1);

  sp_str_t dir = sp_fs_join_path(mem, runs[0].path, sp_str_lit("child.pass"));
  sp_expect(t, sp_fs_is_file(sp_fs_join_path(mem, dir, sp_str_lit("A"))));
  sp_expect(t, sp_str_contains(out.out, sp_test_format(t, "\n  dir {}\n", sp_fmt_str(dir))));
  return SP_OK;
}
