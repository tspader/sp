#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  u32 capacity;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "fits",
    .input = "Hello",
    .capacity = 8,
    .expect = "HelloZZZ"
  },
  {
    .name = "exact",
    .input = "Hello",
    .capacity = 5,
    .expect = "HelloZZZ"
  },
  {
    .name = "truncated",
    .input = "Hello World",
    .capacity = 8,
    .expect = "Hello Wo"
  },
  {
    .name = "capacity_shorter",
    .input = "Hello",
    .capacity = 2,
    .expect = "HeZZZZZZ"
  },
  {
    .name = "zero_capacity",
    .input = "Hello",
    .expect = "ZZZZZZZZ"
  },
  {
    .name = "empty",
    .input = "",
    .capacity = 8,
    .expect = "ZZZZZZZZ"
  },
  {
    .name = "null",
    .capacity = 8,
    .expect = "ZZZZZZZZ"
  },
};

sp_test_each(str, copy_to, test_t, tests) {
  c8 buffer [9] = "ZZZZZZZZ";
  sp_str_copy_to(sp_cstr_as_str(it->input), buffer, it->capacity);
  sp_expect_str_eq_c(t, sp_str(buffer, 8), it->expect);
  return SP_OK;
}
