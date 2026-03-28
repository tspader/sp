#include "fs.h"

typedef struct {
  const c8* input;
  bool expected;
} is_glob_case_t;

UTEST(fs_is_glob, cases) {
  is_glob_case_t cases[] = {
    { "",          false },
    { "foo",       false },
    { "foo.txt",   false },
    { "foo?bar",   false },
    { "*",         true },
    { "*.txt",     true },
    { "foo/*",     true },
    { "foo*bar",   true },
  };

  SP_CARR_FOR(cases, i) {
    bool result = sp_fs_is_glob(sp_str_view(cases[i].input));
    EXPECT_EQ(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
