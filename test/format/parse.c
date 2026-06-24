#include "format.h"

typedef enum {
  PARSE_U8,
  PARSE_U16,
  PARSE_U32,
  PARSE_U64,
  PARSE_S8,
  PARSE_S16,
  PARSE_S32,
  PARSE_S64,
  PARSE_F32,
  PARSE_HEX,
  PARSE_HASH,
  PARSE_BOOL,
  PARSE_C8,
  PARSE_C16,
} format_parse_kind_t;

typedef struct {
  format_parse_kind_t kind;
  const c8* input;
  u64 u;
  s64 i;
  f64 f;
} format_parse_test_t;

static void run_format_parse_test(s32* utest_result, format_parse_test_t t) {
  sp_str_t in = sp_str_view(t.input);
  switch (t.kind) {
    case PARSE_U8: EXPECT_EQ(sp_parse_u8(in), (u8)t.u); break;
    case PARSE_U16: EXPECT_EQ(sp_parse_u16(in), (u16)t.u); break;
    case PARSE_U32: EXPECT_EQ(sp_parse_u32(in), (u32)t.u); break;
    case PARSE_U64: EXPECT_EQ(sp_parse_u64(in), t.u); break;
    case PARSE_S8: EXPECT_EQ(sp_parse_s8(in), (s8)t.i); break;
    case PARSE_S16: EXPECT_EQ(sp_parse_s16(in), (s16)t.i); break;
    case PARSE_S32: EXPECT_EQ(sp_parse_s32(in), (s32)t.i); break;
    case PARSE_S64: EXPECT_EQ(sp_parse_s64(in), t.i); break;
    case PARSE_F32: EXPECT_NEAR(sp_parse_f32(in), (f32)t.f, 1e-5f); break;
    case PARSE_HEX: EXPECT_EQ(sp_parse_hex(in), t.u); break;
    case PARSE_HASH: EXPECT_EQ(sp_parse_hash(in), (sp_hash_t)t.u); break;
    case PARSE_BOOL: EXPECT_EQ(sp_parse_bool(in), (bool)t.u); break;
    case PARSE_C8: EXPECT_EQ(sp_parse_c8(in), (c8)t.u); break;
    case PARSE_C16: EXPECT_EQ(sp_parse_c16(in), (u16)t.u); break;
  }
}

typedef struct {
  format_parse_kind_t kind;
  const c8* input;
  bool ok;
  u64 u;
  s64 i;
  f64 f;
} format_parse_ex_test_t;

static void run_format_parse_ex_test(s32* utest_result, format_parse_ex_test_t t) {
  sp_str_t in = sp_str_view(t.input);
  switch (t.kind) {
    case PARSE_U8: { u8 v; EXPECT_EQ(sp_parse_u8_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (u8)t.u); break; }
    case PARSE_U16: { u16 v; EXPECT_EQ(sp_parse_u16_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (u16)t.u); break; }
    case PARSE_U32: { u32 v; EXPECT_EQ(sp_parse_u32_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (u32)t.u); break; }
    case PARSE_U64: { u64 v; EXPECT_EQ(sp_parse_u64_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, t.u); break; }
    case PARSE_S8: { s8 v; EXPECT_EQ(sp_parse_s8_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (s8)t.i); break; }
    case PARSE_S16: { s16 v; EXPECT_EQ(sp_parse_s16_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (s16)t.i); break; }
    case PARSE_S32: { s32 v; EXPECT_EQ(sp_parse_s32_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (s32)t.i); break; }
    case PARSE_S64: { s64 v; EXPECT_EQ(sp_parse_s64_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, t.i); break; }
    case PARSE_F32: { f32 v; EXPECT_EQ(sp_parse_f32_ex(in, &v), t.ok); if (t.ok) EXPECT_NEAR(v, (f32)t.f, 1e-5f); break; }
    case PARSE_HEX: { u64 v; EXPECT_EQ(sp_parse_hex_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, t.u); break; }
    case PARSE_HASH: { sp_hash_t v; EXPECT_EQ(sp_parse_hash_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (sp_hash_t)t.u); break; }
    case PARSE_BOOL: { bool v; EXPECT_EQ(sp_parse_bool_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (bool)t.u); break; }
    case PARSE_C8: { c8 v; EXPECT_EQ(sp_parse_c8_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (c8)t.u); break; }
    case PARSE_C16: { u16 v; EXPECT_EQ(sp_parse_c16_ex(in, &v), t.ok); if (t.ok) EXPECT_EQ(v, (u16)t.u); break; }
  }
}

UTEST(format_parse, unsigned_integers) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_U8, .input = "0", .u = 0 },
    { .kind = PARSE_U8, .input = "255", .u = 255 },
    { .kind = PARSE_U8, .input = "128", .u = 128 },
    { .kind = PARSE_U8, .input = "42", .u = 42 },
    { .kind = PARSE_U16, .input = "0", .u = 0 },
    { .kind = PARSE_U16, .input = "65535", .u = 65535 },
    { .kind = PARSE_U16, .input = "32768", .u = 32768 },
    { .kind = PARSE_U16, .input = "1234", .u = 1234 },
    { .kind = PARSE_U32, .input = "0", .u = 0 },
    { .kind = PARSE_U32, .input = "4294967295", .u = 4294967295U },
    { .kind = PARSE_U32, .input = "2147483648", .u = 2147483648U },
    { .kind = PARSE_U32, .input = "123456789", .u = 123456789U },
    { .kind = PARSE_U64, .input = "0", .u = 0ULL },
    { .kind = PARSE_U64, .input = "18446744073709551615", .u = 18446744073709551615ULL },
    { .kind = PARSE_U64, .input = "9223372036854775808", .u = 9223372036854775808ULL },
    { .kind = PARSE_U64, .input = "1234567890123", .u = 1234567890123ULL },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, signed_integers) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_S8, .input = "0", .i = 0 },
    { .kind = PARSE_S8, .input = "127", .i = 127 },
    { .kind = PARSE_S8, .input = "-128", .i = -128 },
    { .kind = PARSE_S8, .input = "-42", .i = -42 },
    { .kind = PARSE_S8, .input = "42", .i = 42 },
    { .kind = PARSE_S16, .input = "0", .i = 0 },
    { .kind = PARSE_S16, .input = "32767", .i = 32767 },
    { .kind = PARSE_S16, .input = "-32768", .i = -32768 },
    { .kind = PARSE_S16, .input = "-1234", .i = -1234 },
    { .kind = PARSE_S16, .input = "1234", .i = 1234 },
    { .kind = PARSE_S32, .input = "0", .i = 0 },
    { .kind = PARSE_S32, .input = "2147483647", .i = 2147483647 },
    { .kind = PARSE_S32, .input = "-2147483648", .i = INT32_MIN },
    { .kind = PARSE_S32, .input = "-123456789", .i = -123456789 },
    { .kind = PARSE_S32, .input = "123456789", .i = 123456789 },
    { .kind = PARSE_S64, .input = "0", .i = 0LL },
    { .kind = PARSE_S64, .input = "9223372036854775807", .i = 9223372036854775807LL },
    { .kind = PARSE_S64, .input = "-9223372036854775808", .i = INT64_MIN },
    { .kind = PARSE_S64, .input = "-1234567890123", .i = -1234567890123LL },
    { .kind = PARSE_S64, .input = "1234567890123", .i = 1234567890123LL },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, floating_point) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_F32, .input = "0", .f = 0.0 },
    { .kind = PARSE_F32, .input = "0.0", .f = 0.0 },
    { .kind = PARSE_F32, .input = "3.14159", .f = 3.14159 },
    { .kind = PARSE_F32, .input = "-3.14159",.f = -3.14159 },
    { .kind = PARSE_F32, .input = "1.23e2", .f = 123.0 },
    { .kind = PARSE_F32, .input = "1.23e-2", .f = 0.0123 },
    { .kind = PARSE_F32, .input = "-1.23e2", .f = -123.0 },
    { .kind = PARSE_F32, .input = "42", .f = 42.0 },
    { .kind = PARSE_F32, .input = "-42", .f = -42.0 },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, hex) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_HEX, .input = "0", .u = 0ULL },
    { .kind = PARSE_HEX, .input = "F", .u = 0xFULL },
    { .kind = PARSE_HEX, .input = "f", .u = 0xfULL },
    { .kind = PARSE_HEX, .input = "FF", .u = 0xFFULL },
    { .kind = PARSE_HEX, .input = "ff", .u = 0xffULL },
    { .kind = PARSE_HEX, .input = "DEADBEEF", .u = 0xDEADBEEFULL },
    { .kind = PARSE_HEX, .input = "deadbeef", .u = 0xdeadbeefULL },
    { .kind = PARSE_HEX, .input = "123ABC", .u = 0x123ABCULL },
    { .kind = PARSE_HEX, .input = "FFFFFFFFFFFFFFFF", .u = 0xFFFFFFFFFFFFFFFFULL },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, hash) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_HASH, .input = "0", .u = 0U },
    { .kind = PARSE_HASH, .input = "FFFFFFFF", .u = 0xFFFFFFFFU },
    { .kind = PARSE_HASH, .input = "12345678", .u = 0x12345678U },
    { .kind = PARSE_HASH, .input = "DEADBEEF", .u = 0xDEADBEEFU },
    { .kind = PARSE_HASH, .input = "deadbeef", .u = 0xdeadbeefU },
    { .kind = PARSE_HASH, .input = "ABCD", .u = 0xABCDU },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, boolean) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_BOOL, .input = "true", .u = true },
    { .kind = PARSE_BOOL, .input = "false", .u = false },
    { .kind = PARSE_BOOL, .input = "1", .u = true },
    { .kind = PARSE_BOOL, .input = "0", .u = false },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, characters) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_C8, .input = "'A'", .u = 'A' },
    { .kind = PARSE_C8, .input = "'z'", .u = 'z' },
    { .kind = PARSE_C8, .input = "'0'", .u = '0' },
    { .kind = PARSE_C8, .input = "' '", .u = ' ' },
    { .kind = PARSE_C8, .input = "'!'", .u = '!' },
    { .kind = PARSE_C16, .input = "'A'", .u = L'A' },
    { .kind = PARSE_C16, .input = "'z'", .u = L'z' },
    { .kind = PARSE_C16, .input = "'0'", .u = L'0' },
    { .kind = PARSE_C16, .input = "' '", .u = L' ' },
    { .kind = PARSE_C16, .input = "'!'", .u = L'!' },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, lenient) {
  format_parse_test_t cases[] = {
    { .kind = PARSE_U32, .input = "00042", .u = 42U },
    { .kind = PARSE_S32, .input = "-00042", .i = -42 },
    { .kind = PARSE_F32, .input = "003.14", .f = 3.14 },
    { .kind = PARSE_S32, .input = "+42", .i = 42 },
    { .kind = PARSE_F32, .input = "+3.14", .f = 3.14 },
    { .kind = PARSE_HEX, .input = "DeAdBeEf", .u = 0xdeadbeefULL },
  };
  SP_CARR_FOR(cases, i) run_format_parse_test(utest_result, cases[i]);
}

UTEST(format_parse, extended) {
  format_parse_ex_test_t cases[] = {
    { .kind = PARSE_U8, .input = "255", .ok = true, .u = 255 },
    { .kind = PARSE_U8, .input = "256", .ok = false },
    { .kind = PARSE_U16, .input = "65535", .ok = true, .u = 65535 },
    { .kind = PARSE_U16, .input = "65536", .ok = false },
    { .kind = PARSE_U32, .input = "42", .ok = true, .u = 42U },
    { .kind = PARSE_U32, .input = "4294967296", .ok = false },
    { .kind = PARSE_U32, .input = "-1", .ok = false },
    { .kind = PARSE_U32, .input = "abc", .ok = false },
    { .kind = PARSE_U32, .input = "", .ok = false },
    { .kind = PARSE_U64, .input = "18446744073709551615", .ok = true, .u = 18446744073709551615ULL },
    { .kind = PARSE_U64, .input = "not_a_number", .ok = false },
    { .kind = PARSE_S8, .input = "-128", .ok = true, .i = -128 },
    { .kind = PARSE_S8, .input = "-129", .ok = false },
    { .kind = PARSE_S16, .input = "32767", .ok = true, .i = 32767 },
    { .kind = PARSE_S16, .input = "32768", .ok = false },
    { .kind = PARSE_S32, .input = "42", .ok = true, .i = 42 },
    { .kind = PARSE_S32, .input = "2147483648", .ok = false },
    { .kind = PARSE_S32, .input = "-2147483649", .ok = false },
    { .kind = PARSE_S32, .input = "text", .ok = false },
    { .kind = PARSE_S32, .input = "", .ok = false },
    { .kind = PARSE_S64, .input = "9223372036854775807", .ok = true, .i = 9223372036854775807LL },
    { .kind = PARSE_S64, .input = "invalid", .ok = false },
    { .kind = PARSE_F32, .input = "3.14", .ok = true, .f = 3.14 },
    { .kind = PARSE_F32, .input = "abc", .ok = false },
    { .kind = PARSE_F32, .input = "", .ok = false },
    { .kind = PARSE_BOOL, .input = "true", .ok = true, .u = true },
    { .kind = PARSE_BOOL, .input = "maybe", .ok = false },
    { .kind = PARSE_BOOL, .input = "", .ok = false },
    { .kind = PARSE_HEX, .input = "DEADBEEF", .ok = true, .u = 0xDEADBEEFULL },
    { .kind = PARSE_HEX, .input = "XYZ", .ok = false },
    { .kind = PARSE_HEX, .input = "", .ok = false },
    { .kind = PARSE_HASH, .input = "DEADBEEF", .ok = true, .u = 0xDEADBEEF },
    { .kind = PARSE_HASH, .input = "GHIJKLMN", .ok = false },
    { .kind = PARSE_HASH, .input = "", .ok = false },
    { .kind = PARSE_C8, .input = "'A'", .ok = true, .u = 'A' },
    { .kind = PARSE_C8, .input = "AB", .ok = false },
    { .kind = PARSE_C8, .input = "", .ok = false },
    { .kind = PARSE_C16, .input = "'Z'", .ok = true, .u = L'Z' },
    { .kind = PARSE_C16, .input = "XY", .ok = false },
    { .kind = PARSE_C16, .input = "", .ok = false },
  };
  SP_CARR_FOR(cases, i) run_format_parse_ex_test(utest_result, cases[i]);
}
