#include "fs.h"

typedef struct {
  const c8* input;
  bool posix;
  bool windows;
} is_absolute_case_t;

UTEST(fs_is_absolute, cases) {
  SKIP_ON_WASM()
  is_absolute_case_t cases[] = {
    { "",        false, false },
    { "/",       true,  true  },
    { "\\",      true,  true  },
    { "//",      true,  true  },
    { "/foo",    true,  true  },
    { "\\foo",   true,  true  },
    { "foo",     false, false },
    { "foo/bar", false, false },
    { "C:",      false, false },
    { "a:",      false, false },
    { "C:foo",   false, false },
    { "C:/",     false, true  },
    { "C:\\",    false, true  },
    { "C:/foo",  false, true  },
    { "C:\\foo", false, true  },
  };

  SP_CARR_FOR(cases, i) {
    sp_str_t path = sp_str_view(cases[i].input);
    EXPECT_EQ(sp_fs_is_absolute_for(path, SP_FS_PATH_POSIX),   cases[i].posix);
    EXPECT_EQ(sp_fs_is_absolute_for(path, SP_FS_PATH_WINDOWS), cases[i].windows);
  }
}
