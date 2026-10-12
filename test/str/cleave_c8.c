#include "sp/sp_test.h"

typedef struct {
  const c8* first;
  const c8* second;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  c8 delimiter;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "simple",
    .input = "hello,world",
    .delimiter = ',',
    .expect = { .first = "hello", .second = "world" }
  },
  {
    .name = "key_value",
    .input = "key=value",
    .delimiter = '=',
    .expect = { .first = "key", .second = "value" }
  },
  {
    .name = "no_delimiter",
    .input = "no_delimiter",
    .delimiter = ',',
    .expect = { .first = "no_delimiter", .second = "" }
  },
  {
    .name = "first_delimiter_wins",
    .input = "a,b,c",
    .delimiter = ',',
    .expect = { .first = "a", .second = "b,c" }
  },
  {
    .name = "leading",
    .input = ",trailing",
    .delimiter = ',',
    .expect = { .first = "", .second = "trailing" }
  },
  {
    .name = "trailing",
    .input = "leading,",
    .delimiter = ',',
    .expect = { .first = "leading", .second = "" }
  },
  {
    .name = "empty",
    .input = "",
    .delimiter = ',',
    .expect = { .first = "", .second = "" }
  },
  {
    .name = "only_delimiter",
    .input = ",",
    .delimiter = ',',
    .expect = { .first = "", .second = "" }
  },
};

sp_test_each(str, cleave_c8, test_t, tests) {
  sp_str_pair_t pair = sp_str_cleave_c8(sp_cstr_as_str(it->input), it->delimiter);
  sp_expect_str_eq_c(t, pair.first, it->expect.first);
  sp_expect_str_eq_c(t, pair.second, it->expect.second);
  return SP_OK;
}
