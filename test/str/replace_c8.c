#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  c8 from;
  c8 to;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "multiple",
    .input = "hello world",
    .from = 'l',
    .to = 'X',
    .expect = "heXXo worXd"
  },
  {
    .name = "no_match",
    .input = "hello world",
    .from = 'z',
    .to = 'X',
    .expect = "hello world"
  },
  {
    .name = "all",
    .input = "aaa",
    .from = 'a',
    .to = 'b',
    .expect = "bbb"
  },
  {
    .name = "empty",
    .input = "",
    .from = 'a',
    .to = 'b',
    .expect = ""
  },
  {
    .name = "same_from_to",
    .input = "abc",
    .from = 'a',
    .to = 'a',
    .expect = "abc"
  },
  {
    .name = "single",
    .input = "x",
    .from = 'x',
    .to = 'y',
    .expect = "y"
  },
  {
    .name = "first_and_last",
    .input = "/path/to/file/",
    .from = '/',
    .to = '\\',
    .expect = "\\path\\to\\file\\"
  },
};

sp_test_each(str, replace_c8, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_str_replace_c8(sp_test_arena(t), input, it->from, it->to), it->expect);
  return SP_OK;
}
