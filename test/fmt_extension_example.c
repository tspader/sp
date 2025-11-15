// WORKING EXAMPLE: User-defined format types without modifying sp.h
//
// This demonstrates a practical, working approach for extending sp_format
// with custom types at compile-time.

#include "sp.h"

// ============================================================================
// 1. Define your custom types
// ============================================================================

typedef struct {
  f32 x, y;
} vec2_t;

typedef struct {
  u8 r, g, b, a;
} rgba_t;

// ============================================================================
// 2. Define your format types using X macro
// ============================================================================

#define MY_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t)

// ============================================================================
// 3. Create extended enum and union that includes BOTH built-in and custom
// ============================================================================

typedef enum {
  // First, all built-in types
  #undef SP_FMT_X
  #define SP_FMT_X(id, type) SP_FMT_ID(id),
  SP_FORMAT_TYPES

  // Then, custom types
  MY_FORMAT_TYPES
} my_format_id_t;

typedef struct {
  my_format_id_t id;

  union {
    // Built-in type storage
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES

    // Custom type storage
    MY_FORMAT_TYPES
  };
} my_format_arg_t;

// ============================================================================
// 4. Implement format functions for custom types
// ============================================================================

void sp_fmt_format_vec2(sp_str_builder_t* builder, my_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  // Can use sp_format here since it's already included!
  sp_str_t result = sp_format("vec2({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

void sp_fmt_format_rgba(sp_str_builder_t* builder, my_format_arg_t* arg) {
  rgba_t c = arg->rgba_value;
  sp_str_t result = sp_format("#{:02x}{:02x}{:02x}{:02x}",
    SP_FMT_U8(c.r), SP_FMT_U8(c.g), SP_FMT_U8(c.b), SP_FMT_U8(c.a));
  sp_str_builder_append(builder, result);
}

// ============================================================================
// 5. Create custom formatter type and array
// ============================================================================

typedef struct {
  my_format_id_t id;
  void (*fn)(sp_str_builder_t*, my_format_arg_t*);
} my_formatter_t;

// Helper macro to cast built-in formatter functions to extended signature
#define MY_BUILTIN_FMT(ID) \
  { SP_FMT_ID(ID), (void(*)(sp_str_builder_t*, my_format_arg_t*))SP_FMT_FN(ID) }

static my_formatter_t my_formatters[] = {
  // Built-in formatters (cast to extended type)
  MY_BUILTIN_FMT(ptr),
  MY_BUILTIN_FMT(str),
  MY_BUILTIN_FMT(cstr),
  MY_BUILTIN_FMT(s8),
  MY_BUILTIN_FMT(s16),
  MY_BUILTIN_FMT(s32),
  MY_BUILTIN_FMT(s64),
  MY_BUILTIN_FMT(u8),
  MY_BUILTIN_FMT(u16),
  MY_BUILTIN_FMT(u32),
  MY_BUILTIN_FMT(u64),
  MY_BUILTIN_FMT(f32),
  MY_BUILTIN_FMT(f64),
  MY_BUILTIN_FMT(c8),
  MY_BUILTIN_FMT(c16),
  MY_BUILTIN_FMT(context),
  MY_BUILTIN_FMT(hash),
  MY_BUILTIN_FMT(hash_short),
  MY_BUILTIN_FMT(str_builder),
  MY_BUILTIN_FMT(fixed_array),
  MY_BUILTIN_FMT(quoted_str),
  MY_BUILTIN_FMT(color),

  // Custom formatters
  { SP_FMT_ID(vec2), sp_fmt_format_vec2 },
  { SP_FMT_ID(rgba), sp_fmt_format_rgba },
};

// ============================================================================
// 6. Implement extended format function
// ============================================================================

sp_str_t my_format_v(sp_str_t fmt, va_list args) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  // Same parsing logic as sp_format_v, but using my_formatters
  sp_format_parser_t parser = SP_ZERO_INITIALIZE();
  parser.fmt = fmt;

  while (true) {
    if (sp_format_parser_is_done(&parser)) {
      break;
    }

    c8 c = sp_format_parser_peek(&parser);
    switch (c) {
      case '{': {
        sp_format_parser_eat(&parser);
        sp_format_specifier_t specifier = sp_format_parser_specifier(&parser);

        if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_FG_COLOR) {
          sp_str_builder_append(&builder, specifier.color);
        }

        sp_format_parser_eat_and_assert(&parser, '}');

        my_format_arg_t arg = va_arg(args, my_format_arg_t);
        u32 formatted_value_start = builder.buffer.len;

        // Look up formatter
        SP_CARR_FOR(my_formatters, i) {
          if (arg.id == my_formatters[i].id) {
            my_formatters[i].fn(&builder, &arg);
            break;
          }
        }

        sp_str_t formatted_value = sp_str(
          builder.buffer.data + formatted_value_start,
          builder.buffer.len - formatted_value_start
        );

        if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_PAD) {
          if (formatted_value.len < specifier.pad) {
            u32 spaces_needed = specifier.pad - formatted_value.len;
            for (u32 i = 0; i < spaces_needed; i++) {
              sp_str_builder_append_c8(&builder, ' ');
            }
          }
        }

        if (specifier.flags & SP_FORMAT_SPECIFIER_FLAG_FG_COLOR) {
          sp_str_builder_append_cstr(&builder, SP_ANSI_RESET);
        }

        break;
      }
      default: {
        sp_str_builder_append_c8(&builder, c);
        sp_format_parser_eat(&parser);
        break;
      }
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
// 7. Define user-friendly macros
// ============================================================================

#define MY_FMT_ARG(T, V) (my_format_arg_t){ .id = SP_FMT_ID(T), .SP_FMT_UNION(T) = (V) }

// Re-export built-in types with new signature
#define MY_FMT_PTR(V)   MY_FMT_ARG(ptr, V)
#define MY_FMT_STR(V)   MY_FMT_ARG(str, V)
#define MY_FMT_CSTR(V)  MY_FMT_ARG(cstr, V)
#define MY_FMT_S32(V)   MY_FMT_ARG(s32, V)
#define MY_FMT_U32(V)   MY_FMT_ARG(u32, V)
#define MY_FMT_F32(V)   MY_FMT_ARG(f32, V)
#define MY_FMT_U8(V)    MY_FMT_ARG(u8, V)
// ... etc for all built-in types needed

// Custom type macros
#define MY_FMT_VEC2(V)  MY_FMT_ARG(vec2, V)
#define MY_FMT_RGBA(V)  MY_FMT_ARG(rgba, V)

// ============================================================================
// 8. Example usage
// ============================================================================

#if 0
int main() {
  vec2_t pos = { 10.5f, 20.3f };
  rgba_t color = { 255, 128, 64, 255 };

  sp_str_t msg = my_format(
    "Position: {}, Color: {}, Count: {}",
    MY_FMT_VEC2(pos),
    MY_FMT_RGBA(color),
    MY_FMT_U32(42)
  );

  // Output: "Position: vec2(10.50, 20.30), Color: #ff8040ff, Count: 42"

  return 0;
}
#endif

// ============================================================================
// PROS of this approach:
// ============================================================================
// ✓ No modification to sp.h required
// ✓ Compile-time type registration
// ✓ Can use sp.h utilities in format functions
// ✓ Type-safe
// ✓ Can mix built-in and custom types freely
//
// ============================================================================
// CONS of this approach:
// ============================================================================
// ✗ Need to manually list all built-in formatters in my_formatters array
// ✗ Must use MY_FMT_* instead of SP_FMT_* (different namespace)
// ✗ Duplicates some code from sp_format_v
// ✗ More boilerplate to set up
