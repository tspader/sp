#include "fs.h"

typedef struct {
  sp_err_t err;
  const c8* content;
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .expect = {
      .content = "A",
    },
  },
  {
    .name = "empty",
    .setup = {
      { "A" },
    },
  },
  {
    .name = "missing",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "directory",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .expect = {
      .err = SP_ERR_SYS_IS_DIR,
    },
  },
};

sp_test_each(fs, read, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_sys_dir_t dir = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_str_t content = sp_zero;
  sp_err_t err = sp_fs_read_at(mem, sp_path(dir, sp_str_lit("A")), &content);
  sp_fs_close_dir(dir);

  sp_expect_err_eq(t, err, it->expect.err);
  sp_expect_str_eq(t, content, it->expect.content ? sp_cstr_as_str(it->expect.content) : sp_str_lit(""));
  return SP_OK;
}

#if defined(SP_LINUX)
sp_test(fs, read_procfs) {
  sp_str_t content = sp_zero;
  sp_expect_ok(t, sp_fs_read(sp_test_arena(t), sp_str_lit("/proc/self/stat"), &content));
  sp_expect(t, !sp_str_empty(content));
  return SP_OK;
}
#endif
