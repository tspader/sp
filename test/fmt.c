#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

UTEST(format, basic_integers) {
  sp_str_t result = sp_format("u32: {}, s32: {}", SP_FMT_U32(42), SP_FMT_S32(-100));
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("u32: 42, s32: -100")));
}

UTEST(format, basic_floats) {
  sp_str_t result = sp_format("pi: {}", SP_FMT_F32(3.14f));
  ASSERT_TRUE(sp_str_starts_with(result, SP_LIT("pi: 3.14")));
}

UTEST(format, strings) {
  sp_str_t msg = SP_LIT("hello world");
  sp_str_t result = sp_format("str: {}, cstr: {}", SP_FMT_STR(msg), SP_FMT_CSTR("test"));
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("str: hello world, cstr: test")));
}

UTEST(format, quoted_strings) {
  sp_str_t filename = SP_LIT("test.txt");
  sp_str_t result = sp_format("file: {}", SP_FMT_QUOTED_STR(filename));
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("file: \"test.txt\"")));
}

UTEST(format, pointers) {
  void* ptr = (void*)0xDEADBEEF;
  sp_str_t result = sp_format("ptr: {}", SP_FMT_PTR(ptr));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("0x")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("deadbeef")));
}

UTEST(format, unsigned_types) {
  sp_str_t result = sp_format("u8:{}, u16:{}, u32:{}, u64:{}",
    SP_FMT_U8(255),
    SP_FMT_U16(65535),
    SP_FMT_U32(4000000000),
    SP_FMT_U64(18446744073709551615ULL));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("u8:255")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("u16:65535")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("u32:4000000000")));
}

UTEST(format, signed_types) {
  sp_str_t result = sp_format("s8:{}, s16:{}, s32:{}, s64:{}",
    SP_FMT_S8(-128),
    SP_FMT_S16(-32768),
    SP_FMT_S32(-2147483648),
    SP_FMT_S64(-9223372036854775807LL));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("s8:-128")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("s16:-32768")));
}

UTEST(format, characters) {
  sp_str_t result = sp_format("char: {}", SP_FMT_C8('A'));
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("char: A")));
}

UTEST(format, mixed_types) {
  u32 count = 10;
  f32 percent = 75.5f;
  sp_str_t name = SP_LIT("test");

  sp_str_t result = sp_format("Processed {} items at {}% - {}",
    SP_FMT_U32(count),
    SP_FMT_F32(percent),
    SP_FMT_STR(name));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("Processed 10 items")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("75.5")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("test")));
}

UTEST(format, empty_format_string) {
  sp_str_t result = sp_format("");
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("")));
}

UTEST(format, no_placeholders) {
  sp_str_t result = sp_format("just plain text");
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("just plain text")));
}

UTEST(format, single_placeholder) {
  sp_str_t result = sp_format("{}", SP_FMT_U32(42));
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("42")));
}

UTEST(format, concatenation) {
  sp_str_t base = SP_LIT("/usr/local");
  sp_str_t file = SP_LIT("bin");
  sp_str_t result = sp_format("{}/{}", SP_FMT_STR(base), SP_FMT_STR(file));
  ASSERT_TRUE(sp_str_equal(result, SP_LIT("/usr/local/bin")));
}

UTEST(format, zero_values) {
  sp_str_t result = sp_format("u32:{}, s32:{}, f32:{}",
    SP_FMT_U32(0),
    SP_FMT_S32(0),
    SP_FMT_F32(0.0f));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("u32:0")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("s32:0")));
}

UTEST(format, hash_values) {
  sp_hash_t hash = sp_hash_cstr("test");
  sp_str_t result = sp_format("hash: {}", SP_FMT_HASH(hash));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("hash: ")));
  ASSERT_GT(result.len, 10);
}

UTEST(format, hash_short) {
  sp_hash_t hash = sp_hash_cstr("test");
  sp_str_t result = sp_format("short: {}", SP_FMT_SHORT_HASH(hash));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("short: ")));
  ASSERT_LT(result.len, 20);
}

UTEST_MAIN()
