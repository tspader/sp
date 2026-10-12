#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  bool expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "empty",
    .input = "",
    .expect = true
  },
  {
    .name = "ascii",
    .input = "hello",
    .expect = true
  },
  {
    .name = "two_byte",
    .input = "\xC2\xA2",
    .expect = true
  },
  {
    .name = "three_byte",
    .input = "\xE2\x82\xAC",
    .expect = true
  },
  {
    .name = "four_byte",
    .input = "\xF0\x9F\x98\x80",
    .expect = true
  },
  {
    .name = "mixed_widths",
    .input = "a\xC2\xA2\xE2\x82\xACz",
    .expect = true
  },
  {
    .name = "lone_continuation",
    .input = "\x80"
  },
  {
    .name = "truncated_two_byte",
    .input = "\xC2"
  },
  {
    .name = "truncated_three_byte",
    .input = "\xE2\x82"
  },
  {
    .name = "truncated_four_byte",
    .input = "\xF0\x9F\x98"
  },
  {
    .name = "bad_continuation",
    .input = "\xC3\x28"
  },
  {
    .name = "overlong_two_byte",
    .input = "\xC0\x80"
  },
  {
    .name = "overlong_four_byte",
    .input = "\xF0\x80\x80\x80"
  },
  {
    .name = "overlong_four_byte_max",
    .input = "\xF0\x8F\xBF\xBF"
  },
  {
    .name = "surrogate",
    .input = "\xED\xA0\x80"
  },
  {
    .name = "above_max",
    .input = "\xF4\x90\x80\x80"
  },
};

sp_test_each(utf8, validate, test_t, tests) {
  sp_expect_eq(t, sp_utf8_validate(sp_cstr_as_str(it->input)), it->expect);
  return SP_OK;
}
