---
name: sp.h
description: Guide for using sp.h, a single-header C standard library replacement. This skill should be used when working with C code that uses the sp.h library, implementing new features in sp.h, or when the user needs guidance on sp.h conventions, patterns, and APIs.
license: MIT
---

# sp.h Usage Guide

sp.h is a single-header C standard library replacement, designed like `stb` libraries but for the entire standard library. It provides memory management, strings, containers, I/O, formatting, threading, time operations, and OS abstractions.

## When to Use This Skill

Use this skill when:
- Writing C code that uses sp.h APIs
- Implementing new features or tests for sp.h
- Converting code from standard library to sp.h conventions
- Understanding sp.h design patterns and idioms
- Working with code that follows sp.h coding standards

## Core Philosophy

sp.h follows strict conventions:
- **No C standard library**: Never use malloc, printf, strcmp, etc. Use sp_alloc, sp_format, sp_str_equal instead.
- **Context-based allocation**: All allocation goes through sp_alloc which respects the current allocator context.
- **String safety**: Use sp_str_t (length + pointer) everywhere except at API boundaries with external code.
- **Zero initialization**: Always initialize structs with SP_ZERO_INITIALIZE().
- **Designated initializers**: Use C99 designated initializers for struct literals.

## Quick Reference

### Essential Edicts

Refer to `references/edicts.md` for the complete list of coding standards that must be followed when working with sp.h.

### API Namespaces

sp.h organizes its APIs into logical namespaces. For detailed documentation of each namespace and available functions, see `references/namespaces.md`.

Key namespaces include:
- **Memory** (`sp_alloc`, `sp_context`, `sp_allocator`) - Context-based allocation
- **Strings** (`sp_str_t`, `sp_str_builder_t`) - Safe string handling
- **Containers** (`sp_dyn_array`, `sp_ht`, `sp_ring_buffer`) - Data structures
- **I/O** (`sp_io_*`) - Stream-based file I/O
- **Time** (`sp_tm_*`) - Timers, epochs, date/time
- **OS** (`sp_os_*`) - Cross-platform OS abstractions
- **Threading** (`sp_thread`, `sp_mutex`, `sp_semaphore`, `sp_atomic`, `sp_spin_lock`) - Concurrency primitives
- **Formatting** (`sp_format`, `SP_LOG`, `SP_FMT_*`) - Type-safe printf replacement

### Real-World Examples

For concrete usage patterns from production code, see `references/examples.md` which contains patterns extracted from spn.h (the package manager built with sp.h).

## Common Patterns

### Initialization Pattern

```c
// Always zero-initialize structs
sp_str_builder_t builder = SP_ZERO_INITIALIZE();
sp_dynamic_array_t arr = SP_ZERO_INITIALIZE();
```

### String Handling

```c
// Create strings
sp_str_t literal = SP_LIT("hello");           // Compile-time string literal
sp_str_t from_cstr = SP_CSTR(some_char_ptr);  // Runtime C string (calculates length)
sp_str_t copy = sp_str_from_cstr("hello");    // Allocates and copies

// Only convert to C string at API boundaries
const char* cstr = sp_str_to_cstr(str);  // For external APIs only
```

### Dynamic Arrays (stb-style)

```c
// Initialize to NULL
sp_dyn_array(int) numbers = SP_NULLPTR;

// Push elements (automatically grows)
sp_dyn_array_push(numbers, 42);
sp_dyn_array_push(numbers, 100);

// Iterate
sp_dyn_array_for(numbers, i) {
  SP_LOG("numbers[{}] = {}", SP_FMT_U32(i), SP_FMT_S32(numbers[i]));
}

// Size and capacity
u32 count = sp_dyn_array_size(numbers);
u32 capacity = sp_dyn_array_capacity(numbers);

// Cleanup happens automatically via allocator
```

### Hash Tables (stb-style)

```c
// Initialize to NULL, specify key and value types
sp_ht(sp_str_t, int) map = sp_ht_new(sp_str_t, int);

// Initialize if needed
if (!map) sp_ht_init(map);

// Insert
sp_ht_insert(map, SP_LIT("answer"), 42);

// Lookup
int* value_ptr = sp_ht_getp(map, SP_LIT("answer"));
if (value_ptr) {
  SP_LOG("Found: {}", SP_FMT_S32(*value_ptr));
}

// Check existence
if (sp_ht_exists(map, SP_LIT("answer"))) {
  // key exists
}

// Iterate
sp_ht_for(map, it) {
  sp_str_t* key = sp_ht_it_getkp(map, it);
  int* val = sp_ht_it_getp(map, it);
  SP_LOG("{} -> {}", SP_FMT_STR(*key), SP_FMT_S32(*val));
}
```

### Formatting and Logging

```c
// Type-safe formatting with color support
SP_LOG("Processing {:fg cyan} with {} items", 
  SP_FMT_STR(name), 
  SP_FMT_U32(count));

// Build strings
sp_str_t msg = sp_format("Result: {}", SP_FMT_S32(42));

// Colors: :fg, :bg, :color
// Colors: black, red, green, yellow, blue, magenta, cyan, white
// Add 'bright' prefix for bright variants
```

### Switch Statements

```c
// Always use braces, always handle all cases
switch (state) {
  case STATE_IDLE: {
    // Handle idle
    break;
  }
  case STATE_RUNNING: {
    // Handle running
    break;
  }
  default: {
    SP_UNREACHABLE_CASE();
  }
}
```

### Error Handling

```c
// Return bool for recoverable errors
bool load_config(sp_str_t path, config_t* config) {
  if (!sp_os_does_path_exist(path)) {
    SP_LOG("Config not found: {}", SP_FMT_STR(path));
    return false;
  }
  // ... load config
  return true;
}

// Use SP_ASSERT for programming errors (non-recoverable)
void process_array(int* arr, u32 size) {
  SP_ASSERT(arr);
  SP_ASSERT(size > 0);
  // ...
}

// Use SP_FATAL for fatal errors with formatted messages
if (critical_failure) {
  SP_FATAL("Cannot continue: {:fg red}", SP_FMT_STR(reason));
}
```

## How to Use This Skill

1. **Check edicts first**: When writing sp.h code, consult `references/edicts.md` for mandatory coding standards.

2. **Find the right API**: Use `references/namespaces.md` to locate functions by category (memory, strings, I/O, etc.).

3. **See real usage**: Check `references/examples.md` for concrete patterns from production code.

4. **Follow conventions**: 
   - Two-space indentation
   - Use `sp_str_t` everywhere except API boundaries
   - Initialize with `SP_ZERO_INITIALIZE()`
   - Never call standard library functions directly

5. **Use the type system**:
   - Sized integers: `s8`, `s16`, `s32`, `s64`, `u8`, `u16`, `u32`, `u64`
   - Floats: `f32`, `f64`
   - Characters: `c8`, `c16`

6. **Leverage macros**:
   - `SP_LIT("string")` for string literals
   - `SP_CSTR(ptr)` for C string conversion
   - `SP_CARR_FOR(array, i)` for C array iteration
   - `SP_FMT_*` for type-safe formatting

## Testing

When adding tests, use the utest framework (see existing tests in `test/main.c`):

```c
UTEST(namespace, test_name) {
  // Setup
  sp_str_t str = SP_LIT("hello");
  
  // Test
  ASSERT_TRUE(sp_str_valid(str));
  ASSERT_EQ(str.len, 5);
}
```

## Building

```bash
make c      # Build and run C tests
make test   # Build and run C + C++ tests
make ps     # Build and run process tests
```
