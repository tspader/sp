#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  u32 n;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "shorter",
    .input = "hello",
    .n = 10,
    .expect = "hello     "
  },
  {
    .name = "by_three",
    .input = "hi",
    .n = 5,
    .expect = "hi   "
  },
  {
    .name = "longer",
    .input = "hello world",
    .n = 5,
    .expect = "hello world"
  },
  {
    .name = "exact",
    .input = "hello",
    .n = 5,
    .expect = "hello"
  },
  {
    .name = "empty",
    .input = "",
    .n = 5,
    .expect = "     "
  },
  {
    .name = "zero",
    .input = "hello",
    .expect = "hello"
  },
  {
    .name = "empty_zero",
    .input = "",
    .expect = ""
  },
};

sp_test_each(str, pad, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_str_pad(sp_test_arena(t), input, it->n), it->expect);
  return SP_OK;
}
