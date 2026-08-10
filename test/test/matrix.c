#include "sp/sp_test.h"

typedef struct {
  const c8* name;
  u32 base;
  u32 bit;
} matrix_t;

static const matrix_t matrix_cases [] = {
  { .name = "A", .base = 1 },
  { .name = "B", .base = 2 },
};

sp_test_each(axis, matrix, matrix_t, matrix_cases,
  .axes = {
    sp_test_axis(matrix_t, bit, 8, 16),
  }
) {
  sp_str_t want = sp_test_format(t, "axis.matrix.{}.bit={}", sp_fmt_cstr(it->name), sp_fmt_uint(it->bit));
  sp_expect_str_eq(t, sp_test_get_name(t), want);
  sp_expect_eq(t, it->base, it->name[0] == 'A' ? 1u : 2u);
  return SP_OK;
}
