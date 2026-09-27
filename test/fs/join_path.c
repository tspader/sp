#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  sp_fs_path_kind_t kind;
  const c8* a;
  const c8* b;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "basic",                .a = "A",    .b = "B",    .expect = "A/B" },
  { .name = "nested_rhs",           .a = "A",    .b = "B/C",  .expect = "A/B/C" },
  { .name = "trailing_lhs",         .a = "A/",   .b = "B",    .expect = "A/B" },
  { .name = "trailing_rhs",         .a = "A",    .b = "B/",   .expect = "A/B" },
  { .name = "leading_rhs",          .a = "A",    .b = "/B",   .expect = "A/B" },
  { .name = "leading_run_rhs",      .a = "A/",   .b = "//B",  .expect = "A/B" },
  { .name = "absolute_both",        .a = "/A",   .b = "/B",   .expect = "/A/B" },
  { .name = "root_lhs",             .a = "/",    .b = "A",    .expect = "/A" },
  { .name = "root_both",            .a = "/",    .b = "/A",   .expect = "/A" },
  { .name = "root_rhs",             .a = "A",    .b = "/",    .expect = "A" },
  { .name = "dot_lhs",              .a = ".",    .b = "A",    .expect = "./A" },
  { .name = "empty_lhs",            .a = "",     .b = "A",    .expect = "A" },
  { .name = "empty_lhs_absolute",   .a = "",     .b = "/A",   .expect = "/A" },
  { .name = "empty_rhs",            .a = "A",    .b = "",     .expect = "A" },
  { .name = "empty_both",           .a = "",     .b = "",     .expect = "" },
  { .name = "backslash_is_a_name",  .a = "A\\",  .b = "\\B",  .expect = "A\\/\\B" },

  { .name = "windows_trailing_lhs", .kind = SP_FS_PATH_WINDOWS, .a = "A\\",  .b = "B",    .expect = "A/B" },
  { .name = "windows_leading_rhs",  .kind = SP_FS_PATH_WINDOWS, .a = "A",    .b = "\\B",  .expect = "A/B" },
  { .name = "windows_drive",        .kind = SP_FS_PATH_WINDOWS, .a = "C:/",  .b = "A",    .expect = "C:/A" },
  { .name = "windows_drive_back",   .kind = SP_FS_PATH_WINDOWS, .a = "C:\\", .b = "A",    .expect = "C:\\A" },
  { .name = "windows_drive_bare",   .kind = SP_FS_PATH_WINDOWS, .a = "C:",   .b = "A",    .expect = "C:/A" },
};

sp_test_each(fs, join_path, test_t, tests) {
  sp_str_t result = sp_fs_join_path_for(sp_test_arena(t), sp_cstr_as_str(it->a), sp_cstr_as_str(it->b), it->kind);
  sp_expect_str_eq_c(t, result, it->expect);
  return SP_OK;
}
