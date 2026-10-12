#include "sp/sp_test.h"

typedef struct {
  const c8* left;
  const c8* right;
  const c8* both;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  const c8* strip;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "both_sides",
    .input = "sp_foo_t_sp",
    .strip = "sp",
    .expect = { .left = "_foo_t_sp", .right = "sp_foo_t_", .both = "_foo_t_" }
  },
  {
    .name = "one_of_run",
    .input = "___hello___",
    .strip = "_",
    .expect = { .left = "__hello___", .right = "___hello__", .both = "__hello__" }
  },
  {
    .name = "multi_char",
    .input = "xyzabcxyz",
    .strip = "xyz",
    .expect = { .left = "abcxyz", .right = "xyzabc", .both = "abc" }
  },
  {
    .name = "left_only",
    .input = "sp_foo_t",
    .strip = "sp",
    .expect = { .left = "_foo_t", .right = "sp_foo_t", .both = "_foo_t" }
  },
  {
    .name = "right_only",
    .input = "foo_t_sp",
    .strip = "sp",
    .expect = { .left = "foo_t_sp", .right = "foo_t_", .both = "foo_t_" }
  },
  {
    .name = "no_match",
    .input = "hello",
    .strip = "xyz",
    .expect = { .left = "hello", .right = "hello", .both = "hello" }
  },
  {
    .name = "empty_strip",
    .input = "hello",
    .strip = "",
    .expect = { .left = "hello", .right = "hello", .both = "hello" }
  },
  {
    .name = "empty_input",
    .input = "",
    .strip = "abc",
    .expect = { .left = "", .right = "", .both = "" }
  },
  {
    .name = "whole",
    .input = "sp",
    .strip = "sp",
    .expect = { .left = "", .right = "", .both = "" }
  },
  {
    .name = "doubled",
    .input = "spsp",
    .strip = "sp",
    .expect = { .left = "sp", .right = "sp", .both = "" }
  },
};

sp_test_each(str, strip, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_str_t strip = sp_cstr_as_str(it->strip);
  sp_expect_str_eq_c(t, sp_str_strip_left(input, strip), it->expect.left);
  sp_expect_str_eq_c(t, sp_str_strip_right(input, strip), it->expect.right);
  sp_expect_str_eq_c(t, sp_str_strip(input, strip), it->expect.both);
  return SP_OK;
}
