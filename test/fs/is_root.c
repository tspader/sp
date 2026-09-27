#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  bool posix;
  bool windows;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                 .input = "" },
  { .name = "slash",                 .input = "/",     .posix = true, .windows = true },
  { .name = "slash_run",             .input = "//",    .posix = true, .windows = true },
  { .name = "backslash",             .input = "\\",                   .windows = true },
  { .name = "drive_bare",            .input = "C:" },
  { .name = "drive_lower_bare",      .input = "a:" },
  { .name = "drive_slash",           .input = "C:/",                  .windows = true },
  { .name = "drive_slash_run",       .input = "C://",                 .windows = true },
  { .name = "drive_backslash",       .input = "C:\\",                 .windows = true },
  { .name = "relative",              .input = "A" },
  { .name = "dot",                   .input = "." },
  { .name = "slash_prefix",          .input = "/A" },
  { .name = "slash_prefix_trailing", .input = "/A/" },
  { .name = "drive_prefix",          .input = "C:/A" },
};

sp_test_each(fs, is_root, test_t, tests) {
  sp_str_t path = sp_cstr_as_str(it->input);
  sp_expect_eq(t, sp_fs_is_root_for(path, SP_FS_PATH_POSIX), it->posix);
  sp_expect_eq(t, sp_fs_is_root_for(path, SP_FS_PATH_WINDOWS), it->windows);
  return SP_OK;
}
