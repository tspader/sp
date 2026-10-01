#include "sp.h"
#include "sp/sp_test.h"

#define DIR_FD 9000

typedef struct {
  const c8* name;
  const c8* a;
  const c8* b;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "basic",        .a = "A",  .b = "B",   .expect = "A/B" },
  { .name = "nested_rhs",   .a = "A",  .b = "B/C", .expect = "A/B/C" },
  { .name = "dot_lhs",      .a = ".",  .b = "A",   .expect = "./A" },
  { .name = "trailing_lhs", .a = "A/", .b = "B",   .expect = "A/B" },
  { .name = "leading_rhs",  .a = "A",  .b = "/B",  .expect = "A/B" },
  { .name = "empty_lhs",    .a = "",   .b = "A",   .expect = "A" },
  { .name = "empty_rhs",    .a = "A",  .b = "",    .expect = "A" },
};

sp_test_each(fs, path_join, test_t, tests) {
  sp_path_t path = sp_path((sp_sys_fd_t)DIR_FD, sp_cstr_as_str(it->a));
  sp_path_t result = sp_path_join(sp_test_arena(t), path, sp_cstr_as_str(it->b));
  sp_expect_eq(t, result.dir, (sp_sys_fd_t)DIR_FD);
  sp_expect_str_eq_c(t, result.sub, it->expect);
  return SP_OK;
}
