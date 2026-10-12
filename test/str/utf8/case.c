#include "sp/sp_test.h"

typedef struct {
  u32 upper;
  u32 lower;
} expect_t;

typedef struct {
  const c8* name;
  u32 codepoint;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "lower_a",
    .codepoint = 'a',
    .expect = { .upper = 'A', .lower = 'a' }
  },
  {
    .name = "lower_z",
    .codepoint = 'z',
    .expect = { .upper = 'Z', .lower = 'z' }
  },
  {
    .name = "upper_a",
    .codepoint = 'A',
    .expect = { .upper = 'A', .lower = 'a' }
  },
  {
    .name = "upper_z",
    .codepoint = 'Z',
    .expect = { .upper = 'Z', .lower = 'z' }
  },
  {
    .name = "digit",
    .codepoint = '1',
    .expect = { .upper = '1', .lower = '1' }
  },
  {
    .name = "space",
    .codepoint = ' ',
    .expect = { .upper = ' ', .lower = ' ' }
  },
  {
    .name = "before_upper_a",
    .codepoint = '@',
    .expect = { .upper = '@', .lower = '@' }
  },
  {
    .name = "after_upper_z",
    .codepoint = '[',
    .expect = { .upper = '[', .lower = '[' }
  },
  {
    .name = "before_lower_a",
    .codepoint = '`',
    .expect = { .upper = '`', .lower = '`' }
  },
  {
    .name = "after_lower_z",
    .codepoint = '{',
    .expect = { .upper = '{', .lower = '{' }
  },
  {
    .name = "latin_1",
    .codepoint = 0xE9,
    .expect = { .upper = 0xE9, .lower = 0xE9 }
  },
  {
    .name = "emoji",
    .codepoint = 0x1F600,
    .expect = { .upper = 0x1F600, .lower = 0x1F600 }
  },
};

sp_test_each(utf8, case, test_t, tests) {
  sp_expect_eq(t, sp_utf8_to_upper(it->codepoint), it->expect.upper);
  sp_expect_eq(t, sp_utf8_to_lower(it->codepoint), it->expect.lower);
  return SP_OK;
}
