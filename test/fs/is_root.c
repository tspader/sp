#include "fs.h"

typedef struct {
  const c8* input;
  bool expected;
} is_root_case_t;

UTEST(fs_is_root, cases) {
  is_root_case_t cases[] = {
    { "",      true },
    { "/",     true },
    { "C:",    true },
    { "a:",    true },
    { "C:/",   true },
    { "C:\\",  true },
    { "foo",   false },
    { "/foo",  false },
    { "C:/foo",false },
    { "//",    false },
  };

  SP_CARR_FOR(cases, i) {
    bool result = sp_fs_is_root(sp_str_view(cases[i].input));
    EXPECT_EQ(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
