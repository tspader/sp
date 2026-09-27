#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  c8 input;
  bool posix;
  bool windows;
} test_t;

static const test_t tests [] = {
  { .name = "slash",     .input = '/',  .posix = true, .windows = true },
  { .name = "backslash", .input = '\\',                .windows = true },
  { .name = "letter",    .input = 'A' },
  { .name = "colon",     .input = ':' },
  { .name = "dot",       .input = '.' },
};

sp_test_each(fs, is_sep, test_t, tests) {
  sp_expect_eq(t, sp_fs_is_sep_for(it->input, SP_FS_PATH_POSIX), it->posix);
  sp_expect_eq(t, sp_fs_is_sep_for(it->input, SP_FS_PATH_WINDOWS), it->windows);
  return SP_OK;
}
