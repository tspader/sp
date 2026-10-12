#include "sp/sp_test.h"

#define MAX_PARTS 8

typedef struct {
  const c8* name;
  const c8* input;
  c8 delimiter;
  const c8* parts [MAX_PARTS];
} test_t;

static const test_t tests [] = {
  {
    .name = "empty",
    .input = "",
    .delimiter = ','
  },
  {
    .name = "no_delimiter",
    .input = "hello",
    .delimiter = ',',
    .parts = { "hello" }
  },
  {
    .name = "single_char",
    .input = "x",
    .delimiter = ',',
    .parts = { "x" }
  },
  {
    .name = "only_delimiter",
    .input = ",",
    .delimiter = ',',
    .parts = { "", "" }
  },
  {
    .name = "simple",
    .input = "hello,world,test",
    .delimiter = ',',
    .parts = { "hello", "world", "test" }
  },
  {
    .name = "leading",
    .input = "/home/user/file.txt",
    .delimiter = '/',
    .parts = { "", "home", "user", "file.txt" }
  },
  {
    .name = "leading_and_trailing",
    .input = ",hello,world,",
    .delimiter = ',',
    .parts = { "", "hello", "world", "" }
  },
  {
    .name = "empty_middle",
    .input = "a,,b",
    .delimiter = ',',
    .parts = { "a", "", "b" }
  },
  {
    .name = "empty_middle_run",
    .input = "a,,,b",
    .delimiter = ',',
    .parts = { "a", "", "", "b" }
  },
};

sp_test_each(str, split_c8, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);

  u32 count = 0;
  sp_carr_for(it->parts, i) {
    if (!it->parts[i]) break;
    count++;
  }

  sp_da(sp_str_t) parts = sp_str_split_c8(sp_test_arena(t), input, it->delimiter);
  sp_must_eq(t, sp_da_size(parts), count);
  sp_expect_strs_eq(t, parts, count, it->parts);

  u32 n = 0;
  sp_str_for_split(input, it->delimiter, split) {
    sp_must_lt(t, n, count);
    sp_expect_str_eq_c(t, split.value, it->parts[n]);
    n++;
  }
  sp_expect_eq(t, n, count);

  return SP_OK;
}
