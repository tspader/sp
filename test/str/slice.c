#include "sp/sp_test.h"

typedef struct {
  const c8* prefix;
  const c8* suffix;
  const c8* skip;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  s32 n;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "zero",
    .input = "jerry",
    .expect = { .prefix = "", .suffix = "", .skip = "jerry" }
  },
  {
    .name = "one",
    .input = "jerry",
    .n = 1,
    .expect = { .prefix = "j", .suffix = "y", .skip = "erry" }
  },
  {
    .name = "middle",
    .input = "jerry",
    .n = 3,
    .expect = { .prefix = "jer", .suffix = "rry", .skip = "ry" }
  },
  {
    .name = "whole",
    .input = "jerry",
    .n = 5,
    .expect = { .prefix = "jerry", .suffix = "jerry", .skip = "" }
  },
  {
    .name = "empty",
    .input = "",
    .expect = { .prefix = "", .suffix = "", .skip = "" }
  },
};

sp_test_each(str, slice, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_str_prefix(input, it->n), it->expect.prefix);
  sp_expect_str_eq_c(t, sp_str_suffix(input, it->n), it->expect.suffix);
  sp_expect_str_eq_c(t, sp_str_skip(input, it->n), it->expect.skip);
  return SP_OK;
}
