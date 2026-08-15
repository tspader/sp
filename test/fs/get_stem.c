#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                   .input = "",             .expect = "" },
  { .name = "bare",                    .input = "A",            .expect = "A" },
  { .name = "trailing_slash",          .input = "A/",           .expect = "A/" },
  { .name = "after_dir",               .input = "A/B",          .expect = "B" },
  { .name = "strips_ext",              .input = "A/B.txt",      .expect = "B" },
  { .name = "double_dot",              .input = "A..txt",       .expect = "A." },
  { .name = "leading_dot",             .input = ".A",           .expect = "" },
  { .name = "leading_dot_with_ext",    .input = ".A.txt",       .expect = ".A" },
  { .name = "drive_forward",           .input = "C:/A/B.txt",   .expect = "B" },
  // backslash: get_name returns the whole thing, then the extension is stripped
  { .name = "backslash_not_separator", .input = "C:\\A\\B.txt", .expect = "C:\\A\\B" },
  { .name = "simple",                  .input = "A.B",          .expect = "A" },
  { .name = "trailing_dot",            .input = "A.",           .expect = "A" },
  { .name = "multiple_dots",           .input = "A.B.C",        .expect = "A.B" },
  { .name = "multiple_dots_trailing",  .input = "A.B.",         .expect = "A.B" },
  { .name = "hidden_single",           .input = ".B",           .expect = "" },
};

sp_test_each(fs, get_stem, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_stem(sp_str_view(it->input)), it->expect);
  return SP_OK;
}
