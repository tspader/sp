#include "fs.h"

typedef struct {
  const c8* a;
  const c8* b;
  const c8* expected;
} join_path_case_t;

UTEST(fs_join_path, cases) {
  join_path_case_t cases[] = {
    { "foo",  "bar",     "foo/bar" },
    { "foo/", "bar",     "foo/bar" },
    { "foo/", "bar/",    "foo/bar" },
    { "foo",  "bar/baz", "foo/bar/baz" },
    { "/",    "bar",     "//bar" },
    { "",     "bar",     "bar" },
    { "foo",  "",        "foo" },
    { "",     "",        "" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_join_path(sp_str_view(cases[i].a), sp_str_view(cases[i].b));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
