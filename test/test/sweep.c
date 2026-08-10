#include "sp/sp_test.h"

typedef enum {
  A,
  B,
} enum_t;

typedef struct {
  u64 n;
  u32 k;
  enum_t e1;
  enum_t e2;
} case_t;

static const c8* enum_name(s64 value);

sp_test_sweep(axis, sweep, case_t,
  .axes = {
    sp_test_axis(case_t, e1, A, B),
    sp_test_axis_named(case_t, e2, enum_name, A, B),
    sp_test_axis_range(case_t, n, 0, 1, 1),
    sp_test_axis_range(case_t, k, 69, 70, 1)
  }
) {
  sp_str_t want = sp_test_format(t, "axis.sweep.e1={}.e2={}.n={}.k={}", sp_fmt_uint(it->e1), sp_fmt_cstr(enum_name(it->e2)), sp_fmt_uint(it->n), sp_fmt_uint(it->k));
  sp_expect_str_eq(t, sp_test_get_name(t), want);
  return SP_OK;
}

static const c8* enum_name(s64 value) {
  return value == A ? "A" : "B";
}
