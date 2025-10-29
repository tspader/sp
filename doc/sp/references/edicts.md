# sp.h Coding Edicts

These are **mandatory** rules when working with sp.h code.

## Core Rules

### 1. Never Use the C Standard Library

**FORBIDDEN:**
```c
char* str = malloc(100);           // NO
printf("Hello %s\n", name);        // NO
strcmp(a, b);                      // NO
strcpy(dest, src);                 // NO
FILE* f = fopen("file.txt", "r");  // NO
```

**CORRECT:**
```c
void* ptr = sp_alloc(100);
SP_LOG("Hello {}", SP_FMT_STR(name));
sp_str_equal(a, b);
sp_str_copy_to(str, buffer, buffer_size);
sp_io_stream_t f = sp_io_from_file(SP_LIT("file.txt"), SP_IO_MODE_READ);
```

### 2. C Strings Only at API Boundaries

Use `sp_str_t` everywhere inside your code. Only convert to/from C strings when calling external APIs or being called by external code.

**WRONG:**
```c
void internal_function(const char* path) {  // NO - internal function
  // ...
}
```

**CORRECT:**
```c
void internal_function(sp_str_t path) {     // YES - sp_str_t for internal
  // Only convert when calling external API
  FILE* f = fopen(sp_str_to_cstr(path), "r");  
}

// At true API boundary (external callers)
SP_API void public_api(const char* path) {
  sp_str_t path_str = SP_CSTR(path);  // Convert immediately
  internal_function(path_str);
}
```

### 3. Always Initialize with SP_ZERO_INITIALIZE()

**WRONG:**
```c
sp_str_builder_t builder;           // NO - uninitialized
sp_dynamic_array_t arr = {0};       // NO - use macro
```

**CORRECT:**
```c
sp_str_builder_t builder = SP_ZERO_INITIALIZE();
sp_dynamic_array_t arr = SP_ZERO_INITIALIZE();
```

### 4. Use Designated Initializers

**WRONG:**
```c
sp_str_t str = {5, "hello"};                    // NO - positional
return (sp_str_t) {5, "hello"};                 // NO - positional
```

**CORRECT:**
```c
sp_str_t str = {.len = 5, .data = "hello"};
return SP_RVAL(sp_str_t) {.len = 5, .data = "hello"};
```

### 5. Use sp_alloc for All Allocation

All memory allocation must go through `sp_alloc`, `sp_realloc`, `sp_free` to respect the current allocator context.

**WRONG:**
```c
void* ptr = malloc(100);
ptr = realloc(ptr, 200);
free(ptr);
```

**CORRECT:**
```c
void* ptr = sp_alloc(100);
ptr = sp_realloc(ptr, 200);
sp_free(ptr);
```

### 6. Use sp_dyn_array for Resizable Arrays

Use the stb-style dynamic array macros, not manual realloc.

**WRONG:**
```c
int* arr = malloc(10 * sizeof(int));
// manually track size/capacity and realloc
```

**CORRECT:**
```c
sp_dyn_array(int) arr = SP_NULLPTR;  // NULL to start
sp_dyn_array_push(arr, 42);
sp_dyn_array_push(arr, 100);
// Cleanup automatic via allocator
```

### 7. Use sp_ht for Hash Tables

Use the stb-style hash table macros for key-value storage.

**CORRECT:**
```c
sp_ht(sp_str_t, int) map = sp_ht_new(sp_str_t, int);
if (!map) sp_ht_init(map);
sp_ht_insert(map, SP_LIT("key"), 42);
int* val = sp_ht_getp(map, SP_LIT("key"));
```

### 8. Prefer switch Statements to if/else

When handling multiple enum values or states, use switch statements.

**WRONG:**
```c
if (state == STATE_IDLE) {
  // ...
} else if (state == STATE_RUNNING) {
  // ...
} else if (state == STATE_DONE) {
  // ...
}
```

**CORRECT:**
```c
switch (state) {
  case STATE_IDLE: {
    // ...
    break;
  }
  case STATE_RUNNING: {
    // ...
    break;
  }
  case STATE_DONE: {
    // ...
    break;
  }
  default: {
    SP_UNREACHABLE_CASE();
  }
}
```

### 9. Always Use {} Brackets Around Case Statements

**WRONG:**
```c
switch (x) {
  case 1:
    do_something();
    break;
  case 2:
    do_other();
    break;
}
```

**CORRECT:**
```c
switch (x) {
  case 1: {
    do_something();
    break;
  }
  case 2: {
    do_other();
    break;
  }
  default: {
    SP_UNREACHABLE_CASE();
  }
}
```

### 10. ALWAYS Indent with Two Spaces

**WRONG:**
```c
void foo() {
    if (condition) {  // 4 spaces - NO
        bar();
    }
}
```

**CORRECT:**
```c
void foo() {
  if (condition) {  // 2 spaces - YES
    bar();
  }
}
```

## Additional Rules

### Return bool for Recoverable Errors

Don't use errno or return codes. Return `bool` and log the error.

```c
bool load_file(sp_str_t path, file_t* out) {
  if (!sp_os_does_path_exist(path)) {
    SP_LOG("File not found: {}", SP_FMT_STR(path));
    return false;
  }
  // ...
  return true;
}
```

### Use SP_ASSERT for Programming Errors

```c
void process(int* data, u32 size) {
  SP_ASSERT(data);        // Precondition
  SP_ASSERT(size > 0);    // Precondition
  // ...
}
```

### Use SP_FATAL for Unrecoverable Errors

```c
if (critical_system_failure) {
  SP_FATAL("Cannot continue: {:fg red}", SP_FMT_STR(reason));
}
```

### Use Early Returns to Avoid Nesting

**WRONG:**
```c
bool process(sp_str_t path) {
  if (sp_os_does_path_exist(path)) {
    if (sp_os_is_regular_file(path)) {
      if (validate(path)) {
        // deeply nested
        return true;
      }
    }
  }
  return false;
}
```

**CORRECT:**
```c
bool process(sp_str_t path) {
  if (!sp_os_does_path_exist(path)) return false;
  if (!sp_os_is_regular_file(path)) return false;
  if (!validate(path)) return false;
  
  // main logic at top level
  return true;
}
```

### Use SP_LIT for String Literals

```c
sp_str_t name = SP_LIT("config.txt");  // Compile-time constant
```

### Use SP_CSTR for Runtime C Strings

```c
const char* c_string = get_name();
sp_str_t str = SP_CSTR(c_string);  // Calculates length
```

### Use Sized Integer Types

**WRONG:**
```c
int count;          // NO - platform-dependent size
long offset;        // NO - platform-dependent size
unsigned size;      // NO - platform-dependent size
```

**CORRECT:**
```c
s32 count;          // YES - explicitly 32-bit signed
s64 offset;         // YES - explicitly 64-bit signed
u32 size;           // YES - explicitly 32-bit unsigned
```

### Use Type-Safe Formatting

**WRONG:**
```c
printf("Count: %d\n", count);           // NO - type unsafe
fprintf(stderr, "Error: %s\n", msg);    // NO - no colors
```

**CORRECT:**
```c
SP_LOG("Count: {}", SP_FMT_S32(count));
SP_LOG("Error: {:fg red}", SP_FMT_STR(msg));
```

### Handle All Enum Cases in Switches

```c
switch (kind) {
  case KIND_A: {
    break;
  }
  case KIND_B: {
    break;
  }
  case KIND_C: {
    break;
  }
  default: {
    SP_UNREACHABLE_CASE();  // Always have default
  }
}
```

## Summary Checklist

- [ ] No standard library functions used
- [ ] sp_str_t used internally, const char* only at API boundaries
- [ ] All structs initialized with SP_ZERO_INITIALIZE()
- [ ] Designated initializers used for struct literals
- [ ] sp_alloc/sp_realloc/sp_free used for allocation
- [ ] sp_dyn_array used for dynamic arrays
- [ ] sp_ht used for hash tables
- [ ] switch statements preferred over if/else chains
- [ ] All case statements have {} braces
- [ ] Two-space indentation throughout
- [ ] Sized integer types used (s32, u32, s64, etc.)
- [ ] Type-safe formatting with SP_LOG and SP_FMT_* macros
- [ ] Early returns used to avoid deep nesting
- [ ] All enum cases handled in switch statements
