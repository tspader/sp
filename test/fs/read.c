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
};

sp_test_each(fs, read, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_str_t content = sp_zero;
  sp_expect_err_eq(t, sp_fs_read_at(mem, sp_path_join(mem, sandbox, sp_str_lit("A")), &content), it->expect.err);
  sp_expect_str_eq(t, content, it->expect.content ? sp_cstr_as_str(it->expect.content) : sp_str_lit(""));
  return SP_OK;
}
