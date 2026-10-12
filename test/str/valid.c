#include "sp/sp_test.h"

typedef struct {
  bool valid;
  bool empty;
} expect_t;

typedef struct {
  const c8* name;
  const c8* data;
  u32 len;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "null",
    .expect = { .empty = true }
  },
  {
    .name = "null_with_len",
    .len = 5
  },
  {
    .name = "literal",
    .data = "Hello",
    .len = 5,
    .expect = { .valid = true }
  },
  {
    .name = "empty_literal",
    .data = "",
    .expect = { .valid = true, .empty = true }
  },
};

sp_test_each(str, valid, test_t, tests) {
  sp_str_t str = sp_str(it->data, it->len);
  sp_expect_eq(t, sp_str_valid(str), it->expect.valid);
  sp_expect_eq(t, sp_str_empty(str), it->expect.empty);
  return SP_OK;
}
