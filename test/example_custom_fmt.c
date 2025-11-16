// COMPLETE WORKING EXAMPLE: Custom format types
//
// Compile: gcc -I. -Itest -Iexternal/utest.h -Wall -lpthread -lm test/example_custom_fmt.c -std=c99 -o build/debug/example_custom_fmt -DSP_IMPLEMENTATION
// Run: ./build/debug/example_custom_fmt

#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"
#include "utest.h"

// ==================================================================
// STEP 1: Define your custom types
// ==================================================================

typedef struct {
  f32 x, y;
} vec2_t;

typedef struct {
  u8 r, g, b, a;
} rgba_t;

// ==================================================================
// STEP 2: Create extended format enum (built-in + custom)
// ==================================================================

typedef enum {
  // Built-in types (same order as sp.h)
  my_fmt_ptr,
  my_fmt_str,
  my_fmt_cstr,
  my_fmt_s8,
  my_fmt_s16,
  my_fmt_s32,
  my_fmt_s64,
  my_fmt_u8,
  my_fmt_u16,
  my_fmt_u32,
  my_fmt_u64,
  my_fmt_f32,
  my_fmt_f64,
  my_fmt_c8,
  my_fmt_c16,
  my_fmt_context,
  my_fmt_hash,
  my_fmt_hash_short,
  my_fmt_str_builder,
  my_fmt_fixed_array,
  my_fmt_quoted_str,
  my_fmt_color,

  // Custom types
  my_fmt_vec2,
  my_fmt_rgba,
} my_fmt_id_t;

// ==================================================================
// STEP 3: Create extended format arg (built-in + custom)
// ==================================================================

typedef struct {
  my_fmt_id_t id;
  union {
    // Built-in
    void* ptr_value;
    sp_str_t str_value;
    const c8* cstr_value;
    s8 s8_value;
    s16 s16_value;
    s32 s32_value;
    s64 s64_value;
    u8 u8_value;
    u16 u16_value;
    u32 u32_value;
    u64 u64_value;
    f32 f32_value;
    f64 f64_value;
    c8 c8_value;
    c16 c16_value;
    sp_context_t* context_value;
    sp_hash_t hash_value;
    sp_str_builder_t str_builder_value;
    sp_fixed_array_t fixed_array_value;
    const c8* color_value;

    // Custom
    vec2_t vec2_value;
    rgba_t rgba_value;
  };
} my_fmt_arg_t;

// ==================================================================
// STEP 4: Implement custom formatters
// ==================================================================

void my_fmt_format_vec2(sp_str_builder_t* b, my_fmt_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_t s = sp_format("({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(b, s);
}

void my_fmt_format_rgba(sp_str_builder_t* b, my_fmt_arg_t* arg) {
  rgba_t c = arg->rgba_value;
  // Manual hex formatting since sp_format doesn't support :02x
  c8 buf[16];
  snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", c.r, c.g, c.b, c.a);
  sp_str_builder_append_cstr(b, buf);
}

// ==================================================================
// STEP 5: Create formatter table
// ==================================================================

typedef void (*my_fmt_fn_t)(sp_str_builder_t*, my_fmt_arg_t*);

typedef struct {
  my_fmt_id_t id;
  my_fmt_fn_t fn;
} my_formatter_t;

static my_formatter_t formatters[] = {
  // Built-in
  { my_fmt_ptr, (my_fmt_fn_t)sp_fmt_format_ptr },
  { my_fmt_str, (my_fmt_fn_t)sp_fmt_format_str },
  { my_fmt_cstr, (my_fmt_fn_t)sp_fmt_format_cstr },
  { my_fmt_s8, (my_fmt_fn_t)sp_fmt_format_s8 },
  { my_fmt_s16, (my_fmt_fn_t)sp_fmt_format_s16 },
  { my_fmt_s32, (my_fmt_fn_t)sp_fmt_format_s32 },
  { my_fmt_s64, (my_fmt_fn_t)sp_fmt_format_s64 },
  { my_fmt_u8, (my_fmt_fn_t)sp_fmt_format_u8 },
  { my_fmt_u16, (my_fmt_fn_t)sp_fmt_format_u16 },
  { my_fmt_u32, (my_fmt_fn_t)sp_fmt_format_u32 },
  { my_fmt_u64, (my_fmt_fn_t)sp_fmt_format_u64 },
  { my_fmt_f32, (my_fmt_fn_t)sp_fmt_format_f32 },
  { my_fmt_f64, (my_fmt_fn_t)sp_fmt_format_f64 },
  { my_fmt_c8, (my_fmt_fn_t)sp_fmt_format_c8 },
  { my_fmt_c16, (my_fmt_fn_t)sp_fmt_format_c16 },
  { my_fmt_context, (my_fmt_fn_t)sp_fmt_format_context },
  { my_fmt_hash, (my_fmt_fn_t)sp_fmt_format_hash },
  { my_fmt_hash_short, (my_fmt_fn_t)sp_fmt_format_hash_short },
  { my_fmt_str_builder, (my_fmt_fn_t)sp_fmt_format_str_builder },
  { my_fmt_fixed_array, (my_fmt_fn_t)sp_fmt_format_fixed_array },
  { my_fmt_quoted_str, (my_fmt_fn_t)sp_fmt_format_quoted_str },
  { my_fmt_color, (my_fmt_fn_t)sp_fmt_format_color },

  // Custom
  { my_fmt_vec2, my_fmt_format_vec2 },
  { my_fmt_rgba, my_fmt_format_rgba },
};

// ==================================================================
// STEP 6: Implement format function
// ==================================================================

sp_str_t my_format_v(sp_str_t fmt, va_list args) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_format_parser_t parser = { .fmt = fmt };

  while (!sp_format_parser_is_done(&parser)) {
    c8 c = sp_format_parser_peek(&parser);

    if (c == '{') {
      sp_format_parser_eat(&parser);
      sp_format_specifier_t spec = sp_format_parser_specifier(&parser);

      if (spec.flags & SP_FORMAT_SPECIFIER_FLAG_FG_COLOR) {
        sp_str_builder_append(&builder, spec.color);
      }

      sp_format_parser_eat_and_assert(&parser, '}');

      my_fmt_arg_t arg = va_arg(args, my_fmt_arg_t);
      u32 start = builder.buffer.len;

      SP_CARR_FOR(formatters, i) {
        if (arg.id == formatters[i].id) {
          formatters[i].fn(&builder, &arg);
          break;
        }
      }

      sp_str_t formatted = sp_str(
        builder.buffer.data + start,
        builder.buffer.len - start
      );

      if (spec.flags & SP_FORMAT_SPECIFIER_FLAG_PAD) {
        if (formatted.len < spec.pad) {
          u32 pad = spec.pad - formatted.len;
          for (u32 i = 0; i < pad; i++) {
            sp_str_builder_append_c8(&builder, ' ');
          }
        }
      }

      if (spec.flags & SP_FORMAT_SPECIFIER_FLAG_FG_COLOR) {
        sp_str_builder_append_cstr(&builder, SP_ANSI_RESET);
      }
    } else {
      sp_str_builder_append_c8(&builder, c);
      sp_format_parser_eat(&parser);
    }
  }

  return sp_str_builder_write(&builder);
}

sp_str_t my_format(const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t result = my_format_v(SP_CSTR(fmt), args);
  va_end(args);
  return result;
}

// ==================================================================
// STEP 7: Create wrapper macros for ergonomics
// ==================================================================

// Direct constructors for each type
#define MY_VEC2(V)  ((my_fmt_arg_t){my_fmt_vec2, .vec2_value = (V)})
#define MY_RGBA(V)  ((my_fmt_arg_t){my_fmt_rgba, .rgba_value = (V)})
#define MY_STR(V)   ((my_fmt_arg_t){my_fmt_str, .str_value = (V)})
#define MY_CSTR(V)  ((my_fmt_arg_t){my_fmt_cstr, .cstr_value = (V)})
#define MY_U32(V)   ((my_fmt_arg_t){my_fmt_u32, .u32_value = (V)})
#define MY_S32(V)   ((my_fmt_arg_t){my_fmt_s32, .s32_value = (V)})
#define MY_F32(V)   ((my_fmt_arg_t){my_fmt_f32, .f32_value = (V)})
#define MY_PTR(V)   ((my_fmt_arg_t){my_fmt_ptr, .ptr_value = (V)})
#define MY_C8(V)    ((my_fmt_arg_t){my_fmt_c8, .c8_value = (V)})
#define MY_QSTR(V)  ((my_fmt_arg_t){my_fmt_quoted_str, .quoted_str_value = (V)})

// ==================================================================
// TESTS
// ==================================================================

UTEST(custom_format, vec2_basic) {
  vec2_t pos = { 10.5f, 20.25f };
  sp_str_t result = my_format("pos: {}", MY_VEC2(pos));

  ASSERT_TRUE(sp_str_starts_with(result, SP_LIT("pos: (")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("10.5")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("20.25")));
}

UTEST(custom_format, rgba_basic) {
  rgba_t color = { 255, 128, 64, 255 };
  sp_str_t result = my_format("color: {}", MY_RGBA(color));

  ASSERT_TRUE(sp_str_equal(result, SP_LIT("color: #ff8040ff")));
}

UTEST(custom_format, mixed_types) {
  vec2_t pos = { 100.0f, 200.0f };
  rgba_t color = { 255, 0, 0, 255 };
  u32 count = 42;

  sp_str_t result = my_format("Spawn {} at {} with color {}",
    MY_U32(count),
    MY_VEC2(pos),
    MY_RGBA(color));

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("Spawn 42")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("(100")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("#ff0000ff")));
}

UTEST(custom_format, all_builtin_types_work) {
  sp_str_t result = my_format(
    "s32:{} u32:{} f32:{} str:{} cstr:{} c8:{}",
    MY_S32(-42),
    MY_U32(100),
    MY_F32(3.14f),
    MY_STR(SP_LIT("hello")),
    MY_CSTR("world"),
    MY_C8('A')
  );

  ASSERT_TRUE(sp_str_contains(result, SP_LIT("s32:-42")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("u32:100")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("f32:3.14")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("str:hello")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("cstr:world")));
  ASSERT_TRUE(sp_str_contains(result, SP_LIT("c8:A")));
}

UTEST(custom_format, game_example) {
  vec2_t player_pos = { 123.4f, 567.8f };
  vec2_t target_pos = { 150.0f, 600.0f };
  rgba_t damage_color = { 255, 64, 64, 200 };
  sp_str_t player_name = SP_LIT("Hero");
  u32 health = 75;

  sp_str_t log = my_format(
    "[Game] {} (hp:{}) at {} moving to {}, damage: {}",
    MY_STR(player_name),
    MY_U32(health),
    MY_VEC2(player_pos),
    MY_VEC2(target_pos),
    MY_RGBA(damage_color)
  );

  ASSERT_TRUE(sp_str_contains(log, SP_LIT("[Game]")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("Hero")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("hp:75")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("(123.4")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("(150")));
  ASSERT_TRUE(sp_str_contains(log, SP_LIT("#ff4040c8")));
}

UTEST(custom_format, empty_and_plain) {
  ASSERT_TRUE(sp_str_equal(my_format(""), SP_LIT("")));
  ASSERT_TRUE(sp_str_equal(my_format("plain text"), SP_LIT("plain text")));
}

UTEST_MAIN()
