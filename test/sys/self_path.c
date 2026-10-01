#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  PATH_CWD,
  PATH_EXE,
} path_fn_t;

typedef struct {
  const c8* name;
  path_fn_t fn;
} test_t;

static const test_t tests [] = {
  { .name = "cwd_has_no_trailing_separator", .fn = PATH_CWD },
  { .name = "exe_has_no_trailing_separator", .fn = PATH_EXE },
};

static sp_err_t run(sp_test_t* t, test_t* c) {
  c8 buf [SP_PATH_MAX];
  sp_for(it, sizeof(buf)) buf[it] = (c8)0xAB;

  u64 n = 0;
  switch (c->fn) {
    case PATH_CWD: {
      sp_err_t err = sp_sys_get_fd_path(sp_fs_get_cwd(), buf, sizeof(buf), &n);
      if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "not available");
      sp_must_ok(t, err);
      break;
    }
    case PATH_EXE: {
      sp_err_t err = sp_sys_get_exe_path(buf, sizeof(buf), &n);
      if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "not available");
      sp_must_ok(t, err);
      break;
    }
  }

  if (buf[n] != 0) {
    sp_test_fail(t, "not NUL-terminated");
  }
  // A root keeps its separator: "/" is excluded by n > 1, "C:\" by the drive
  // check. Stripping "C:\" would leave "C:", which is drive-relative.
  else if (n > 1 && (buf[n - 1] == '/' || buf[n - 1] == '\\') && !(n == 3 && buf[1] == ':')) {
    sp_test_fail(t, "{.quote} ends in a separator", sp_fmt_cstr(buf));
  }
  return SP_OK;
}

sp_test_each_fn(sys, self_path, test_t, tests, run);
