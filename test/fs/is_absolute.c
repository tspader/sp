#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  bool posix;
  bool windows;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                  .input = "" },
  { .name = "root_slash",             .input = "/",       .posix = true, .windows = true },
  { .name = "root_backslash",         .input = "\\",      .posix = true, .windows = true },
  { .name = "double_slash",           .input = "//",      .posix = true, .windows = true },
  { .name = "slash_prefix",           .input = "/A",      .posix = true, .windows = true },
  { .name = "backslash_prefix",       .input = "\\A",     .posix = true, .windows = true },
  { .name = "relative",               .input = "A" },
  { .name = "relative_nested",        .input = "A/B" },
  { .name = "drive_bare",             .input = "C:" },
  { .name = "drive_lower_bare",       .input = "a:" },
  { .name = "drive_relative",         .input = "C:A" },
  { .name = "drive_slash",            .input = "C:/",                    .windows = true },
  { .name = "drive_backslash",        .input = "C:\\",                   .windows = true },
  { .name = "drive_slash_prefix",     .input = "C:/A",                   .windows = true },
  { .name = "drive_backslash_prefix", .input = "C:\\A",                  .windows = true },
};

sp_test_each(fs, is_absolute, test_t, tests) {
  sp_str_t path = sp_str_view(it->input);
  sp_expect_eq(t, sp_fs_is_absolute_for(path, SP_FS_PATH_POSIX), it->posix);
  sp_expect_eq(t, sp_fs_is_absolute_for(path, SP_FS_PATH_WINDOWS), it->windows);
  return SP_OK;
}
