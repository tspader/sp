#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  u32 n;
  u32 capacity;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "fits",
    .input = "Hello",
    .n = 5,
    .capacity = 8,
    .expect = "Hello"
  },
  {
    .name = "exact",
    .input = "Hello",
    .n = 5,
    .capacity = 6,
    .expect = "Hello"
  },
  {
    .name = "truncated",
    .input = "Hello World",
    .n = 11,
    .capacity = 6,
    .expect = "Hello"
  },
  {
    .name = "partial_n",
    .input = "Hello World",
    .n = 5,
    .capacity = 8,
    .expect = "Hello"
  },
  {
    .name = "zero_capacity",
    .input = "Hello",
    .n = 5,
    .expect = "ZZZZZZZ"
  },
  {
    .name = "null",
    .capacity = 8,
    .expect = "ZZZZZZZ"
  },
  {
    .name = "empty",
    .input = "",
    .capacity = 8,
    .expect = ""
  },
};

sp_test_each(cstr, copy_to, test_t, tests) {
  c8 buffer [8] = "ZZZZZZZ";
  sp_cstr_copy_to_n(it->input, it->n, buffer, it->capacity);
  sp_expect_str_eq_c(t, sp_cstr_as_str(buffer), it->expect);

  c8 full [8] = "ZZZZZZZ";
  c8 full_n [8] = "ZZZZZZZ";
  sp_cstr_copy_to(it->input, full, it->capacity);
  sp_cstr_copy_to_n(it->input, sp_cstr_len(it->input), full_n, it->capacity);
  sp_expect_str_eq(t, sp_cstr_as_str(full), sp_cstr_as_str(full_n));
  return SP_OK;
}
