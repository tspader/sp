#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  const c8* a;
  const c8* b;
  s32 expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "a_first",
    .a = "a",
    .b = "b",
    .expect = SP_QSORT_A_FIRST
  },
  {
    .name = "b_first",
    .a = "b",
    .b = "a",
    .expect = SP_QSORT_B_FIRST
  },
  {
    .name = "equal",
    .a = "same",
    .b = "same"
  },
  {
    .name = "shorter_prefix_first",
    .a = "ab",
    .b = "abc",
    .expect = SP_QSORT_A_FIRST
  },
  {
    .name = "longer_prefix_last",
    .a = "abc",
    .b = "ab",
    .expect = SP_QSORT_B_FIRST
  },
  {
    .name = "both_empty",
    .a = "",
    .b = ""
  },
  {
    .name = "empty_first",
    .a = "",
    .b = "a",
    .expect = SP_QSORT_A_FIRST
  },
  {
    .name = "empty_last",
    .a = "a",
    .b = "",
    .expect = SP_QSORT_B_FIRST
  },
  {
    .name = "upper_before_lower",
    .a = "B",
    .b = "a",
    .expect = SP_QSORT_A_FIRST
  },
};

sp_test_each(str, compare, test_t, tests) {
  sp_str_t a = sp_cstr_as_str(it->a);
  sp_str_t b = sp_cstr_as_str(it->b);
  sp_expect_eq(t, sp_str_compare_alphabetical(a, b), it->expect);
  sp_expect_eq(t, sp_str_sort_kernel_alphabetical(&a, &b), it->expect);
  return SP_OK;
}
