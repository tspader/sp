#include "fs.h"

typedef struct {
  const c8* path;
  const c8* ext;
  const c8* expected;
} replace_ext_case_t;

UTEST(fs_replace_ext, cases) {
  replace_ext_case_t cases[] = {
    { "foo.c",       "o",   "foo.c.o" },
    { "foo.c",       "",    "foo." },
    { "foo",         "txt", "foo.txt" },
    { "foo.bar.baz", "c",   "foo.bar.baz.c" },
    { ".profile",    "txt", ".profile.txt" },
    { "foo/bar.txt", "md",  "foo/bar.txt.md" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_replace_ext(sp_str_view(cases[i].path), sp_str_view(cases[i].ext));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
