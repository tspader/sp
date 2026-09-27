#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",                .input = "",          .expect = "" },
  { .name = "bare",                 .input = "A",         .expect = "A" },
  { .name = "nested",               .input = "A/B",       .expect = "A/B" },
  { .name = "trailing_slash",       .input = "A/",        .expect = "A" },
  { .name = "root",                 .input = "/",         .expect = "/" },
  { .name = "keeps_dot",            .input = "A/./B",     .expect = "A/./B" },
  { .name = "keeps_dotdot",         .input = "A/B/../C",  .expect = "A/B/../C" },
  { .name = "backslash_is_a_name",  .input = "A\\B\\",    .expect = "A\\B\\" },

  { .name = "windows_backslash",    .kind = SP_FS_PATH_WINDOWS, .input = "A\\B",             .expect = "A/B" },
  { .name = "windows_mixed",        .kind = SP_FS_PATH_WINDOWS, .input = "C:/A\\B/C\\D.txt", .expect = "C:/A/B/C/D.txt" },
  { .name = "windows_trailing",     .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\B\\",       .expect = "C:/A/B" },
  { .name = "windows_root",         .kind = SP_FS_PATH_WINDOWS, .input = "\\",               .expect = "/" },
  { .name = "windows_drive",        .kind = SP_FS_PATH_WINDOWS, .input = "C:\\",             .expect = "C:/" },
  { .name = "windows_keeps_dotdot", .kind = SP_FS_PATH_WINDOWS, .input = "A\\B\\..\\C",      .expect = "A/B/../C" },
};

sp_test_each(fs, normalize_path, test_t, tests) {
  sp_str_t result = sp_fs_normalize_path_for(sp_test_arena(t), sp_cstr_as_str(it->input), it->kind);
  sp_expect_str_eq_c(t, result, it->expect);
  return SP_OK;
}
