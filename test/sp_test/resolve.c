#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* file;
  const c8* anchor;
  const c8* expect [16];
} resolve_case_t;

static const resolve_case_t resolve_cases [] = {
  {
    .name = "absolute",
    .file = "/a/b/c/t.c",
    .anchor = "/x/y",
    .expect = {
      "/a/b/c/t.c",
      "/x/y/a/b/c/t.c", "/x/a/b/c/t.c",
      "/x/y/b/c/t.c",   "/x/b/c/t.c",
      "/x/y/c/t.c",     "/x/c/t.c",
    },
  },
  {
    .name = "relative",
    .file = "cli/usage.c",
    .anchor = "/x/build/debug",
    .expect = {
      "/x/build/debug/cli/usage.c", "/x/build/cli/usage.c", "/x/cli/usage.c",
      "cli/usage.c",
    },
  },
  {
    .name = "windows",
    .file = "C:\\repo\\src\\t.c",
    .anchor = "C:\\repo\\build",
    .expect = {
      "C:/repo/src/t.c",
      "C:/repo/build/repo/src/t.c", "C:/repo/repo/src/t.c", "C:/repo/src/t.c",
      "C:/repo/build/src/t.c",      "C:/repo/src/t.c",      "C:/src/t.c",
    },
  },
  {
    .name = "bare",
    .file = "t.c",
    .anchor = "/x",
    .expect = {
      "t.c",
    },
  },
  {
    .name = "no_anchor",
    .file = "a/t.c",
    .anchor = "",
    .expect = {
      "a/t.c",
    },
  },
};

sp_test_each(golden, resolve, resolve_case_t, resolve_cases) {
  sp_da(sp_str_t) candidates = sp_test_resolve_candidates(sp_test_arena(t),
    sp_cstr_as_str(it->file), sp_cstr_as_str(it->anchor));

  sp_must_strs_eq(t, candidates, sp_da_size(candidates), it->expect);
  return SP_OK;
}
