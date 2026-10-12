#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  u32 n;
  const c8* trailer;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "longer",
    .input = "hello world",
    .n = 8,
    .trailer = "...",
    .expect = "hello..."
  },
  {
    .name = "shorter",
    .input = "hi",
    .n = 10,
    .trailer = "...",
    .expect = "hi"
  },
  {
    .name = "exact",
    .input = "exactly",
    .n = 7,
    .trailer = "...",
    .expect = "exactly"
  },
  {
    .name = "zero_is_unlimited",
    .input = "test",
    .trailer = "...",
    .expect = "test"
  },
  {
    .name = "trailer_longer_than_limit",
    .input = "hello world",
    .n = 2,
    .trailer = "...",
    .expect = "hello world"
  },
  {
    .name = "trailer_fills_limit",
    .input = "hello world",
    .n = 3,
    .trailer = "...",
    .expect = "..."
  },
  {
    .name = "empty_trailer",
    .input = "hello world",
    .n = 5,
    .trailer = "",
    .expect = "hello"
  },
};

sp_test_each(str, truncate, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_str_t trailer = sp_cstr_as_str(it->trailer);
  sp_expect_str_eq_c(t, sp_str_truncate(sp_test_arena(t), input, it->n, trailer), it->expect);
  return SP_OK;
}
