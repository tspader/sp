#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                   .input = "",             .expect = "" },
  { .name = "bare",                    .input = "A",            .expect = "" },
  { .name = "trailing_slash",          .input = "A/",           .expect = "" },
  { .name = "nested",                  .input = "A/B",          .expect = "A" },
  { .name = "nested_ext",              .input = "A/B.txt",      .expect = "A" },
  // backslash: no '/' found so the parent is empty
  { .name = "backslash_not_separator", .input = "C:\\A\\B.txt", .expect = "" },
  { .name = "drive_nested",            .input = "C:/A/B/C.txt", .expect = "C:/A/B" },
  { .name = "drive_trailing_slash",    .input = "C:/A/B/",      .expect = "C:/A" },
  { .name = "drive_multi_trailing",    .input = "C:/A/B///",    .expect = "C:/A" },
  { .name = "drive_root",              .input = "C:/",          .expect = "C:/" },
  { .name = "root",                    .input = "/",            .expect = "/" },
  { .name = "absolute_single",         .input = "/A",           .expect = "" },
  { .name = "absolute_nested",         .input = "/A/B/C",       .expect = "/A/B" },
};

sp_test_each(fs, parent_path, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_parent_path(sp_str_view(it->input)), it->expect);
  return SP_OK;
}
