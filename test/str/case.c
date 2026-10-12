#include "sp/sp_test.h"

typedef struct {
  const c8* upper;
  const c8* lower;
  const c8* pascal_case;
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
    .expect = { .upper = "", .lower = "", .pascal_case = "" }
  },
  {
    .name = "mixed_words",
    .input = "Hello World",
    .expect = { .upper = "HELLO WORLD", .lower = "hello world", .pascal_case = "Hello World" }
  },
  {
    .name = "already_upper",
    .input = "ALREADY UPPER",
    .expect = { .upper = "ALREADY UPPER", .lower = "already upper", .pascal_case = "Already Upper" }
  },
  {
    .name = "already_lower",
    .input = "already lower",
    .expect = { .upper = "ALREADY LOWER", .lower = "already lower", .pascal_case = "Already Lower" }
  },
  {
    .name = "alternating",
    .input = "MiXeD cAsE",
    .expect = { .upper = "MIXED CASE", .lower = "mixed case", .pascal_case = "Mixed Case" }
  },
  {
    .name = "many_words",
    .input = "the quick brown fox",
    .expect = { .upper = "THE QUICK BROWN FOX", .lower = "the quick brown fox", .pascal_case = "The Quick Brown Fox" }
  },
  {
    .name = "leading_digits",
    .input = "123 numbers first",
    .expect = { .upper = "123 NUMBERS FIRST", .lower = "123 numbers first", .pascal_case = "123 Numbers First" }
  },
  {
    .name = "non_ascii_untouched",
    .input = "caf\xC3\xA9 123 {[]}",
    .expect = { .upper = "CAF\xC3\xA9 123 {[]}", .lower = "caf\xC3\xA9 123 {[]}", .pascal_case = "Caf\xC3\xA9 123 {[]}" }
  },
  {
    .name = "whitespace_splits_words",
    .input = "a\tb\nc\rd",
    .expect = { .upper = "A\tB\nC\rD", .lower = "a\tb\nc\rd", .pascal_case = "A\tB\nC\rD" }
  },
};

sp_test_each(str, case, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_str_to_upper(mem, input), it->expect.upper);
  sp_expect_str_eq_c(t, sp_str_to_lower(mem, input), it->expect.lower);
  sp_expect_str_eq_c(t, sp_str_to_pascal_case(mem, input), it->expect.pascal_case);
  return SP_OK;
}
