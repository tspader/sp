#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  const c8* ext;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  { .name = "swap",          .input = "A.c",     .ext = "o",   .expect = "A.o" },
  { .name = "strip",         .input = "A.c",     .ext = "",    .expect = "A" },
  { .name = "add",           .input = "A",       .ext = "txt", .expect = "A.txt" },
  { .name = "trailing_dot",  .input = "A.",      .ext = "txt", .expect = "A.txt" },
  { .name = "double_dot",    .input = "A..txt",  .ext = "md",  .expect = "A..md" },
  { .name = "multiple_dots", .input = "A.B.C",   .ext = "d",   .expect = "A.B.d" },
  { .name = "hidden",        .input = ".A",      .ext = "txt", .expect = ".txt" },
  { .name = "nested",        .input = "A/B.txt", .ext = "md",  .expect = "A/B.md" },
};

sp_test_each(fs, replace_ext, test_t, tests) {
  sp_str_t result = sp_fs_replace_ext(sp_test_arena(t), sp_str_view(it->input), sp_str_view(it->ext));
  sp_expect_str_eq_c(t, result, it->expect);
  return SP_OK;
}
