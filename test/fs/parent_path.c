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
  { .name = "bare",                 .input = "A",       .expect = "." },
  { .name = "bare_trailing",        .input = "A/",      .expect = "." },
  { .name = "dot",                  .input = ".",       .expect = "." },
  { .name = "nested",               .input = "A/B",     .expect = "A" },
  { .name = "nested_trailing",      .input = "A/B/",    .expect = "A" },
  { .name = "separator_run",        .input = "A//B",    .expect = "A" },
  { .name = "root",                 .input = "/",       .expect = "/" },
  { .name = "under_root",           .input = "/A",      .expect = "/" },
  { .name = "absolute_nested",      .input = "/A/B/C",  .expect = "/A/B" },
  { .name = "backslash_is_a_name",  .input = "A\\B",    .expect = "." },
  { .name = "drive_is_a_name",      .input = "C:/A",    .expect = "C:" },

  { .name = "windows_backslash",    .kind = SP_FS_PATH_WINDOWS, .input = "A\\B",          .expect = "A" },
  { .name = "windows_mixed",        .kind = SP_FS_PATH_WINDOWS, .input = "A/B\\C",        .expect = "A/B" },
  { .name = "windows_root",         .kind = SP_FS_PATH_WINDOWS, .input = "\\",            .expect = "\\" },
  { .name = "windows_under_root",   .kind = SP_FS_PATH_WINDOWS, .input = "\\A",           .expect = "\\" },
  { .name = "windows_drive",        .kind = SP_FS_PATH_WINDOWS, .input = "C:/",           .expect = "C:/" },
  { .name = "windows_under_drive",  .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A",         .expect = "C:\\" },
  { .name = "windows_drive_nested", .kind = SP_FS_PATH_WINDOWS, .input = "C:/A/B///",     .expect = "C:/A" },
};

sp_test_each(fs, parent_path, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_parent_path_for(sp_cstr_as_str(it->input), it->kind), it->expect);
  return SP_OK;
}
