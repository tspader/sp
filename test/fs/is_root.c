#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  bool root;
} test_t;

static const test_t tests [] = {
  { .name = "empty",            .input = "" },
  { .name = "slash",            .input = "/",     .root = true },
  { .name = "backslash",        .input = "\\",    .root = true },
  { .name = "drive_bare",       .input = "C:" },
  { .name = "drive_lower_bare", .input = "a:" },
  { .name = "drive_slash",      .input = "C:/",   .root = true },
  { .name = "drive_backslash",  .input = "C:\\",  .root = true },
  { .name = "relative",         .input = "A" },
  { .name = "slash_prefix",     .input = "/A" },
  { .name = "drive_prefix",     .input = "C:/A" },
  { .name = "double_slash",     .input = "//" },
};

sp_test_each(fs, is_root, test_t, tests) {
  sp_expect_eq(t, sp_fs_is_root(sp_str_view(it->input)), it->root);
  return SP_OK;
}
