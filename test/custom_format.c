// WORKING EXAMPLE: Custom format types integrated into sp.h
//
// Compile: gcc -I. -Itest -Iexternal/utest.h -Wall -lpthread -lm test/custom_format.c -std=c99 -o build/debug/custom_format -DSP_IMPLEMENTATION
// Run: ./build/debug/custom_format

// STEP 1: Define your custom types
typedef struct { float x, y; } vec2_t;
typedef struct { unsigned char r, g, b, a; } rgba_t;

// STEP 2: Define format types BEFORE including sp.h
#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t)

// STEP 3: Include sp.h (gets extended enum/union/forward decls)
#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"
#include "utest.h"

// STEP 4: Implement your format functions (can use sp_format!)
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  // Using sp_format recursively!
  sp_str_t result = sp_format("({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

void sp_fmt_format_rgba(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  rgba_t c = arg->rgba_value;
  c8 buf[16];
  snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", c.r, c.g, c.b, c.a);
  sp_str_builder_append_cstr(builder, buf);
}

// STEP 5: Create wrapper macros for ergonomics
#define SP_FMT_VEC2(V) SP_FMT_ARG(vec2, V)
#define SP_FMT_RGBA(V) SP_FMT_ARG(rgba, V)

// TESTS
UTEST(custom_format, vec2_basic) {
  vec2_t pos = { 10.5f, 20.25f };
  sp_str_t result = sp_format("Position: {}", SP_FMT_VEC2(pos));

  ASSERT_TRUE(sp_str_starts_with(result, SP_LIT("Position: (")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("10.5")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("20.25")));
}

UTEST(custom_format, rgba_basic) {
  rgba_t color = { 255, 128, 64, 255 };
  sp_str_t result = sp_format("Color: {}", SP_FMT_RGBA(color));

  ASSERT_TRUE(sp_str_equal(result, SP_LIT("Color: #ff8040ff")));
}

UTEST(custom_format, mix_builtin_and_custom) {
  vec2_t pos = { 100.0f, 200.0f };
  rgba_t color = { 255, 0, 0, 255 };
  u32 count = 42;

  sp_str_t result = sp_format("Spawn {} units at {} with color {}",
    SP_FMT_U32(count),
    SP_FMT_VEC2(pos),
    SP_FMT_RGBA(color));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("Spawn 42")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("(100")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("#ff0000ff")));
}

UTEST(custom_format, recursive_sp_format_in_custom_formatter) {
  // This tests that custom formatters can call sp_format
  vec2_t v = { 3.14f, 2.71f };
  sp_str_t result = sp_format("Vector: {}", SP_FMT_VEC2(v));

  // The vec2 formatter internally uses sp_format with SP_FMT_F32
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("Vector: (")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("3.14")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("2.71")));
}

UTEST(custom_format, game_scenario) {
  vec2_t player_pos = { 123.4f, 567.8f };
  rgba_t damage_color = { 255, 64, 64, 200 };
  sp_str_t player_name = SP_LIT("Hero");
  u32 health = 75;

  sp_str_t log = sp_format(
    "[Game] {} (hp:{}) at {} damage:{}",
    SP_FMT_STR(player_name),
    SP_FMT_U32(health),
    SP_FMT_VEC2(player_pos),
    SP_FMT_RGBA(damage_color)
  );

  ASSERT_TRUE(sp_str_contains(log, SP_LIT("[Game]")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("Hero")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("hp:75")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("(123.4")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("#ff4040c8")));
}

UTEST(custom_format, builtin_types_still_work) {
  // Verify we didn't break built-in types
  sp_str_t test_str = SP_LIT("test");
  sp_str_t result = sp_format("s32:{} u32:{} f32:{} str:{}",
    SP_FMT_S32(-42),
    SP_FMT_U32(100),
    SP_FMT_F32(3.14f),
    SP_FMT_STR(test_str));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("s32:-42")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("u32:100")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("f32:3.14")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("str:test")));
}

UTEST_MAIN()
