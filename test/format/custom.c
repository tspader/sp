#include "format.h"

static sp_err_t render_hex(sp_io_writer_t* io, sp_fmt_arg_t* arg) {
  return sp_fmt_write_u64_ex(io, arg->value.u, SP_FMT_RADIX_HEX);
}

UTEST(format_custom, fn) {
  format_fmt_test_t cases[] = {
    {
      .fmt = "{}",
      .args = { sp_fmt_u64_custom(255, render_hex) },
      .expect = "ff"
    },
    {
      .fmt = "{:>4}",
      .args = { sp_fmt_u64_custom(255, render_hex) },
      .expect = "  ff"
    },
  };
  SP_CARR_FOR(cases, i) run_format_fmt(utest_result, cases[i]);
}

typedef struct { s32 x; s32 y; } point_t;

static sp_err_t render_point(sp_io_writer_t* io, sp_fmt_arg_t* arg) {
  point_t* p = (point_t*)arg->value.p;
  sp_io_write_c8(io, '(');
  sp_fmt_write_s64(io, p->x);
  sp_io_write_cstr(io, ", ", SP_NULLPTR);
  sp_fmt_write_s64(io, p->y);
  return sp_io_write_c8(io, ')');
}

#define sp_fmt_point(_p) sp_fmt_custom(point_t, render_point, (_p))

UTEST(format_custom, by_pointer) {
  point_t pt = { 3, 4 };
  SP_EXPECT_STR_EQ_CSTR(sp_fmt(sp_mem_get_scratch(), "{}", sp_fmt_point(pt)).value, "(3, 4)");
  SP_EXPECT_STR_EQ_CSTR(sp_fmt(sp_mem_get_scratch(), "{:>8}", sp_fmt_point(pt)).value, "  (3, 4)");
}
