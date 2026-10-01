#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                .input = "",        .expect = "" },
  { .name = "bare",                 .input = "A",       .expect = "A" },
  { .name = "trailing_slash",       .input = "A/",      .expect = "A" },
  { .name = "nested",               .input = "A/B.txt", .expect = "B.txt" },
  { .name = "nested_trailing",      .input = "A/B/",    .expect = "B" },
  { .name = "absolute",             .input = "/A",      .expect = "A" },
  { .name = "root",                 .input = "/",       .expect = "" },
  { .name = "dot",                  .input = ".",       .expect = "." },
  { .name = "dotdot",               .input = "A/..",    .expect = ".." },
  { .name = "backslash_is_a_name",  .input = "A\\B",    .expect = "A\\B" },
  { .name = "drive_is_a_name",      .input = "C:/",     .expect = "C:" },

  { .name = "windows_backslash",    .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\B.txt", .expect = "B.txt" },
  { .name = "windows_mixed",        .kind = SP_FS_PATH_WINDOWS, .input = "A\\B/C",       .expect = "C" },
  { .name = "windows_trailing",     .kind = SP_FS_PATH_WINDOWS, .input = "A\\B\\",       .expect = "B" },
  { .name = "windows_dotdot",       .kind = SP_FS_PATH_WINDOWS, .input = "A\\..",        .expect = ".." },
  { .name = "windows_root",         .kind = SP_FS_PATH_WINDOWS, .input = "\\",           .expect = "" },
  { .name = "windows_drive",        .kind = SP_FS_PATH_WINDOWS, .input = "C:\\",         .expect = "" },
  { .name = "windows_under_drive",  .kind = SP_FS_PATH_WINDOWS, .input = "C:/A",         .expect = "A" },
};

sp_test_each(fs, get_name, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_name_for(sp_cstr_as_str(it->input), it->kind), it->expect);
  return SP_OK;
}
