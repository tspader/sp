#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} get_name_case_t;

UTEST(fs_get_name, cases) {
  get_name_case_t cases[] = {
    { "",                        "" },
    { "foo",                     "foo" },
    { "foo/",                    "" },
    { "foo/bar.txt",             "bar.txt" },
    // Backslash: get_name only splits on '/', whole thing is the "name"
    { "C:\\foo\\bar.txt",        "C:\\foo\\bar.txt" },
    { "C:/Users/Test/file.txt",  "file.txt" },
    { "/foo",                    "foo" },
    { "/foo/bar/",               "" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_get_name(sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
