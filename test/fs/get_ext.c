#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                  .input = "",             .expect = "" },
  { .name = "no_ext",                 .input = "A",            .expect = "" },
  { .name = "no_ext_with_dir",        .input = "A/B",          .expect = "" },
  { .name = "ext_after_dir",          .input = "A/B.txt",      .expect = "txt" },
  { .name = "double_dot",             .input = "A..txt",       .expect = "txt" },
  { .name = "leading_dot",            .input = ".A",           .expect = "A" },
  { .name = "simple",                 .input = "A.B",          .expect = "B" },
  { .name = "trailing_dot",           .input = "A.",           .expect = "" },
  { .name = "multiple_dots",          .input = "A.B.C",        .expect = "C" },
  { .name = "multiple_dots_trailing", .input = "A.B.",         .expect = "" },
  { .name = "windows_path",           .input = "C:\\A\\B.txt", .expect = "txt" },
  { .name = "dot_in_dir",             .input = "/A/B.C/D",     .expect = "" },
};

sp_test_each(fs, get_ext, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_ext(sp_str_view(it->input)), it->expect);
  return SP_OK;
}
