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
  { .name = "none",                   .input = "A",         .expect = "A" },
  { .name = "simple",                 .input = "A.B",       .expect = "A" },
  { .name = "nested",                 .input = "A/B.txt",   .expect = "B" },
  { .name = "trailing_slash",         .input = "A/B.txt/",  .expect = "B" },
  { .name = "last_dot_wins",          .input = "A.B.C",     .expect = "A.B" },
  { .name = "double_dot",             .input = "A..txt",    .expect = "A." },
  { .name = "trailing_dot",           .input = "A.",        .expect = "A" },
  { .name = "hidden",                 .input = ".A",        .expect = ".A" },
  { .name = "hidden_with_ext",        .input = ".A.txt",    .expect = ".A" },
  { .name = "dot",                    .input = ".",         .expect = "." },
  { .name = "dotdot",                 .input = "..",        .expect = ".." },
  { .name = "root",                   .input = "/",         .expect = "" },
  { .name = "backslash_is_a_name",    .input = "A\\B.txt",  .expect = "A\\B" },

  { .name = "windows_backslash",      .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\B.txt", .expect = "B" },
  { .name = "windows_dot_in_dir",     .kind = SP_FS_PATH_WINDOWS, .input = "A.B\\C",       .expect = "C" },
};

sp_test_each(fs, get_stem, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_stem_for(sp_cstr_as_str(it->input), it->kind), it->expect);
  return SP_OK;
}
