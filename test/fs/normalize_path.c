#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                    .input = "",                       .expect = "" },
  { .name = "bare",                     .input = "A",                      .expect = "A" },
  { .name = "trailing_slash",           .input = "A/",                     .expect = "A" },
  { .name = "nested",                   .input = "A/B",                    .expect = "A/B" },
  { .name = "nested_ext",               .input = "A/B.txt",                .expect = "A/B.txt" },
  { .name = "hidden",                   .input = ".A",                     .expect = ".A" },
  { .name = "drive_forward",            .input = "C:/A/B.txt",             .expect = "C:/A/B.txt" },
  { .name = "drive_backslash",          .input = "C:\\A\\B.txt",           .expect = "C:/A/B.txt" },
  { .name = "mixed_separators",         .input = "C:/A\\B/C\\D.txt",       .expect = "C:/A/B/C/D.txt" },
  { .name = "drive_backslash_trailing", .input = "C:\\A\\B\\",             .expect = "C:/A/B" },
  { .name = "backslash",                .input = "A\\B",                   .expect = "A/B" },
  { .name = "root_backslash",           .input = "\\",                     .expect = "" },
  { .name = "root_slash",               .input = "/",                      .expect = "" },
  { .name = "backslash_trailing",       .input = "A\\",                    .expect = "A" },
  { .name = "preserves_dotdot",         .input = "A\\B\\..\\C",            .expect = "A/B/../C" },
  { .name = "preserves_dot",            .input = "A\\.\\B",                .expect = "A/./B" },
  { .name = "nonexistent_path",         .input = "C:\\no\\such\\path\\file.txt", .expect = "C:/no/such/path/file.txt" },
};

sp_test_each(fs, normalize_path, test_t, tests) {
  sp_str_t result = sp_fs_normalize_path(sp_test_arena(t), sp_str_view(it->input));
  sp_expect_str_eq_c(t, result, it->expect);
  return SP_OK;
}
