#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "windows_backslash",  .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\B.txt", .expect = "B.txt" },
  { .name = "windows_mixed",      .kind = SP_FS_PATH_WINDOWS, .input = "A\\B/C",       .expect = "C" },
  { .name = "windows_dotdot",     .kind = SP_FS_PATH_WINDOWS, .input = "A\\..",        .expect = ".." },
  { .name = "windows_drive_root", .kind = SP_FS_PATH_WINDOWS, .input = "C:\\",         .expect = "" },
  { .name = "posix_backslash",    .kind = SP_FS_PATH_POSIX,   .input = "A\\B",         .expect = "A\\B" },
  { .name = "posix_slash",        .kind = SP_FS_PATH_POSIX,   .input = "A/B",          .expect = "B" },
};

sp_test_each(fs, get_name_for, test_t, tests) {
  sp_expect_str_eq_c(t, sp_fs_get_name_for(sp_cstr_as_str(it->input), it->kind), it->expect);
  return SP_OK;
}
