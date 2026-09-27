#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  const c8* ext;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "swap",                 .input = "A.c",      .ext = "o",   .expect = "A.o" },
  { .name = "strip",                .input = "A.c",      .ext = "",    .expect = "A" },
  { .name = "add",                  .input = "A",        .ext = "txt", .expect = "A.txt" },
  { .name = "trailing_dot",         .input = "A.",       .ext = "txt", .expect = "A.txt" },
  { .name = "double_dot",           .input = "A..txt",   .ext = "md",  .expect = "A..md" },
  { .name = "last_dot_wins",        .input = "A.B.C",    .ext = "d",   .expect = "A.B.d" },
  { .name = "hidden",               .input = ".A",       .ext = "txt", .expect = ".A.txt" },
  { .name = "nested",               .input = "A/B.txt",  .ext = "md",  .expect = "A/B.md" },
  { .name = "trailing_slash",       .input = "A/B.txt/", .ext = "md",  .expect = "A/B.md" },
  { .name = "dot_in_dir",           .input = "A.B/C",    .ext = "d",   .expect = "A.B/C.d" },
  { .name = "backslash_is_a_name",  .input = "A.B\\C",   .ext = "d",   .expect = "A.d" },

  { .name = "windows_dot_in_dir",   .kind = SP_FS_PATH_WINDOWS, .input = "A.B\\C",   .ext = "d", .expect = "A.B\\C.d" },
  { .name = "windows_trailing",     .kind = SP_FS_PATH_WINDOWS, .input = "A\\B.c\\", .ext = "o", .expect = "A\\B.o" },
};

sp_test_each(fs, replace_ext, test_t, tests) {
  sp_str_t result = sp_fs_replace_ext_for(sp_test_arena(t), sp_cstr_as_str(it->input), sp_cstr_as_str(it->ext), it->kind);
  sp_expect_str_eq_c(t, result, it->expect);
  return SP_OK;
}
