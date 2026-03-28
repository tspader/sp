#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} trim_path_case_t;

UTEST(fs_trim_path, cases) {
  trim_path_case_t cases[] = {
    { "",                  "" },
    { "foo",               "foo" },
    { "foo/",              "foo" },
    { "foo/bar",           "foo/bar" },
    { ".profile",          ".profile" },
    { "C:/foo/bar.txt",    "C:/foo/bar.txt" },
    { "C:\\foo\\bar.txt",  "C:\\foo\\bar.txt" },
    { "/",                 "/" },
    { "C:/",               "C:/" },
    { "C:\\",              "C:\\" },
    { "/foo/",             "/foo" },
    { "foo///",            "foo" },
    { "foo\\\\",           "foo" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_trim_path(sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
