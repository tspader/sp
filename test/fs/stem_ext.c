#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* stem;
  const c8* ext;
} expect_t;

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* input;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "empty",               .input = "",         .expect = { "",     "" } },
  { .name = "none",                .input = "A",        .expect = { "A",    "" } },
  { .name = "simple",              .input = "A.B",      .expect = { "A",    "B" } },
  { .name = "nested",              .input = "A/B.txt",  .expect = { "B",    "txt" } },
  { .name = "trailing_slash",      .input = "A/B.txt/", .expect = { "B",    "txt" } },
  { .name = "last_dot_wins",       .input = "A.B.C",    .expect = { "A.B",  "C" } },
  { .name = "double_dot",          .input = "A..txt",   .expect = { "A.",   "txt" } },
  { .name = "trailing_dot",        .input = "A.",       .expect = { "A",    "" } },
  { .name = "dot_in_dir",          .input = "/A/B.C/D", .expect = { "D",    "" } },
  { .name = "hidden",              .input = ".A",       .expect = { ".A",   "" } },
  { .name = "hidden_with_ext",     .input = ".A.txt",   .expect = { ".A",   "txt" } },
  { .name = "dot",                 .input = ".",        .expect = { ".",    "" } },
  { .name = "dotdot",              .input = "..",       .expect = { "..",   "" } },
  { .name = "root",                .input = "/",        .expect = { "",     "" } },
  { .name = "backslash_is_a_name", .input = "A.B\\C",   .expect = { "A",    "B\\C" } },

  { .name = "windows_backslash",   .kind = SP_FS_PATH_WINDOWS, .input = "C:\\A\\B.txt", .expect = { "B", "txt" } },
  { .name = "windows_dot_in_dir",  .kind = SP_FS_PATH_WINDOWS, .input = "A.B\\C",       .expect = { "C", "" } },
};

sp_test_each(fs, stem_ext, test_t, tests) {
  sp_str_t path = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_fs_get_stem_for(path, it->kind), it->expect.stem);
  sp_expect_str_eq_c(t, sp_fs_get_ext_for(path, it->kind), it->expect.ext);
  return SP_OK;
}
