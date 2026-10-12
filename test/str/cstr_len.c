#include "sp/sp_test.h"

typedef struct {
  u32 len;
  u32 len_n;
} expect_t;

typedef struct {
  const c8* name;
  const c8* input;
  u32 n;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "n_shorter",
    .input = "Hello",
    .n = 3,
    .expect = { .len = 5, .len_n = 3 }
  },
  {
    .name = "n_longer",
    .input = "Hello World!",
    .n = 20,
    .expect = { .len = 12, .len_n = 12 }
  },
  {
    .name = "n_zero",
    .input = "Hello",
    .expect = { .len = 5 }
  },
  {
    .name = "empty",
    .input = "",
    .n = 5
  },
  {
    .name = "null",
    .n = 5
  },
  {
    .name = "embedded_nul",
    .input = "He\0lo",
    .n = 5,
    .expect = { .len = 2, .len_n = 2 }
  },
};

sp_test_each(cstr, len, test_t, tests) {
  sp_expect_eq(t, sp_cstr_len(it->input), it->expect.len);
  sp_expect_eq(t, sp_cstr_len_n(it->input, it->n), it->expect.len_n);
  return SP_OK;
}
