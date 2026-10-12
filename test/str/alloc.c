#include "sp/sp_test.h"

typedef struct {
  bool valid;
} expect_t;

typedef struct {
  const c8* name;
  u32 capacity;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "zero"
  },
  {
    .name = "some",
    .capacity = 100,
    .expect = { .valid = true }
  },
};

sp_test_each(str, alloc, test_t, tests) {
  sp_str_t str = sp_str_alloc(sp_test_arena(t), it->capacity);
  sp_expect_eq(t, str.len, 0u);
  sp_expect_eq(t, sp_str_valid(str), it->expect.valid);
  return SP_OK;
}
