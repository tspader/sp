#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                  .input = "",          .expect = "" },
  { .name = "none",                   .input = "A",         .expect = "" },
  { .name = "simple",                 .input = "A.B",       .expect = "B" },
  { .name = "nested",                 .input = "A/B.txt",   .expect = "txt" },
  { .name = "trailing_slash",         .input = "A/B.txt/",  .expect = "txt" },
  { .name = "last_dot_wins",          .input = "A.B.C",     .expect = "C" },
  { .name = "double_dot",             .input = "A..txt",    .expect = "txt" },
  { .name = "trailing_dot",           .input = "A.",        .expect = "" },
  { .name = "dot_in_dir",             .input = "/A/B.C/D",  .expect = "" },
  { .name = "hidden",                 .input = ".A",        .expect = "" },
  { .name = "hidden_with_ext",        .input = ".A.txt",    .expect = "txt" },
  { .name = "dot",                    .input = ".",         .expect = "" },
  { .name = "dotdot",                 .input = "..",        .expect = "" },
  { .name = "root",                   .input = "/",         .expect = "" },
  { .name = "backslash_is_a_name",    .input = "A.B\\C",    .expect = "B\\C" },

  { .name = "windows_backslash",      .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\B.txt", .expect = "txt" },
  { .name = "windows_dot_in_dir",     .kind = SP_FS_PATH_WINDOWS, .input = "A.B\\C",       .expect = "" },
};

sp_test_each(fs, get_ext, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_ext_for(sp_cstr_as_str(it->input), it->kind), it->expect);
  return SP_OK;
}
