#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  SETUP_NONE,
  SETUP_FILE,
  SETUP_LINK,
  SETUP_LINK_ABSOLUTE,
} setup_t;

typedef struct {
  sp_err_t err;
  const c8* target;
} expect_t;

typedef struct {
  const c8* name;
  setup_t setup;
  const c8* target;
  u64 size;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "relative", .setup = SETUP_LINK, .target = "A", .expect = { .target = "A" } },
  { .name = "nested", .setup = SETUP_LINK, .target = "B/C", .expect = { .target = "B/C" } },
  { .name = "unicode", .setup = SETUP_LINK, .target = "\xc3\xa9", .expect = { .target = "\xc3\xa9" } },
  { .name = "absolute", .setup = SETUP_LINK_ABSOLUTE, .target = "A" },
  { .name = "too_small", .setup = SETUP_LINK, .target = "A", .size = 1, .expect = { .err = SP_ERR_SYS_NAME_TOO_LONG } },
  { .name = "missing", .expect = { .err = SP_ERR_SYS_NOT_FOUND } },
  { .name = "not_a_link", .setup = SETUP_FILE, .expect = { .err = SP_ERR_SYS_INVALID } },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  sp_str_t path = sp_fs_join_path(mem, sandbox, sp_str_lit("L"));
  sp_str_t expected = c->expect.target ? sp_cstr_as_str(c->expect.target) : sp_zero_s(sp_str_t);

  switch (c->setup) {
    case SETUP_NONE: {
      break;
    }
    case SETUP_FILE: {
      sp_must_ok(t, sp_fs_create_file(path));
      break;
    }
    case SETUP_LINK: {
      if (sp_fs_create_sym_link(sp_cstr_as_str(c->target), path)) {
        return sp_test_skip(t, "symlinks not available");
      }
      break;
    }
    case SETUP_LINK_ABSOLUTE: {
      expected = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(c->target));
      if (sp_fs_create_sym_link(expected, path)) {
        return sp_test_skip(t, "symlinks not available");
      }
      break;
    }
  }

  // sys returns the target as the OS stores it; on Windows the writer stores
  // backslashes, and readlink does not translate them back.
#if defined(SP_WIN32)
  expected = sp_str_replace_c8(mem, expected, '/', '\\');
#endif

  c8 buf [SP_PATH_MAX];
  sp_str_t target = sp_zero;
  sp_err_t err = sp_sys_readlink_s(sp_sys_get_root(0), path, buf, c->size ? c->size : sizeof(buf), &target);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "readlink not supported");

  sp_expect_err_eq(t, err, c->expect.err);
  sp_expect_str_eq(t, target, expected);
  return SP_OK;
}

sp_test_each_fn(sys, readlink, test_t, tests, run);
