#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* a;
  const c8* b;
  bool expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "same",
    .a = "Hello",
    .b = "Hello",
    .expect = true
  },
  {
    .name = "both_empty",
    .a = "",
    .b = "",
    .expect = true
  },
  {
    .name = "both_null",
    .expect = true
  },
  {
    .name = "null_vs_empty",
    .b = "",
    .expect = true
  },
  {
    .name = "different",
    .a = "Hello",
    .b = "World"
  },
  {
    .name = "b_longer",
    .a = "Hello",
    .b = "Hello!"
  },
  {
    .name = "b_shorter",
    .a = "Hello",
    .b = "Hell"
  },
  {
    .name = "b_null",
    .a = "Hello"
  },
  {
    .name = "a_null",
    .b = "Hello"
  },
  {
    .name = "case_sensitive",
    .a = "Hello",
    .b = "hello"
  },
};

sp_test_each(cstr, equal, test_t, tests) {
  sp_expect_eq(t, sp_cstr_equal(it->a, it->b), it->expect);
  return SP_OK;
}
