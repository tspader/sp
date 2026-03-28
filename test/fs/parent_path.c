#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} parent_path_case_t;

UTEST(fs_parent_path, cases) {
  parent_path_case_t cases[] = {
    { "",                       "" },
    { "foo",                    "" },
    { "foo/",                   "" },
    { "foo/bar",                "foo" },
    { "foo/bar.txt",            "foo" },
    // Backslash: no '/' found so empty
    { "C:\\foo\\bar.txt",       "" },
    { "C:/Users/Test/file.txt", "C:/Users/Test" },
    { "C:/Users/Test/",         "C:/Users" },
    { "C:/Users/Test///",       "C:/Users" },
    { "C:/",                    "C:/" },
    { "/",                      "/" },
    { "/foo",                   "" },
    { "/home/user/file",        "/home/user" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_parent_path(sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
