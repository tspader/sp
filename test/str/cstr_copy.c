#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  u32 n;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "full",
    .input = "Hello World",
    .n = 11,
    .expect = "Hello World"
  },
  {
    .name = "partial",
    .input = "Hello World",
    .n = 5,
    .expect = "Hello"
  },
  {
    .name = "zero_n",
    .input = "Hello",
    .expect = ""
  },
  {
    .name = "empty",
    .input = "",
    .expect = ""
  },
  {
    .name = "null",
    .expect = ""
  },
  {
    .name = "null_with_n",
    .n = 5,
    .expect = ""
  },
};

sp_test_each(cstr, copy, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);

  c8* copy = sp_cstr_copy_n(mem, it->input, it->n);
  sp_expect_str_eq_c(t, sp_cstr_as_str(copy), it->expect);
  sp_expect(t, copy != it->input);

  sp_str_t full = sp_cstr_as_str(sp_cstr_copy(mem, it->input));
  sp_str_t full_n = sp_cstr_as_str(sp_cstr_copy_n(mem, it->input, sp_cstr_len(it->input)));
  sp_expect_str_eq(t, full, full_n);
  return SP_OK;
}
