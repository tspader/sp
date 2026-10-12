#include "sp/sp_test.h"

typedef struct {
  s32 first;
  s32 last;
} expect_t;

typedef struct {
  const c8* name;
  const c8* str;
  c8 needle;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "first_char",
    .str = "hello world",
    .needle = 'h'
  },
  {
    .name = "repeated",
    .str = "hello world",
    .needle = 'o',
    .expect = { .first = 4, .last = 7 }
  },
  {
    .name = "last_char",
    .str = "hello world",
    .needle = 'd',
    .expect = { .first = 10, .last = 10 }
  },
  {
    .name = "repeated_vowel",
    .str = "banana",
    .needle = 'a',
    .expect = { .first = 1, .last = 5 }
  },
  {
    .name = "repeated_consonant",
    .str = "banana",
    .needle = 'n',
    .expect = { .first = 2, .last = 4 }
  },
  {
    .name = "path_sep",
    .str = "/home/user/file.txt",
    .needle = '/',
    .expect = { .last = 10 }
  },
  {
    .name = "path_dot",
    .str = "/home/user/file.txt",
    .needle = '.',
    .expect = { .first = 15, .last = 15 }
  },
  {
    .name = "empty",
    .str = "",
    .needle = 'x',
    .expect = { .first = SP_STR_NO_MATCH, .last = SP_STR_NO_MATCH }
  },
  {
    .name = "absent",
    .str = "hello",
    .needle = 'z',
    .expect = { .first = SP_STR_NO_MATCH, .last = SP_STR_NO_MATCH }
  },
  {
    .name = "single",
    .str = "x",
    .needle = 'x'
  },
};

sp_test_each(str, find_c8, test_t, tests) {
  sp_str_t str = sp_cstr_as_str(it->str);
  sp_expect_eq(t, sp_str_find_c8(str, it->needle), it->expect.first);
  sp_expect_eq(t, sp_str_find_c8_reverse(str, it->needle), it->expect.last);
  return SP_OK;
}
