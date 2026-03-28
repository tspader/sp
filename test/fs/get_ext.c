#include "fs.h"

typedef struct {
  const c8* input;
  const c8* expected;
} get_ext_case_t;

UTEST(fs_get_ext, cases) {
  get_ext_case_t cases[] = {
    { "",                    "" },
    { "foo",                 "" },
    { "foo/bar",             "" },
    { "foo/bar.txt",         "txt" },
    { "foo..txt",            "txt" },
    { ".foo",                "foo" },
    { "foo.bar",             "bar" },
    { "foo.",                "" },
    { "foo.bar.baz",         "baz" },
    { "foo.bar.",            "" },
    { "C:\\foo\\bar.txt",    "txt" },
    { "/foo/bar.baz/qux",   "" },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t result = sp_fs_get_ext(sp_str_view(cases[i].input));
    SP_EXPECT_STR_EQ_CSTR(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
