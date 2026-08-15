#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* a;
  const c8* b;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "basic",               .a = "A",   .b = "B",   .expect = "A/B" },
  { .name = "trailing_slash_lhs",  .a = "A/",  .b = "B",   .expect = "A/B" },
  { .name = "trailing_slash_both", .a = "A/",  .b = "B/",  .expect = "A/B" },
  { .name = "nested_rhs",          .a = "A",   .b = "B/C", .expect = "A/B/C" },
  { .name = "root_lhs",            .a = "/",   .b = "A",   .expect = "/A" },
  { .name = "root_lhs_nested",     .a = "/",   .b = "A/B", .expect = "/A/B" },
  { .name = "drive_slash_lhs",     .a = "C:/", .b = "A",   .expect = "C:/A" },
  { .name = "drive_bare_lhs",      .a = "C:",  .b = "A",   .expect = "C:/A" },
  { .name = "empty_lhs",           .a = "",    .b = "A",   .expect = "A" },
  { .name = "empty_rhs",           .a = "A",   .b = "",    .expect = "A" },
  { .name = "empty_both",          .a = "",    .b = "",    .expect = "" },
};

sp_test_each(fs, join_path, test_t, tests) {
  sp_str_t result = sp_fs_join_path(sp_test_arena(t), sp_str_view(it->a), sp_str_view(it->b));
  sp_expect_str_eq_c(t, result, it->expect);
  return SP_OK;
}
