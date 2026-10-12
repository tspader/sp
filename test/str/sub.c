#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* input;
  s32 index;
  s32 len;
  const c8* expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "head",
    .input = "Jerry Garcia",
    .len = 5,
    .expect = "Jerry"
  },
  {
    .name = "tail",
    .input = "Jerry Garcia",
    .index = 6,
    .len = 6,
    .expect = "Garcia"
  },
  {
    .name = "middle",
    .input = "Jerry Garcia",
    .index = 5,
    .len = 1,
    .expect = " "
  },
  {
    .name = "last",
    .input = "Jerry Garcia",
    .index = 11,
    .len = 1,
    .expect = "a"
  },
  {
    .name = "whole",
    .input = "Jerry Garcia",
    .len = 12,
    .expect = "Jerry Garcia"
  },
  {
    .name = "zero_len",
    .input = "Jerry Garcia",
    .index = 3,
    .expect = ""
  },
  {
    .name = "empty",
    .input = "",
    .expect = ""
  },
};

sp_test_each(str, sub, test_t, tests) {
  sp_str_t input = sp_cstr_as_str(it->input);
  sp_expect_str_eq_c(t, sp_str_sub(input, it->index, it->len), it->expect);
  return SP_OK;
}
