#include "sp/sp_test.h"

typedef struct {
  const c8* left;
  const c8* right;
  const c8* both;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "empty",
    .input = "",
    .expect = { .left = "", .right = "", .both = "" }
  },
  {
    .name = "spaces_only",
    .input = "   ",
    .expect = { .left = "", .right = "", .both = "" }
  },
  {
    .name = "mixed_only",
    .input = "\t\n\r",
    .expect = { .left = "", .right = "", .both = "" }
  },
  {
    .name = "none",
    .input = "hello",
    .expect = { .left = "hello", .right = "hello", .both = "hello" }
  },
  {
    .name = "spaces",
    .input = "  hello  ",
    .expect = { .left = "hello  ", .right = "  hello", .both = "hello" }
  },
  {
    .name = "tabs",
    .input = "\thello\t",
    .expect = { .left = "hello\t", .right = "\thello", .both = "hello" }
  },
  {
    .name = "newlines",
    .input = "\nhello\n",
    .expect = { .left = "hello\n", .right = "\nhello", .both = "hello" }
  },
  {
    .name = "mixed",
    .input = "  \t\nhello\n\t  ",
    .expect = { .left = "hello\n\t  ", .right = "  \t\nhello", .both = "hello" }
  },
  {
    .name = "inner_preserved",
    .input = "  hello world  ",
    .expect = { .left = "hello world  ", .right = "  hello world", .both = "hello world" }
  },
  {
    .name = "inner_tab_preserved",
    .input = "\ttab\tseparated\t",
    .expect = { .left = "tab\tseparated\t", .right = "\ttab\tseparated", .both = "tab\tseparated" }
  },
};

sp_test_each(str, trim, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_str_trim_left(input), it->expect.left);
  sp_expect_str_eq_c(t, sp_str_trim_right(input), it->expect.right);
  sp_expect_str_eq_c(t, sp_str_trim(input), it->expect.both);
  return SP_OK;
}
