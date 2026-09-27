#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                 .input = "",      .expect = "" },
  { .name = "bare",                  .input = "A",     .expect = "A" },
  { .name = "trailing_slash",        .input = "A/",    .expect = "A" },
  { .name = "trailing_slash_run",    .input = "A///",  .expect = "A" },
  { .name = "nested",                .input = "A/B",   .expect = "A/B" },
  { .name = "absolute_trailing",     .input = "/A/",   .expect = "/A" },
  { .name = "root",                  .input = "/",     .expect = "/" },
  { .name = "root_run",              .input = "//",    .expect = "/" },
  { .name = "backslash_is_a_name",   .input = "A\\",   .expect = "A\\" },
  { .name = "drive_is_a_name",       .input = "C:/",   .expect = "C:" },

  { .name = "windows_slash",         .kind = SP_FS_PATH_WINDOWS, .input = "A/",     .expect = "A" },
  { .name = "windows_backslash",     .kind = SP_FS_PATH_WINDOWS, .input = "A\\",    .expect = "A" },
  { .name = "windows_mixed_run",     .kind = SP_FS_PATH_WINDOWS, .input = "A/\\/",  .expect = "A" },
  { .name = "windows_root",          .kind = SP_FS_PATH_WINDOWS, .input = "\\",     .expect = "\\" },
  { .name = "windows_drive",         .kind = SP_FS_PATH_WINDOWS, .input = "C:/",    .expect = "C:/" },
  { .name = "windows_drive_back",    .kind = SP_FS_PATH_WINDOWS, .input = "C:\\",   .expect = "C:\\" },
  { .name = "windows_drive_run",     .kind = SP_FS_PATH_WINDOWS, .input = "C:\\\\", .expect = "C:\\" },
  { .name = "windows_drive_nested",  .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\", .expect = "C:\\A" },
};

sp_test_each(fs, trim_path, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_trim_path_for(sp_cstr_as_str(it->input), it->kind), it->expect);
  return SP_OK;
}
