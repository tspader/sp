# Real-World sp.h Usage Examples

Examples extracted from spn.h (the package manager built with sp.h) showing production usage patterns.

## Dynamic Arrays

### Basic Usage

```c
// Initialize to NULL
sp_dyn_array(spn_dep_info_t) deps = SP_NULLPTR;

// Push elements
sp_dyn_array_push(deps, dep_info);

// Iterate
sp_dyn_array_for(deps, i) {
  spn_dep_info_t* dep = deps + i;
  SP_LOG("Processing {}", SP_FMT_STR(dep->name));
}

// Get size
u32 count = sp_dyn_array_size(deps);
```

### Arrays of Strings

```c
sp_dyn_array(sp_str_t) args = SP_NULLPTR;
sp_dyn_array_push(args, SP_LIT("-C"));
sp_dyn_array_push(args, repo_path);
sp_dyn_array_push(args, SP_LIT("fetch"));
```

### Reserve Capacity

```c
sp_dyn_array(sp_str_t) entries = SP_NULLPTR;
sp_dyn_array_reserve(entries, 100);  // Pre-allocate space
```

## Hash Tables

### String to Integer Map

```c
sp_ht(sp_str_t, int) map = sp_ht_new(sp_str_t, int);
if (!map) sp_ht_init(map);

// Set custom hash/compare for string keys
sp_ht_set_fns(map, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

// Insert
sp_ht_insert(map, SP_LIT("count"), 42);

// Lookup
int* value_ptr = sp_ht_getp(map, SP_LIT("count"));
if (value_ptr) {
  SP_LOG("Count: {}", SP_FMT_S32(*value_ptr));
}

// Iterate
sp_ht_for(map, it) {
  sp_str_t* key = sp_ht_it_getkp(map, it);
  int* val = sp_ht_it_getp(map, it);
  // Process key/value
}
```

## String Operations

### String Building with Indentation

```c
sp_str_builder_t builder = SP_ZERO_INITIALIZE();

sp_str_builder_append_fmt(&builder, "function {} {", SP_FMT_STR(name));
sp_str_builder_new_line(&builder);
sp_str_builder_indent(&builder);

sp_str_builder_append(&builder, SP_LIT("return 42;"));
sp_str_builder_new_line(&builder);
sp_str_builder_dedent(&builder);

sp_str_builder_append_c8(&builder, '}');

sp_str_t result = sp_str_builder_write(&builder);
```

### String Padding

```c
// Find max length for alignment
u32 max_len = 0;
sp_dyn_array_for(deps, i) {
  max_len = SP_MAX(max_len, deps[i].name.len);
}

// Pad each name
sp_dyn_array_for(deps, i) {
  sp_str_t padded = sp_str_pad(deps[i].name, max_len);
  SP_LOG("{} {:fg cyan}", SP_FMT_STR(padded), SP_FMT_STR(deps[i].url));
}
```

### String Splitting

```c
sp_dyn_array(sp_str_t) parts = sp_str_split_c8(input, '/');
sp_dyn_array_for(parts, i) {
  SP_LOG("Part {}: {}", SP_FMT_U32(i), SP_FMT_STR(parts[i]));
}
```

### String Trimming

```c
sp_ps_output_t result = sp_ps_run(config);
sp_str_t trimmed = sp_str_trim_right(result.out);  // Remove trailing whitespace
```

### String Joining

```c
sp_dyn_array(sp_str_t) flags = SP_NULLPTR;
sp_dyn_array_push(flags, SP_LIT("-I/usr/include"));
sp_dyn_array_push(flags, SP_LIT("-L/usr/lib"));
sp_dyn_array_push(flags, SP_LIT("-lpthread"));

sp_str_t joined = sp_str_join_n(flags, sp_dyn_array_size(flags), SP_LIT(" "));
// Result: "-I/usr/include -L/usr/lib -lpthread"
```

## Formatting and Logging

### Colored Output

```c
SP_LOG("{:fg brightcyan} {:fg green} {:fg brightblack} {}",
  SP_FMT_STR(name),
  SP_FMT_STR(state),
  SP_FMT_STR(commit),
  SP_FMT_STR(message));

SP_LOG("Error: {:fg red}", SP_FMT_STR(error_msg));
SP_LOG("Success: {:fg brightgreen}", SP_FMT_STR(status));
```

### Format String Building

```c
sp_str_t command = sp_format("git -C {} checkout {}",
  SP_FMT_STR(repo),
  SP_FMT_STR(commit));
```

### Quoted Strings

```c
// For shell-safe output
sp_str_t output = sp_format("SPN_INCLUDES={}", 
  SP_FMT_QUOTED_STR(includes));
```

## Switch Statements

### Enum Handling

```c
switch (state) {
  case SPN_DEP_BUILD_STATE_IDLE: {
    SP_LOG("Idle");
    break;
  }
  case SPN_DEP_BUILD_STATE_CLONING: {
    SP_LOG("Cloning");
    break;
  }
  case SPN_DEP_BUILD_STATE_BUILDING: {
    SP_LOG("Building");
    break;
  }
  case SPN_DEP_BUILD_STATE_DONE: {
    SP_LOG("Done");
    break;
  }
  case SPN_DEP_BUILD_STATE_FAILED: {
    SP_LOG("Failed");
    break;
  }
  default: {
    SP_UNREACHABLE_CASE();
  }
}
```

### Platform-Specific Code

```c
switch (kind) {
  case SP_OS_LIB_SHARED: {
    #ifdef SP_LINUX
      return SP_LIT("so");
    #elif defined(SP_MACOS)
      return SP_LIT("dylib");
    #elif defined(SP_WIN32)
      return SP_LIT("dll");
    #endif
  }
  case SP_OS_LIB_STATIC: {
    #ifdef SP_WIN32
      return SP_LIT("lib");
    #else
      return SP_LIT("a");
    #endif
  }
  default: {
    SP_UNREACHABLE_RETURN(SP_LIT(""));
  }
}
```

## File I/O

### Read Entire File

```c
sp_str_t contents = sp_io_read_file(SP_LIT("config.lua"));
if (!sp_str_valid(contents)) {
  SP_LOG("Failed to read file");
  return false;
}
```

### Write to File

```c
sp_io_stream_t file = sp_io_from_file(path, SP_IO_MODE_WRITE);
if (sp_io_write_str(&file, output)) {
  SP_FATAL("Failed to write {}", SP_FMT_STR(path));
}
sp_io_close(&file);
```

## Process Spawning

### Run Command and Capture Output

```c
sp_ps_output_t result = sp_ps_run((sp_ps_config_t) {
  .command = SP_LIT("git"),
  .args = {
    SP_LIT("-C"), repo,
    SP_LIT("rev-parse"),
    SP_LIT("--short=10"),
    commit_id
  }
});

if (!result.status.exit_code) {
  sp_str_t commit = sp_str_trim_right(result.out);
  SP_LOG("Commit: {}", SP_FMT_STR(commit));
}
```

### Create Process

```c
sp_ps_config_t config = SP_ZERO_INITIALIZE();
config.command = SP_LIT("make");

sp_ps_config_add_arg(&config, SP_LIT("-C"));
sp_ps_config_add_arg(&config, build_dir);

sp_ps_t process = sp_ps_create(config);
sp_ps_status_t status = sp_ps_wait(&process);
```

## Threading

### Spawn Worker Thread

```c
typedef struct {
  sp_thread_t thread;
  sp_mutex_t mutex;
  bool running;
} worker_t;

s32 worker_fn(void* userdata) {
  worker_t* worker = (worker_t*)userdata;
  
  sp_mutex_lock(&worker->mutex);
  worker->running = true;
  sp_mutex_unlock(&worker->mutex);
  
  // Do work...
  
  return 0;
}

// Start thread
worker_t worker = SP_ZERO_INITIALIZE();
sp_mutex_init(&worker.mutex, SP_MUTEX_PLAIN);
sp_thread_init(&worker.thread, worker_fn, &worker);

// Wait for completion
sp_thread_join(&worker.thread);
sp_mutex_destroy(&worker.mutex);
```

### Atomic Operations

```c
sp_atomic_s32 control = 0;

// In signal handler
sp_atomic_s32_set(&control, 1);

// In main loop
if (sp_atomic_s32_get(&control) != 0) {
  // User requested cancellation
  break;
}
```

## Path Operations

### Join Paths

```c
sp_str_t full_path = sp_os_join_path(base_dir, SP_LIT("config.lua"));
```

### Extract Components

```c
sp_str_t dir = sp_os_parent_path(full_path);
sp_str_t filename = sp_os_extract_file_name(full_path);
sp_str_t extension = sp_os_extract_extension(full_path);
sp_str_t stem = sp_os_extract_stem(full_path);
```

### Normalize Paths

```c
sp_str_t path = SP_CSTR(argv[1]);
path = sp_os_normalize_path(path);  // Fix separators
path = sp_os_canonicalize_path(path);  // Resolve to absolute
```

## Directory Operations

### Scan Directory

```c
sp_dyn_array(sp_os_dir_entry_t) entries = sp_os_scan_directory(dir);
for (u32 i = 0; i < sp_dyn_array_size(entries); i++) {
  sp_os_dir_entry_t* entry = entries + i;
  if (entry->attributes & SP_OS_FILE_ATTR_REGULAR_FILE) {
    SP_LOG("File: {}", SP_FMT_STR(entry->file_name));
  }
}
```

### Create/Remove Directories

```c
// Creates parent directories if needed
sp_os_create_directory(SP_LIT("build/cache/store"));

sp_os_remove_directory(SP_LIT("build/cache"));
```

### Copy Operations

```c
// Copy file
sp_os_copy_file(src, dest);

// Copy directory recursively
sp_os_copy_directory(src_dir, dest_dir);

// Copy matching glob
sp_os_copy_glob(src_dir, SP_LIT("*.so"), dest_dir);
```

## Error Handling

### Recoverable Errors (Return bool)

```c
bool load_config(sp_str_t path, config_t* out) {
  if (!sp_os_does_path_exist(path)) {
    SP_LOG("Config not found: {}", SP_FMT_STR(path));
    return false;
  }
  
  sp_str_t contents = sp_io_read_file(path);
  if (!sp_str_valid(contents)) {
    SP_LOG("Failed to read config: {}", SP_FMT_STR(path));
    return false;
  }
  
  // Parse config...
  return true;
}
```

### Fatal Errors

```c
if (critical_condition) {
  SP_FATAL("Cannot continue: {:fg brightred}", SP_FMT_STR(reason));
}
```

### Assertions with Messages

```c
SP_ASSERT_FMT(!result.status.exit_code,
  "Failed to get remote URL for {:fg brightcyan}",
  SP_FMT_STR(repo));
```

## Enum to String Conversion

### Using X-Macros

```c
// Define enum
#define SPN_OUTPUT_MODE_X(X) \
  X(SPN_OUTPUT_MODE_INTERACTIVE) \
  X(SPN_OUTPUT_MODE_NONINTERACTIVE) \
  X(SPN_OUTPUT_MODE_QUIET) \
  X(SPN_OUTPUT_MODE_NONE)

typedef enum {
  SPN_OUTPUT_MODE_X(SP_X_ENUM_DEFINE)
} spn_output_mode_t;

// Convert to string
sp_str_t spn_output_mode_to_str(spn_output_mode_t mode) {
  switch (mode) {
    SPN_OUTPUT_MODE_X(SP_X_ENUM_CASE_TO_STRING_LOWER)
  }
  SP_UNREACHABLE_RETURN(SP_LIT(""));
}
```

### String to Enum

```c
spn_output_mode_t spn_output_mode_from_str(sp_str_t str) {
  if      (sp_str_equal_cstr(str, "interactive"))    return SPN_OUTPUT_MODE_INTERACTIVE;
  else if (sp_str_equal_cstr(str, "noninteractive")) return SPN_OUTPUT_MODE_NONINTERACTIVE;
  else if (sp_str_equal_cstr(str, "quiet"))          return SPN_OUTPUT_MODE_QUIET;
  else if (sp_str_equal_cstr(str, "none"))           return SPN_OUTPUT_MODE_NONE;
  
  SP_FATAL("Unknown output mode {:fg brightyellow}", SP_FMT_STR(str));
  SP_UNREACHABLE_RETURN(SPN_OUTPUT_MODE_NONE);
}
```

## Timing

### Timer Usage

```c
sp_tm_timer_t timer = sp_tm_start_timer();

// Do work...

u64 elapsed_ns = sp_tm_read_timer(&timer);
SP_LOG("Took {} ms", SP_FMT_U64(elapsed_ns / 1000000));
```

### Sleep

```c
sp_os_sleep_ms(10);  // Sleep for 10 milliseconds
```

## Initialization Pattern

```c
typedef struct {
  sp_str_t name;
  sp_dyn_array(sp_str_t) items;
  sp_ht(sp_str_t, int) map;
  sp_mutex_t mutex;
} my_context_t;

void init_context(my_context_t* ctx, sp_str_t name) {
  *ctx = (my_context_t) {
    .name = name,
    .items = SP_NULLPTR,  // Dynamic array
    .map = sp_ht_new(sp_str_t, int),  // Hash table
    .mutex = SP_ZERO_INITIALIZE()
  };
  
  sp_mutex_init(&ctx->mutex, SP_MUTEX_PLAIN);
  if (!ctx->map) sp_ht_init(ctx->map);
}
```

## Common Idioms

### Check Program on PATH

```c
if (sp_os_is_program_on_path(SP_LIT("lsd"))) {
  // Use lsd command
} else if (sp_os_is_program_on_path(SP_LIT("tree"))) {
  // Fallback to tree
} else {
  // Fallback to ls
}
```

### Parse Number Safely

```c
u32 value;
if (sp_parse_u32_ex(str, &value)) {
  // Success
} else {
  SP_LOG("Invalid number: {}", SP_FMT_STR(str));
}
```

### String Comparison

```c
// Compare strings
if (sp_str_equal(a, b)) { }

// Compare with C string
if (sp_str_equal_cstr(str, "expected")) { }

// Check prefix
if (sp_str_starts_with(path, SP_LIT("/"))) { }

// Check suffix
if (sp_str_ends_with(path, SP_LIT(".lua"))) { }
```
