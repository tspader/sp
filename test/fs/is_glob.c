#include "sp.h"
#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  bool glob;
} test_t;

static const test_t tests [] = {
  { .name = "empty",             .input = "" },
  { .name = "plain",             .input = "A" },
  { .name = "plain_ext",         .input = "A.txt" },
  { .name = "question_not_glob", .input = "A?B" },
  { .name = "star",              .input = "*",     .glob = true },
  { .name = "star_ext",          .input = "*.txt", .glob = true },
  { .name = "star_in_dir",       .input = "A/*",   .glob = true },
  { .name = "star_infix",        .input = "A*B",   .glob = true },
};

sp_test_each(fs, is_glob, test_t, tests) {
  sp_expect_eq(t, sp_fs_is_glob(sp_str_view(it->input)), it->glob);
  return SP_OK;
}
