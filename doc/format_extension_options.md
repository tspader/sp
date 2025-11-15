# Format Type Extension: Compile-Time Registration Options

Three viable approaches for allowing users to define custom format types at compile-time.

## Option 1: Preprocessor Extension Hook (Minimal sp.h Change)

**What changes in sp.h:**
```c
// In sp.h, change SP_FORMAT_TYPES definition to:
#ifndef SP_USER_FORMAT_TYPES
#define SP_USER_FORMAT_TYPES
#endif

#define SP_FORMAT_TYPES \
  SP_FMT_X(ptr, void*) \
  SP_FMT_X(str, sp_str_t) \
  /* ... all built-in types ... */ \
  SP_USER_FORMAT_TYPES  /* <-- append user types */
```

**User code:**
```c
// 1. Define types and format functions BEFORE including sp.h
typedef struct { f32 x, y; } vec2_t;

// 2. Define user format types
#define SP_USER_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t)

// 3. Include sp.h (must come AFTER definitions above)
#include "sp.h"

// 4. Implement format functions
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  // ERROR: Can't use sp_format here - it's not defined yet!
  // Must use lower-level sp_str_builder_* functions
  c8 buf[64];
  snprintf(buf, sizeof(buf), "vec2(%.2f, %.2f)", v.x, v.y);
  sp_str_builder_append_cstr(builder, buf);
}

// 5. Use it
sp_str_t msg = sp_format("pos: {}", SP_FMT_VEC2((vec2_t){1, 2}));
```

**Pros:**
- Minimal sp.h modification (1 line changed)
- Clean integration - uses same sp_format() and SP_FMT_* macros
- No separate format systems
- Type-safe

**Cons:**
- **Format functions cannot use sp_format() recursively** (ordering issue)
- Must define everything before #include "sp.h"
- Awkward ordering requirements
- Can't use sp.h utilities in format functions

**Verdict:** Works but constrained. The inability to use sp_format() in custom formatters is limiting.

---

## Option 2: User-Side Extension (No sp.h Changes Required)

**What changes in sp.h:**
None.

**User code:**
```c
#include "sp.h"

// 1. Define custom types
typedef struct { f32 x, y; } vec2_t;
typedef struct { u8 r, g, b, a; } rgba_t;

// 2. Define format type list
#define MY_FORMAT_TYPES \
  SP_FMT_X(vec2, vec2_t) \
  SP_FMT_X(rgba, rgba_t)

// 3. Extended enum (boilerplate - could be in header)
typedef enum {
  #undef SP_FMT_X
  #define SP_FMT_X(id, type) SP_FMT_ID(id),
  SP_FORMAT_TYPES    // Built-in
  MY_FORMAT_TYPES    // Custom
} my_format_id_t;

// 4. Extended arg struct (boilerplate)
typedef struct {
  my_format_id_t id;
  union {
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES    // Built-in
    MY_FORMAT_TYPES    // Custom
  };
} my_format_arg_t;

// 5. Implement format functions (can use sp_format!)
void sp_fmt_format_vec2(sp_str_builder_t* builder, my_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_t result = sp_format("vec2({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

void sp_fmt_format_rgba(sp_str_builder_t* builder, my_format_arg_t* arg) {
  rgba_t c = arg->rgba_value;
  sp_str_t result = sp_format("rgba({},{},{},{})",
    SP_FMT_U8(c.r), SP_FMT_U8(c.g), SP_FMT_U8(c.b), SP_FMT_U8(c.a));
  sp_str_builder_append(builder, result);
}

// 6. Formatter table (automatic via X macro)
typedef void (*my_format_fn_t)(sp_str_builder_t*, my_format_arg_t*);
typedef struct {
  my_format_id_t id;
  my_format_fn_t fn;
} my_formatter_t;

static my_formatter_t my_formatters[] = {
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) { SP_FMT_ID(ID), (my_format_fn_t)SP_FMT_FN(ID) },
  SP_FORMAT_TYPES    // Built-in - automatic

  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) { SP_FMT_ID(ID), sp_fmt_format_##ID },
  MY_FORMAT_TYPES    // Custom - automatic
};

// 7. Format function (copy sp_format_v logic, use my_formatters)
sp_str_t my_format_v(sp_str_t fmt, va_list args) {
  // ... same as sp_format_v but uses my_formatters array ...
}

sp_str_t my_format(const c8* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  sp_str_t result = my_format_v(SP_CSTR(fmt), args);
  va_end(args);
  return result;
}

// 8. Convenience macros
#define MY_FMT_ARG(T, V) \
  (my_format_arg_t){ .id = SP_FMT_ID(T), .SP_FMT_UNION(T) = (V) }

#define MY_FMT_VEC2(V) MY_FMT_ARG(vec2, V)
#define MY_FMT_RGBA(V) MY_FMT_ARG(rgba, V)
#define MY_FMT_U32(V)  MY_FMT_ARG(u32, V)
// ... etc for each type needed

// 9. Use it
vec2_t pos = {100, 200};
rgba_t color = {255, 128, 64, 255};
sp_str_t msg = my_format("Player at {} with color {}",
  MY_FMT_VEC2(pos), MY_FMT_RGBA(color));
```

**Pros:**
- **No sp.h modification required**
- **Format functions CAN use sp_format() recursively**
- Full control over extension
- Type-safe
- X macros automate formatter array generation

**Cons:**
- More boilerplate (though much is automatable)
- Separate format function (my_format vs sp_format)
- Must use MY_FMT_* macros instead of SP_FMT_*
- Need to duplicate sp_format_v parsing logic

**Verdict:** Most practical. Works today without any sp.h changes.

---

## Option 3: Extern Formatter Registration (Requires sp.h Changes)

**What changes in sp.h:**
```c
// Allow external formatters to be registered
#ifndef SP_EXTERNAL_FORMATTERS
#define SP_EXTERNAL_FORMATTERS
#endif

// In sp_format_v, change formatter array to be extensible
static sp_formatter_t builtin_formatters[] = {
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) { SP_FMT_ID(ID), SP_FMT_FN(ID) },
  SP_FORMAT_TYPES
};

// Concat with external formatters
static sp_formatter_t formatters[] = {
  #undef SP_FMT_X
  #define SP_FMT_X(ID, t) { SP_FMT_ID(ID), SP_FMT_FN(ID) },
  SP_FORMAT_TYPES
  SP_EXTERNAL_FORMATTERS
};

// Extend sp_format_id_t enum
typedef enum {
  #undef SP_FMT_X
  #define SP_FMT_X(id, type) SP_FMT_ID(id),
  SP_FORMAT_TYPES
  #ifndef SP_EXTERNAL_FORMAT_IDS
  #define SP_EXTERNAL_FORMAT_IDS
  #endif
  SP_EXTERNAL_FORMAT_IDS
} sp_format_id_t;

// Extend sp_format_arg_t union
typedef struct sp_format_arg_t {
  sp_format_id_t id;
  union {
    #undef SP_FMT_X
    #define SP_FMT_X(name, type) type SP_FMT_UNION(name);
    SP_FORMAT_TYPES
    #ifndef SP_EXTERNAL_FORMAT_UNIONS
    #define SP_EXTERNAL_FORMAT_UNIONS
    #endif
    SP_EXTERNAL_FORMAT_UNIONS
  };
} sp_format_arg_t;
```

**User code:**
```c
// 1. Define types and functions
typedef struct { f32 x, y; } vec2_t;

void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg);

// 2. Define extension macros BEFORE including sp.h
#define SP_EXTERNAL_FORMAT_IDS \
  SP_FMT_X(vec2, vec2_t)

#define SP_EXTERNAL_FORMAT_UNIONS \
  SP_FMT_X(vec2, vec2_t)

#define SP_EXTERNAL_FORMATTERS \
  SP_FMT_X(vec2, vec2_t)

// 3. Include sp.h
#include "sp.h"

// 4. Implement format functions (can use sp_format!)
void sp_fmt_format_vec2(sp_str_builder_t* builder, sp_format_arg_t* arg) {
  vec2_t v = arg->vec2_value;
  sp_str_t result = sp_format("vec2({}, {})", SP_FMT_F32(v.x), SP_FMT_F32(v.y));
  sp_str_builder_append(builder, result);
}

// 5. Use it with regular sp_format!
sp_str_t msg = sp_format("pos: {}", SP_FMT_VEC2((vec2_t){1,2}));
```

**Pros:**
- Uses same sp_format() function (no separate API)
- Uses same SP_FMT_* macro namespace
- Format functions can use sp_format()
- Clean user-side API

**Cons:**
- Requires significant sp.h modifications (3 extension points)
- Still has ordering issues (define before include)
- **Format functions still can't use sp_format() at definition time**
- More complex sp.h implementation

**Verdict:** Better integration than Option 1, but still has ordering issues and requires sp.h changes.

---

## Recommendation

**Best option: Option 2 (User-Side Extension)**

Reasoning:
1. **Works today** - no sp.h changes needed
2. **Most flexible** - format functions can use sp_format()
3. **Automatable** - X macros handle boilerplate
4. **Could be packaged** as a header library (sp_format_extend.h)

The boilerplate can be wrapped in a helper header:

```c
// sp_format_extend.h - drop-in extension framework
#define SP_FORMAT_EXTEND_BEGIN(NAME, USER_TYPES) \
  /* ... generates all the boilerplate ... */

#define SP_FORMAT_EXTEND_END(NAME) \
  /* ... generates format function and macros ... */
```

Usage becomes:
```c
#include "sp.h"
#include "sp_format_extend.h"

#define MY_TYPES SP_FMT_X(vec2, vec2_t) SP_FMT_X(rgba, rgba_t)
SP_FORMAT_EXTEND_BEGIN(my, MY_TYPES)

void sp_fmt_format_vec2(...) { /* impl */ }
void sp_fmt_format_rgba(...) { /* impl */ }

SP_FORMAT_EXTEND_END(my)
```

This gives clean user API with no sp.h modifications.
