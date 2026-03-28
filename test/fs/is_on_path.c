#include "fs.h"

typedef struct {
  const c8* program;
  bool expected;
} is_on_path_case_t;

UTEST(fs_is_on_path, cases) {
  is_on_path_case_t cases[] = {
#if defined(SP_WIN32)
    { "cmd.exe",                          true },
#else
    { "sh",                               true },
#endif
    { "sp_nonexistent_binary_12345",      false },
    { "",                                 false },
  };

  SP_CARR_FOR(cases, i) {
    bool result = sp_fs_is_on_path(sp_str_view(cases[i].program));
    EXPECT_EQ(result, cases[i].expected);
  }
}

SP_TEST_MAIN()
