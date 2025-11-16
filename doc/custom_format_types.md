# Custom Format Types

Extend `sp_format()` with your own types at **compile-time** with **zero runtime overhead**.

## Usage

### Step 1: Define your types
```c
typedef struct { f32 x, y; } vec2_t;
typedef struct { u8 r, g, b, a; } rgba_t;
```

### Step 2: Define format types BEFORE including sp.h
```c
#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t)

#include "sp.h"
```

### Step 3: Implement format functions
```c
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  // Can use sp_format recursively!
  sp_str_t result = sp_format("({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

void sp_fmt_format_rgba(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  rgba_t c = arg->rgba_value;
  c8 buf[16];
  snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", c.r, c.g, c.b, c.a);
  sp_str_builder_append_cstr(builder, buf);
}
```

### Step 4: Create wrapper macros
```c
#define SP_FMT_VEC2(V) SP_FMT_ARG(vec2, V)
#define SP_FMT_RGBA(V) SP_FMT_ARG(rgba, V)
```

### Step 5: Use it!
```c
vec2_t pos = { 100, 200 };
rgba_t color = { 255, 128, 64, 255 };

sp_str_t msg = sp_format("Player at {} with color {}",
  SP_FMT_VEC2(pos),
  SP_FMT_RGBA(color));

// Output: "Player at (100.000, 200.000) with color #ff8040ff"
```

## How it Works

When you define `SP_USER_FORMAT_TYPES` before including `sp.h`:

1. **Extended enum**: Your types are added to `sp_format_id_t`
2. **Extended union**: Your types are added to `sp_format_arg_t`
3. **Forward declarations**: `sp_fmt_format_<type>` functions are forward declared
4. **Formatter table**: Your formatters are included in `sp_format_v`'s dispatch table

After including `sp.h`, you implement the format functions. They can use `sp_format()` recursively because:
- The enum/union/forward declarations are already complete
- The formatters array is `static` inside `sp_format_v` and compiled on first call
- Your implementations exist by the time `sp_format_v` is first invoked

## Example: Game Logging

```c
typedef struct {
  sp_str_t name;
  u32 health;
  vec2_t pos;
} player_t;

#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t) \
  SP_FMT_X(player, player_t)

#include "sp.h"

void sp_fmt_format_player(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  player_t p = arg->player_value;
  // Format can use other custom formatters!
  sp_str_t result = sp_format("Player({}, hp={}, pos={})",
    SP_FMT_STR(p.name),
    SP_FMT_U32(p.health),
    SP_FMT_VEC2(p.pos));
  sp_str_builder_append(builder, result);
}

// Usage:
player_t hero = {
  .name = SP_LIT("Alice"),
  .health = 75,
  .pos = { 50.0f, 100.0f }
};

SP_LOG("[Game] Current: {}", SP_FMT_PLAYER(hero));
// Output: [Game] Current: Player(Alice, hp=75, pos=(50.000, 100.000))
```

## Ergonomics Comparison

**Before (printf):**
```c
printf("[Game] %s (hp:%u) at (%.1f, %.1f) damage: #%02x%02x%02x%02x\n",
  player_name, health, pos.x, pos.y, color.r, color.g, color.b, color.a);
```

**After (sp_format with custom types):**
```c
SP_LOG("[Game] {} (hp:{}) at {} damage: {}",
  SP_FMT_STR(player_name), SP_FMT_U32(health),
  SP_FMT_VEC2(pos), SP_FMT_RGBA(color));
```

## Key Features

✓ **Compile-time registration** - Zero runtime overhead
✓ **Type-safe** - Compiler catches type errors
✓ **Integrated into sp.h** - No separate format systems
✓ **Recursive formatting** - Custom formatters can call `sp_format()`
✓ **Mix with built-ins** - Custom and built-in types work together seamlessly
✓ **Clean API** - Same `sp_format()` function, same macro style

## Implementation Notes

- Define `SP_USER_FORMAT_TYPES` before first `#include "sp.h"`
- Use `SP_FMT_X(name, type)` to register types
- Implement `sp_fmt_format_<name>()` functions after including sp.h
- Create `SP_FMT_<NAME>(V)` macros for convenience

## See Also

- `test/custom_format.c` - Complete working example with tests
- `test/fmt.c` - Smoke tests for built-in types
