#include "sp.h"
#include "sp/sp_test.h"

#define DIR_FD 9000

typedef struct {
  const c8* name;
  const c8* sub;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "nested",          .sub = "A/B",   .expect = "A" },
  { .name = "bare",            .sub = "A",     .expect = "." },
  { .name = "trailing_sep",    .sub = "A/B/",  .expect = "A" },
  { .name = "root_label",      .sub = ".",     .expect = "./.." },
  { .name = "trailing_dot",    .sub = "./",    .expect = "./.." },
  { .name = "empty",           .sub = "",      .expect = "" },
  { .name = "dotdot",          .sub = "..",    .expect = "../.." },
  { .name = "nested_dotdot",   .sub = "A/..",  .expect = "A/../.." },
  { .name = "trailing_dotdot", .sub = "A/../", .expect = "A/../.." },
  { .name = "nested_dot",      .sub = "A/.",   .expect = "A/./.." },
  { .name = "root",            .sub = "/",     .expect = "/" },
  { .name = "under_root",      .sub = "/A",    .expect = "/" },
};

sp_test_each(fs, path_parent, test_t, tests) {
  sp_path_t path = sp_path((sp_sys_fd_t)DIR_FD, sp_cstr_as_str(it->sub));
  sp_path_t result = sp_path_parent(sp_test_arena(t), path);
  sp_expect_eq(t, result.dir, (sp_sys_fd_t)DIR_FD);
  sp_expect_str_eq_c(t, result.sub, it->expect);
  return SP_OK;
}
