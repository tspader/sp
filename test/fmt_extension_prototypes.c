// This file explores 3 different approaches for compile-time custom format type registration
// These are PROTOTYPE designs, not working implementations yet.

#include "sp.h"

// ============================================================================
// OPTION 1: Pre-include extension via SP_USER_FORMAT_TYPES
// ============================================================================
//
// User defines SP_USER_FORMAT_TYPES *before* including sp.h
// sp.h is modified to append user types to SP_FORMAT_TYPES
//
// MECHANICS:
// - sp.h would change SP_FORMAT_TYPES to:
//   #ifndef SP_USER_FORMAT_TYPES
//   #define SP_USER_FORMAT_TYPES
//   #endif
//   #define SP_FORMAT_TYPES \
//     SP_FMT_X(ptr, void*) \
//     ... (all built-in types) ... \
//     SP_USER_FORMAT_TYPES
//
// USAGE:

// Example custom type
typedef struct {
  f32 x, y;
} vec2_t;

#if 0  // Would need to be defined BEFORE #include "sp.h"
// User's custom format type definitions
#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(my_custom, my_type_t)

// User implements format functions
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_builder_append_fmt(builder, "vec2(%.2f, %.2f)", v.x, v.y);
}

void sp_fmt_format_my_custom(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  // ... custom formatting
}
#endif

// PROs:
// - Simple mental model: define types before include
// - All type info centralized in one X macro
// - sp_format_arg_t union automatically includes user types
// - Formatter array automatically includes user formatters
//
// CONs:
// - Requires modifying sp.h to support SP_USER_FORMAT_TYPES
// - User must define types AND implement functions before include (ordering issue)
// - Format functions can't use sp.h utilities easily (they're not defined yet)
// - Doesn't work well with single-header library pattern


// ============================================================================
// OPTION 2: Double-include pattern with SP_FORMAT_EXTEND mode
// ============================================================================
//
// User includes sp.h normally, defines custom types and functions,
// then includes sp.h again with SP_FORMAT_EXTEND defined
//
// MECHANICS:
// - sp.h has conditional compilation based on SP_FORMAT_EXTEND
// - First include: normal behavior
// - Second include with SP_FORMAT_EXTEND:
//   - Skips most of sp.h
//   - Only re-generates formatters array with user additions
//   - User must provide SP_USER_FORMAT_TYPES between includes
//
// USAGE:

#if 0
// First normal include
#include "sp.h"

// User defines their custom types
typedef struct { f32 x, y; } vec2_t;

// User implements format functions (can now use sp.h utilities!)
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_t result = sp_format("vec2({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

// Define extension X macro
#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t)

// Second include to register extensions
#define SP_FORMAT_EXTEND
#include "sp.h"
#undef SP_FORMAT_EXTEND

// Now can use: sp_format("pos: {}", SP_FMT_VEC2(position))
#endif

// PROs:
// - Format functions can use sp.h utilities
// - Clear separation: include, extend, re-include
// - No need to predefine everything
//
// CONs:
// - Including same header twice is unusual/confusing
// - Requires significant sp.h modifications for conditional compilation
// - sp_format_arg_t union problem: it's already defined in first include
//   - Would need to make union extensible somehow
// - Very complex implementation in sp.h


// ============================================================================
// OPTION 3: User-side X macro extension (no sp.h modification)
// ============================================================================
//
// User creates their own complete formatter system that builds on sp.h
// No modifications to sp.h needed at all
//
// MECHANICS:
// - User defines SP_USER_FORMAT_TYPES X macro
// - User creates extended sp_format_arg_t with both built-in and custom types
// - User creates extended formatters array
// - User wraps/reimplements sp_format_v to use extended types
//
// USAGE:

// Define custom types
typedef struct { f32 x, y; } vec2_t;
typedef struct { u8 r, g, b; } color_t;

// Define custom format types
#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(color, color_t)

// Create extended format ID enum
typedef enum {
  // Built-in types
  #undef SP_FMT_X
  #define SP_FMT_X(id, type) SP_FMT_ID(id),
  SP_FORMAT_TYPES

  // User types
  SP_USER_FORMAT_TYPES
} sp_user_format_id_t;

// Create extended format arg struct
typedef struct sp_user_format_arg_t {
  sp_user_format_id_t id;

  union {
    // Built-in types
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES

    // User types
    SP_USER_FORMAT_TYPES
  };
} sp_user_format_arg_t;

// User implements custom format functions
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_user_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_t result = sp_format("({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

void sp_fmt_format_color(sp_str_builder_t* builder, sp_user_format_arg_t* arg) {
  color_t c = arg->color_value;
  sp_str_t result = sp_format("rgb({}, {}, {})",
    SP_FMT_U8(c.r), SP_FMT_U8(c.g), SP_FMT_U8(c.b));
  sp_str_builder_append(builder, result);
}

// User-side formatter wrapper
typedef struct {
  sp_user_format_id_t id;
  void (*fn)(sp_str_builder_t*, sp_user_format_arg_t*);
} sp_user_formatter_t;

// User creates extended formatters array
#if 0  // Pseudo-code - shows the concept
static sp_user_formatter_t user_formatters[] = {
  // Built-in formatters (cast to extended signature)
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) { SP_FMT_ID(ID), (void(*)(sp_str_builder_t*, sp_user_format_arg_t*))SP_FMT_FN(ID) },
  SP_FORMAT_TYPES

  // User formatters
  { SP_FMT_ID(vec2), sp_fmt_format_vec2 },
  { SP_FMT_ID(color), sp_fmt_format_color },
};

// User reimplements sp_format_v-like function
sp_str_t sp_user_format_v(sp_str_t fmt, va_list args) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  // ... same parsing logic as sp_format_v ...
  // but uses user_formatters array instead
  return sp_str_builder_write(&builder);
}

// User macros
#define SP_USER_FMT_VEC2(V)  (sp_user_format_arg_t){ .id = SP_FMT_ID(vec2), .vec2_value = (V) }
#define SP_USER_FMT_COLOR(V) (sp_user_format_arg_t){ .id = SP_FMT_ID(color), .color_value = (V) }

// Usage:
// sp_str_t msg = sp_user_format("pos: {}, tint: {}",
//   SP_USER_FMT_VEC2(position), SP_USER_FMT_COLOR(tint));
#endif

// PROs:
// - NO modification to sp.h required
// - User has full control
// - Can still use built-in formatters
// - Format functions can use sp.h utilities
// - Clear extension point
//
// CONs:
// - User must reimplement sp_format_v parsing logic (or call it differently)
// - Can't mix built-in SP_FMT_* with user SP_USER_FMT_* in same format call
//   - Well, you CAN but need to be careful with types
// - More boilerplate for user
// - Two separate format systems (sp_format vs sp_user_format)
