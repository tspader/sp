#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* cwd;
  const c8* exe_dir;
  const c8* expect [16];
} resolve_case_t;

static const resolve_case_t resolve_cases [] = {
  {
    .name = "disjoint",
    .cwd = "/a/b",
    .exe_dir = "/x/y",
    .expect = { "/a/b", "/a", "/x/y", "/x" },
  },
  {
    .name = "overlap",
    .cwd = "/x/repo",
    .exe_dir = "/x/repo/build/debug",
    .expect = { "/x/repo", "/x", "/x/repo/build/debug", "/x/repo/build" },
  },
  {
    .name = "windows",
    .cwd = "C:\\repo",
    .exe_dir = "C:\\repo\\build",
    .expect = { "C:/repo", "C:", "C:/repo/build" },
  },
  {
    .name = "trailing",
    .cwd = "/a/b/",
    .exe_dir = "",
    .expect = { "/a/b", "/a" },
  },
  {
    .name = "empty",
    .cwd = "",
    .exe_dir = "/x",
    .expect = { "/x" },
  },
};

sp_test_each(golden, resolve, resolve_case_t, resolve_cases) {
  sp_da(sp_str_t) roots = sp_test_resolve_roots(sp_test_arena(t),
    sp_cstr_as_str(it->cwd), sp_cstr_as_str(it->exe_dir));

  sp_must_strs_eq(t, roots, sp_da_size(roots), it->expect);
  return SP_OK;
}
