#include "sp/sp_test.h"

typedef struct {
  c8 at;
  c8 at_reverse;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  s32 index;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "first",
    .input = "Hello",
    .expect = { .at = 'H', .at_reverse = 'o' }
  },
  {
    .name = "second",
    .input = "Hello",
    .index = 1,
    .expect = { .at = 'e', .at_reverse = 'l' }
  },
  {
    .name = "last",
    .input = "Hello",
    .index = 4,
    .expect = { .at = 'o', .at_reverse = 'H' }
  },
  {
    .name = "negative_one",
    .input = "Hello",
    .index = -1,
    .expect = { .at = 'o', .at_reverse = 'H' }
  },
  {
    .name = "negative_two",
    .input = "Hello",
    .index = -2,
    .expect = { .at = 'l', .at_reverse = 'e' }
  },
  {
    .name = "negative_len",
    .input = "Hello",
    .index = -5,
    .expect = { .at = 'H', .at_reverse = 'o' }
  },
  {
    .name = "single",
    .input = "X",
    .expect = { .at = 'X', .at_reverse = 'X' }
  },
};

sp_test_each(str, at, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_eq(t, sp_str_at(input, it->index), it->expect.at);
  sp_expect_eq(t, sp_str_at_reverse(input, it->index), it->expect.at_reverse);
  sp_expect_eq(t, sp_str_back(input), sp_str_at(input, -1));
  return SP_OK;
}
