#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                .input = "",             .expect = "" },
  { .name = "bare",                 .input = "A",            .expect = "A" },
  { .name = "trailing_slash",       .input = "A/",           .expect = "A" },
  { .name = "nested",               .input = "A/B",          .expect = "A/B" },
  { .name = "hidden",               .input = ".A",           .expect = ".A" },
  { .name = "drive_forward",        .input = "C:/A/B.txt",   .expect = "C:/A/B.txt" },
  { .name = "drive_backslash",      .input = "C:\\A\\B.txt", .expect = "C:\\A\\B.txt" },
  { .name = "root_slash",           .input = "/",            .expect = "/" },
  { .name = "drive_slash",          .input = "C:/",          .expect = "C:/" },
  { .name = "drive_backslash_root", .input = "C:\\",         .expect = "C:\\" },
  { .name = "absolute_trailing",    .input = "/A/",          .expect = "/A" },
  { .name = "multi_slash",          .input = "A///",         .expect = "A" },
  { .name = "multi_backslash",      .input = "A\\\\",        .expect = "A" },
};

sp_test_each(fs, trim_path, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_trim_path(sp_str_view(it->input)), it->expect);
  return SP_OK;
}
