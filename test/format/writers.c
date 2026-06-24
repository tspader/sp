#include "format.h"

typedef enum {
  WRITE_U64,
  WRITE_S64,
  WRITE_F64,
  WRITE_PTR,
  WRITE_BOOL,
  WRITE_SIZE,
  WRITE_DURATION,
  WRITE_ORDINAL,
} write_kind_t;

typedef struct {
  write_kind_t kind;
  const c8* expect;
  union {
    struct { u64 value; sp_fmt_radix_t radix; } u;
    struct { s64 value; } s;
    struct { f64 value; u32 precision; } f;
    struct { void* value; } p;
    struct { bool value; } b;
    struct { u64 value; } size;
    struct { u64 value; } duration;
    struct { s64 value; } ordinal;
  };
} write_test_t;

static void run_write_test(s32* utest_result, write_test_t t) {
  c8 buf[128];
  sp_io_mem_writer_t io = sp_zero;
  sp_io_mem_writer_from_buffer(&io, buf, sizeof(buf));

  switch (t.kind) {
    case WRITE_U64:      sp_fmt_write_u64_ex(&io.base, t.u.value, t.u.radix); break;
    case WRITE_S64:      sp_fmt_write_s64(&io.base, t.s.value); break;
    case WRITE_F64:      sp_fmt_write_f64_ex(&io.base, t.f.value, t.f.precision); break;
    case WRITE_PTR:      sp_fmt_write_ptr(&io.base, t.p.value); break;
    case WRITE_BOOL:     sp_fmt_write_bool(&io.base, t.b.value); break;
    case WRITE_SIZE:     sp_fmt_write_size(&io.base, t.size.value); break;
    case WRITE_DURATION: sp_fmt_write_duration(&io.base, t.duration.value); break;
    case WRITE_ORDINAL:  sp_fmt_write_ordinal(&io.base, t.ordinal.value); break;
  }

  sp_str_t got = sp_io_mem_writer_as_str(&io);
  EXPECT_TRUE(sp_str_equal_cstr(got, t.expect));
}

UTEST(format_write, primitives) {
  write_test_t cases [] = {
    { .kind = WRITE_U64, .expect = "0", .u = { .value = 0, .radix = SP_FMT_RADIX_DECIMAL } },
    { .kind = WRITE_U64, .expect = "7", .u = { .value = 7, .radix = SP_FMT_RADIX_DECIMAL } },
    { .kind = WRITE_U64, .expect = "99", .u = { .value = 99, .radix = SP_FMT_RADIX_DECIMAL } },
    { .kind = WRITE_U64, .expect = "100", .u = { .value = 100, .radix = SP_FMT_RADIX_DECIMAL } },
    { .kind = WRITE_U64, .expect = "18446744073709551615", .u = { .value = 18446744073709551615ULL, .radix = SP_FMT_RADIX_DECIMAL } },
    { .kind = WRITE_U64, .expect = "0", .u = { .value = 0, .radix = SP_FMT_RADIX_HEX } },
    { .kind = WRITE_U64, .expect = "ff", .u = { .value = 255, .radix = SP_FMT_RADIX_HEX } },
    { .kind = WRITE_U64, .expect = "deadbeef", .u = { .value = 0xdeadbeef, .radix = SP_FMT_RADIX_HEX } },
    { .kind = WRITE_U64, .expect = "FF", .u = { .value = 255, .radix = SP_FMT_RADIX_HEX_UPPER } },
    { .kind = WRITE_U64, .expect = "DEADBEEF", .u = { .value = 0xdeadbeef, .radix = SP_FMT_RADIX_HEX_UPPER } },
    { .kind = WRITE_U64, .expect = "0", .u = { .value = 0, .radix = SP_FMT_RADIX_BINARY } },
    { .kind = WRITE_U64, .expect = "101", .u = { .value = 5, .radix = SP_FMT_RADIX_BINARY } },
    { .kind = WRITE_U64, .expect = "11111111", .u = { .value = 255, .radix = SP_FMT_RADIX_BINARY } },
    { .kind = WRITE_U64, .expect = "0", .u = { .value = 0, .radix = SP_FMT_RADIX_OCTAL } },
    { .kind = WRITE_U64, .expect = "10", .u = { .value = 8, .radix = SP_FMT_RADIX_OCTAL } },
    { .kind = WRITE_U64, .expect = "777", .u = { .value = 511, .radix = SP_FMT_RADIX_OCTAL } },
    { .kind = WRITE_S64, .expect = "0", .s = { .value = 0 } },
    { .kind = WRITE_S64, .expect = "42", .s = { .value = 42 } },
    { .kind = WRITE_S64, .expect = "-42", .s = { .value = -42 } },
    { .kind = WRITE_S64, .expect = "9223372036854775807", .s = { .value = 9223372036854775807LL } },
    { .kind = WRITE_S64, .expect = "-9223372036854775808", .s = { .value = (-9223372036854775807LL - 1) } },
    { .kind = WRITE_F64, .expect = "0.00", .f = { .value = 0.0, .precision = 2 } },
    { .kind = WRITE_F64, .expect = "1", .f = { .value = 1.0, .precision = 0 } },
    { .kind = WRITE_F64, .expect = "0.5", .f = { .value = 0.5, .precision = 1 } },
    { .kind = WRITE_F64, .expect = "2.5", .f = { .value = 2.5, .precision = 1 } },
    { .kind = WRITE_F64, .expect = "-2.5", .f = { .value = -2.5, .precision = 1 } },
    { .kind = WRITE_F64, .expect = "3.14", .f = { .value = 3.14159, .precision = 2 } },
    { .kind = WRITE_F64, .expect = "3.1416", .f = { .value = 3.14159, .precision = 4 } },
    { .kind = WRITE_F64, .expect = "inf", .f = { .value = 2.0e19, .precision = 2 } },
    { .kind = WRITE_PTR, .expect = "0x0", .p = { .value = (void*)0 } },
    { .kind = WRITE_PTR, .expect = "0xdeadbeef", .p = { .value = (void*)(uintptr_t)0xdeadbeef } },
    { .kind = WRITE_BOOL, .expect = "true", .b = { .value = true } },
    { .kind = WRITE_BOOL, .expect = "false", .b = { .value = false } },
    { .kind = WRITE_SIZE, .expect = "0 B", .size = { .value = 0 } },
    { .kind = WRITE_SIZE, .expect = "512 B", .size = { .value = 512 } },
    { .kind = WRITE_SIZE, .expect = "1 KB", .size = { .value = 1024 } },
    { .kind = WRITE_SIZE, .expect = "1 KB", .size = { .value = 1124 } },
    { .kind = WRITE_SIZE, .expect = "1.5 KB", .size = { .value = 1536 } },
    { .kind = WRITE_SIZE, .expect = "1 MB", .size = { .value = 1048576 } },
    { .kind = WRITE_SIZE, .expect = "1024 PB", .size = { .value = 1152921504606846976ULL } },
    { .kind = WRITE_DURATION, .expect = "0 ns", .duration = { .value = 0 } },
    { .kind = WRITE_DURATION, .expect = "999 ns", .duration = { .value = 999 } },
    { .kind = WRITE_DURATION, .expect = "1 us", .duration = { .value = 1000 } },
    { .kind = WRITE_DURATION, .expect = "1 us", .duration = { .value = 1099 } },
    { .kind = WRITE_DURATION, .expect = "1.5 us", .duration = { .value = 1500 } },
    { .kind = WRITE_DURATION, .expect = "1 ms", .duration = { .value = 1000000 } },
    { .kind = WRITE_DURATION, .expect = "1 s", .duration = { .value = 1000000000 } },
    { .kind = WRITE_DURATION, .expect = "1000 s", .duration = { .value = 1000000000000ULL } },
    { .kind = WRITE_ORDINAL, .expect = "0th", .ordinal = { .value = 0 } },
    { .kind = WRITE_ORDINAL, .expect = "1st", .ordinal = { .value = 1 } },
    { .kind = WRITE_ORDINAL, .expect = "2nd", .ordinal = { .value = 2 } },
    { .kind = WRITE_ORDINAL, .expect = "3rd", .ordinal = { .value = 3 } },
    { .kind = WRITE_ORDINAL, .expect = "4th", .ordinal = { .value = 4 } },
    { .kind = WRITE_ORDINAL, .expect = "5th", .ordinal = { .value = 5 } },
    { .kind = WRITE_ORDINAL, .expect = "6th", .ordinal = { .value = 6 } },
    { .kind = WRITE_ORDINAL, .expect = "7th", .ordinal = { .value = 7 } },
    { .kind = WRITE_ORDINAL, .expect = "8th", .ordinal = { .value = 8 } },
    { .kind = WRITE_ORDINAL, .expect = "9th", .ordinal = { .value = 9 } },
    { .kind = WRITE_ORDINAL, .expect = "10th", .ordinal = { .value = 10 } },
    { .kind = WRITE_ORDINAL, .expect = "11th", .ordinal = { .value = 11 } },
    { .kind = WRITE_ORDINAL, .expect = "12th", .ordinal = { .value = 12 } },
    { .kind = WRITE_ORDINAL, .expect = "13th", .ordinal = { .value = 13 } },
    { .kind = WRITE_ORDINAL, .expect = "14th", .ordinal = { .value = 14 } },
    { .kind = WRITE_ORDINAL, .expect = "15th", .ordinal = { .value = 15 } },
    { .kind = WRITE_ORDINAL, .expect = "16th", .ordinal = { .value = 16 } },
    { .kind = WRITE_ORDINAL, .expect = "17th", .ordinal = { .value = 17 } },
    { .kind = WRITE_ORDINAL, .expect = "18th", .ordinal = { .value = 18 } },
    { .kind = WRITE_ORDINAL, .expect = "19th", .ordinal = { .value = 19 } },
    { .kind = WRITE_ORDINAL, .expect = "20th", .ordinal = { .value = 20 } },
    { .kind = WRITE_ORDINAL, .expect = "21st", .ordinal = { .value = 21 } },
    { .kind = WRITE_ORDINAL, .expect = "22nd", .ordinal = { .value = 22 } },
    { .kind = WRITE_ORDINAL, .expect = "23rd", .ordinal = { .value = 23 } },
    { .kind = WRITE_ORDINAL, .expect = "24th", .ordinal = { .value = 24 } },
    { .kind = WRITE_ORDINAL, .expect = "25th", .ordinal = { .value = 25 } },
    { .kind = WRITE_ORDINAL, .expect = "26th", .ordinal = { .value = 26 } },
    { .kind = WRITE_ORDINAL, .expect = "27th", .ordinal = { .value = 27 } },
    { .kind = WRITE_ORDINAL, .expect = "28th", .ordinal = { .value = 28 } },
    { .kind = WRITE_ORDINAL, .expect = "29th", .ordinal = { .value = 29 } },
    { .kind = WRITE_ORDINAL, .expect = "111th", .ordinal = { .value = 111 } },
    { .kind = WRITE_ORDINAL, .expect = "-1st", .ordinal = { .value = -1 } },
  };

  sp_carr_for(cases, it) run_write_test(utest_result, cases[it]);
}

UTEST(format_write, destinations) {
  sp_str_r m = sp_fmt_write_u64_mem(sp_mem_get_scratch(), 255, SP_FMT_RADIX_HEX);
  EXPECT_EQ(m.err, SP_OK);
  SP_EXPECT_STR_EQ_CSTR(m.value, "ff");

  c8 buf[16];
  sp_str_r b = sp_fmt_write_u64_buf(buf, sizeof(buf), 255, SP_FMT_RADIX_HEX);
  EXPECT_EQ(b.err, SP_OK);
  SP_EXPECT_STR_EQ_CSTR(b.value, "ff");
}
