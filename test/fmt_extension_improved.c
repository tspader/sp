// IMPROVED APPROACH: Minimal boilerplate using X macros
//
// This version uses X macros more cleverly to reduce the boilerplate
// from the previous example.

#include "sp.h"

// ============================================================================
// USER CODE: Define custom types
// ============================================================================

typedef struct { f32 x, y; } vec2_t;
typedef struct { u8 r, g, b, a; } rgba_t;

// ============================================================================
// USER CODE: Define format type list
// ============================================================================

#define MY_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t)

// ============================================================================
// LIBRARY CODE: Extended enum (built-in + custom)
// ============================================================================

typedef enum {
  #undef SP_FMT_X
  #define SP_FMT_X(id, type) SP_FMT_ID(id),
  SP_FORMAT_TYPES
  MY_FORMAT_TYPES
} my_format_id_t;

// ============================================================================
// LIBRARY CODE: Extended arg struct (built-in + custom)
// ============================================================================

typedef struct {
  my_format_id_t id;
  union {
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES
    MY_FORMAT_TYPES
  };
} my_format_arg_t;

// ============================================================================
// USER CODE: Implement custom format functions
// ============================================================================

void sp_fmt_format_vec2(sp_str_builder_t* builder, my_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_t result = sp_format("vec2({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

void sp_fmt_format_rgba(sp_str_builder_t* builder, my_format_arg_t* arg) {
  rgba_t c = arg->rgba_value;
  sp_str_t result = sp_format("rgba({}, {}, {}, {})",
    SP_FMT_U8(c.r), SP_FMT_U8(c.g), SP_FMT_U8(c.b), SP_FMT_U8(c.a));
  sp_str_builder_append(builder, result);
}

// ============================================================================
// LIBRARY CODE: Formatter table using X macros (AUTOMATIC!)
// ============================================================================

typedef void (*my_format_fn_t)(sp_str_builder_t*, my_format_arg_t*);

typedef struct {
  my_format_id_t id;
  my_format_fn_t fn;
} my_formatter_t;

static my_formatter_t my_formatters[] = {
  // Built-in formatters - automatically generated
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) \
    { SP_FMT_ID(ID), (my_format_fn_t)SP_FMT_FN(ID) },
  SP_FORMAT_TYPES

  // Custom formatters - automatically generated
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) \
    { SP_FMT_ID(ID), sp_fmt_format_##ID },
  MY_FORMAT_TYPES
};

// ============================================================================
// LIBRARY CODE: Extended format implementation
// ============================================================================

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

      my_format_arg_t arg = va_arg(args, my_format_arg_t);
      u32 start = builder.buffer.len;

      SP_CARR_FOR(my_formatters, i) {
        if (arg.id == my_formatters[i].id) {
          my_formatters[i].fn(&builder, &arg);
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

// ============================================================================
// LIBRARY CODE: Convenience macros (AUTOMATIC via X macro!)
// ============================================================================

#define MY_FMT_ARG(T, V) \
  (my_format_arg_t){ .id = SP_FMT_ID(T), .SP_FMT_UNION(T) = (V) }

// Auto-generate wrappers for built-in types
#undef SP_FMT_X
#define SP_FMT_X(ID, TYPE) \
  static inline my_format_arg_t my_fmt_##ID(TYPE v) { \
    return MY_FMT_ARG(ID, v); \
  }
SP_FORMAT_TYPES
#undef SP_FMT_X

// Auto-generate wrappers for custom types
#define SP_FMT_X(ID, TYPE) \
  static inline my_format_arg_t my_fmt_##ID(TYPE v) { \
    return MY_FMT_ARG(ID, v); \
  }
MY_FORMAT_TYPES
#undef SP_FMT_X

// ============================================================================
// Optional: Macro-style API (uppercase)
// ============================================================================

#define MY_FMT_VEC2(V)  my_fmt_vec2(V)
#define MY_FMT_RGBA(V)  my_fmt_rgba(V)
#define MY_FMT_U32(V)   my_fmt_u32(V)
#define MY_FMT_F32(V)   my_fmt_f32(V)
#define MY_FMT_STR(V)   my_fmt_str(V)
// ... etc

// ============================================================================
// USAGE EXAMPLE
// ============================================================================

#if 0
void demo() {
  vec2_t position = { 100.5f, 200.3f };
  rgba_t color = { 255, 128, 64, 255 };
  u32 health = 85;

  // Using function style
  sp_str_t msg1 = my_format("Player at {} with {} hp (color: {})",
    my_fmt_vec2(position),
    my_fmt_u32(health),
    my_fmt_rgba(color));

  // Using macro style
  sp_str_t msg2 = my_format("Player at {} with {} hp (color: {})",
    MY_FMT_VEC2(position),
    MY_FMT_U32(health),
    MY_FMT_RGBA(color));

  // Both work!
}
#endif

// ============================================================================
// SUMMARY: What the user needs to do
// ============================================================================
//
// 1. Define custom types (vec2_t, rgba_t, etc.)
// 2. Define MY_FORMAT_TYPES X macro listing them
// 3. Implement sp_fmt_format_<typename> functions
// 4. Copy the "LIBRARY CODE" sections (can be in a header)
// 5. Use my_format() with MY_FMT_*() or my_fmt_*()
//
// The X macros automatically:
// - Generate the enum with all types
// - Generate the union with all types
// - Generate the formatters array
// - Generate wrapper functions for all types
//
// ============================================================================
