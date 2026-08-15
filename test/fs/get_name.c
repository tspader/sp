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
  { .name = "trailing_slash",          .input = "A/",           .expect = "" },
  { .name = "after_dir",               .input = "A/B.txt",      .expect = "B.txt" },
  // backslash: get_name only splits on '/', the whole thing is the name
  { .name = "backslash_not_separator", .input = "C:\\A\\B.txt", .expect = "C:\\A\\B.txt" },
  { .name = "drive_forward",           .input = "C:/A/B/C.txt", .expect = "C.txt" },
  { .name = "absolute",                .input = "/A",           .expect = "A" },
  { .name = "absolute_trailing_slash", .input = "/A/B/",        .expect = "" },
};

sp_test_each(fs, get_name, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_name(sp_str_view(it->input)), it->expect);
  return SP_OK;
}
