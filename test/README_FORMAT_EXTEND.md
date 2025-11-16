# Custom Format Types for sp_format

Complete working example showing how to extend `sp_format()` with custom types at **compile time**, **zero runtime overhead**, **no sp.h modifications required**.

## Full Example

See `test/example_custom_fmt.c` for a complete, runnable example.

## Usage

```c
#include "sp.h"

// 1. Define custom types
typedef struct { f32 x, y; } vec2_t;
typedef struct { u8 r, g, b, a; } rgba_t;

// 2. Create extended enum
typedef enum {
  my_fmt_ptr, my_fmt_str, /* ... all built-in types ... */
  my_fmt_vec2,  // Custom
  my_fmt_rgba,  // Custom
} my_fmt_id_t;

// 3. Create extended arg struct
typedef struct {
  my_fmt_id_t id;
  union {
    void* ptr_value; sp_str_t str_value; /* ... all built-in ... */
    vec2_t vec2_value;  // Custom
    rgba_t rgba_value;  // Custom
  };
} my_fmt_arg_t;

// 4. Implement formatters
void my_fmt_format_vec2(sp_str_builder_t* b, my_fmt_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  // Can use sp_format() here!
  sp_str_t s = sp_format("({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(b, s);
}

// 5. Create formatter table
typedef void (*my_fmt_fn_t)(sp_str_builder_t*, my_fmt_arg_t*);
static my_formatter_t formatters[] = {
  { my_fmt_ptr, (my_fmt_fn_t)sp_fmt_format_ptr },  // Built-in
  /* ... all other built-ins ... */
  { my_fmt_vec2, my_fmt_format_vec2 },  // Custom
  { my_fmt_rgba, my_fmt_format_rgba },  // Custom
};

// 6. Implement format function (same logic as sp_format_v)
sp_str_t my_format_v(sp_str_t fmt, va_list args) { /* ... */ }
sp_str_t my_format(const c8* fmt, ...) { /* ... */ }

// 7. Create wrapper macros
#define MY_VEC2(V) ((my_fmt_arg_t){my_fmt_vec2, .vec2_value = (V)})
#define MY_RGBA(V) ((my_fmt_arg_t){my_fmt_rgba, .rgba_value = (V)})
#define MY_U32(V)  ((my_fmt_arg_t){my_fmt_u32, .u32_value = (V)})
// ... etc for all types you need

// 8. Use it!
vec2_t pos = {100, 200};
rgba_t color = {255, 128, 64, 255};
sp_str_t msg = my_format("Player at {} with color {}",
  MY_VEC2(pos), MY_RGBA(color));
```

## Building and Running

```bash
# Compile
gcc -I. -Itest -Iexternal/utest.h -Wall -lpthread -lm \
  test/example_custom_fmt.c -std=c99 \
  -o build/debug/example_custom_fmt -DSP_IMPLEMENTATION

# Run
./build/debug/example_custom_fmt
```

Output:
```
[==========] Running 6 test cases.
[ RUN      ] custom_format.vec2_basic
[       OK ] custom_format.vec2_basic (0ns)
[ RUN      ] custom_format.rgba_basic
[       OK ] custom_format.rgba_basic (0ns)
[ RUN      ] custom_format.mixed_types
[       OK ] custom_format.mixed_types (0ns)
[ RUN      ] custom_format.all_builtin_types_work
[       OK ] custom_format.all_builtin_types_work (0ns)
[ RUN      ] custom_format.game_example
[       OK ] custom_format.game_example (0ns)
[ RUN      ] custom_format.empty_and_plain
[       OK ] custom_format.empty_and_plain (0ns)
[==========] 6 test cases ran.
[  PASSED  ] 6 tests.
```

## Key Features

✓ **Compile-time registration** - No runtime overhead
✓ **Type-safe** - Compiler catches type errors
✓ **No sp.h modifications** - Works with stock sp.h
✓ **Recursive formatting** - Custom formatters can call `sp_format()` or `my_format()`
✓ **Mix built-in and custom types** - All types work together seamlessly
✓ **Clean ergonomics** - Simple macro API

## How It Works

1. **Extend the enum** - Add custom type IDs after built-in IDs
2. **Extend the union** - Add custom type storage to the union
3. **Implement formatters** - Write format functions for custom types
4. **Build formatter table** - Map IDs to formatter functions (both built-in and custom)
5. **Copy sp_format_v logic** - Use extended enum/union/table instead of built-in ones
6. **Wrap in macros** - Create ergonomic constructors

The magic is that the format arg union can hold **either** built-in types **or** custom types,
and the formatter table dispatches to the right function based on the ID at the front.

## Example Output

```c
vec2_t player_pos = {123.4f, 567.8f};
rgba_t damage_color = {255, 64, 64, 200};
u32 health = 75;

sp_str_t log = my_format(
  "[Game] Hero (hp:{}) at {} damage: {}",
  MY_U32(health),
  MY_VEC2(player_pos),
  MY_RGBA(damage_color)
);

// Result:
// "[Game] Hero (hp:75) at (123.400, 567.800) damage: #ff4040c8"
```

## Comparison to printf

**Before (printf):**
```c
printf("[Game] Hero (hp:%u) at (%.1f, %.1f) damage: #%02x%02x%02x%02x\n",
  health, pos.x, pos.y, color.r, color.g, color.b, color.a);
```

**After (my_format):**
```c
sp_str_t log = my_format("[Game] Hero (hp:{}) at {} damage: {}",
  MY_U32(health), MY_VEC2(pos), MY_RGBA(color));
```

Cleaner, type-safe, and easier to maintain.
