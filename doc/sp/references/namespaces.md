# sp.h API Namespaces

## Memory Management

Tags: memory, allocator, allocation, context

### sp_context_t - Allocator Context

The context stack allows changing the allocator used by `sp_alloc`:

- `sp_context_set` - Set the current context
- `sp_context_push` - Push a context onto the stack
- `sp_context_push_allocator` - Push just an allocator onto the stack
- `sp_context_pop` - Pop the context stack

Example:
```c
sp_bump_allocator_t bump = SP_ZERO_INITIALIZE();
sp_bump_allocator_init(&bump, 1024 * 1024);
sp_context_push_allocator(sp_bump_allocator_get(&bump));
// All allocations now use bump allocator
void* ptr = sp_alloc(100);
sp_context_pop();  // Restore previous allocator
```

### sp_allocator_t - Allocator Interface

- `sp_allocator_default` - Get the default allocator
- `sp_allocator_alloc` - Allocate with specific allocator
- `sp_allocator_realloc` - Reallocate with specific allocator
- `sp_allocator_free` - Free with specific allocator

### Bump Allocator

Fast arena allocator, freed all at once:

- `sp_bump_allocator_init` - Initialize with capacity
- `sp_bump_allocator_clear` - Reset to empty (reuse memory)
- `sp_bump_allocator_destroy` - Free all memory
- `sp_bump_allocator_on_alloc` - Allocator callback

### Malloc Allocator

Default system allocator:

- `sp_malloc_allocator_init` - Get malloc allocator
- `sp_malloc_allocator_on_alloc` - Allocator callback
- `sp_malloc_allocator_get_metadata` - Get allocation metadata

### Direct Allocation

These use the current context allocator:

- `sp_alloc` - Allocate (zero-initialized)
- `sp_realloc` - Reallocate
- `sp_free` - Free

## Hashing

Tags: hash, hashing, checksum

- `sp_hash_cstr` - Hash a C string
- `sp_hash_combine` - Combine multiple hashes
- `sp_hash_bytes` - Hash arbitrary bytes

## Containers

Tags: array, list, table, map

### sp_fixed_array_t - Fixed-Size Array

- `sp_fixed_array_init` - Initialize with capacity
- `sp_fixed_array_push` - Add element(s)
- `sp_fixed_array_reserve` - Reserve space
- `sp_fixed_array_clear` - Remove all elements
- `sp_fixed_array_byte_size` - Get size in bytes
- `sp_fixed_array_at` - Get element at index

### sp_dynamic_array_t - Dynamic Array (Old Style)

- `sp_dynamic_array_init` - Initialize
- `sp_dynamic_array_push` - Add element
- `sp_dynamic_array_push_n` - Add N elements
- `sp_dynamic_array_reserve` - Reserve capacity
- `sp_dynamic_array_clear` - Remove all elements
- `sp_dynamic_array_byte_size` - Size in bytes
- `sp_dynamic_array_at` - Get element at index
- `sp_dynamic_array_grow` - Grow capacity

### sp_dyn_array - Dynamic Array (stb-style, PREFERRED)

Macros for stb-style dynamic arrays:

- `sp_dyn_array(T)` - Declare array of type T
- `sp_dyn_array_push(arr, val)` - Push element
- `sp_dyn_array_pop(arr)` - Pop element
- `sp_dyn_array_size(arr)` - Get size
- `sp_dyn_array_capacity(arr)` - Get capacity
- `sp_dyn_array_clear(arr)` - Clear array
- `sp_dyn_array_free(arr)` - Free array
- `sp_dyn_array_for(arr, i)` - Iteration macro
- `sp_dyn_array_reserve(arr, n)` - Reserve capacity
- `sp_dyn_array_back(arr)` - Get last element

Example:
```c
sp_dyn_array(sp_str_t) strings = SP_NULLPTR;
sp_dyn_array_push(strings, SP_LIT("hello"));
sp_dyn_array_push(strings, SP_LIT("world"));
sp_dyn_array_for(strings, i) {
  SP_LOG("{}", SP_FMT_STR(strings[i]));
}
```

### sp_ht - Hash Table (stb-style)

Macros for stb-style hash tables:

- `sp_ht(K, V)` - Declare hash table type
- `sp_ht_new(K, V)` - Initialize to NULL
- `sp_ht_init(ht)` - Actually initialize
- `sp_ht_insert(ht, key, val)` - Insert/update
- `sp_ht_getp(ht, key)` - Get pointer to value (or NULL)
- `sp_ht_exists(ht, key)` - Check if key exists
- `sp_ht_erase(ht, key)` - Remove key
- `sp_ht_size(ht)` - Get size
- `sp_ht_clear(ht)` - Clear all entries
- `sp_ht_free(ht)` - Free table
- `sp_ht_for(ht, it)` - Iteration macro
- `sp_ht_it_getp(ht, it)` - Get value pointer during iteration
- `sp_ht_it_getkp(ht, it)` - Get key pointer during iteration

Custom hash/compare functions:
- `sp_ht_set_fns(ht, hash_fn, cmp_fn)` - Set custom functions
- `sp_ht_on_hash_str_key` - Hash function for sp_str_t keys
- `sp_ht_on_compare_str_key` - Compare function for sp_str_t keys

### sp_ring_buffer_t - Circular Buffer

- `sp_ring_buffer_at` - Get element at index
- `sp_ring_buffer_init` - Initialize
- `sp_ring_buffer_back` - Get last element
- `sp_ring_buffer_push` - Push (fails if full)
- `sp_ring_buffer_push_zero` - Push zero-initialized
- `sp_ring_buffer_push_overwrite` - Push (overwrites if full)
- `sp_ring_buffer_push_overwrite_zero` - Push zero (overwrites if full)
- `sp_ring_buffer_pop` - Pop oldest element
- `sp_ring_buffer_bytes` - Size in bytes
- `sp_ring_buffer_clear` - Clear all
- `sp_ring_buffer_destroy` - Free
- `sp_ring_buffer_is_full` - Check if full
- `sp_ring_buffer_is_empty` - Check if empty
- `sp_ring_buffer_iter` - Get forward iterator
- `sp_ring_buffer_riter` - Get reverse iterator
- `sp_ring_buffer_iter_deref` - Dereference iterator
- `sp_ring_buffer_iter_next` - Advance iterator
- `sp_ring_buffer_iter_prev` - Reverse iterator
- `sp_ring_buffer_iter_done` - Check if done

## Strings

Tags: string, text, builder, format

### sp_str_builder_t - String Builder

Build strings efficiently with automatic growth and indentation:

- `sp_str_builder_grow` - Ensure capacity
- `sp_str_builder_add_capacity` - Add capacity
- `sp_str_builder_indent` - Increase indent level
- `sp_str_builder_dedent` - Decrease indent level
- `sp_str_builder_append` - Append sp_str_t
- `sp_str_builder_append_cstr` - Append C string
- `sp_str_builder_append_c8` - Append character
- `sp_str_builder_append_fmt_str` - Append formatted (sp_str_t format)
- `sp_str_builder_append_fmt` - Append formatted (C string format)
- `sp_str_builder_new_line` - Append newline with indentation
- `sp_str_builder_move` - Get string and clear builder
- `sp_str_builder_write` - Get string copy
- `sp_str_builder_write_cstr` - Get C string copy

### C String Operations

Only use these when interfacing with external code:

- `sp_cstr_copy` - Copy C string
- `sp_cstr_copy_to` - Copy to buffer
- `sp_cstr_copy_sized` - Copy with length
- `sp_cstr_copy_to_sized` - Copy to buffer with length
- `sp_cstr_equal` - Compare C strings
- `sp_cstr_len` - Get C string length
- `sp_wstr_to_cstr` - Convert wide string
- `sp_str_to_cstr` - Convert sp_str_t to C string
- `sp_str_to_cstr_double_nt` - Convert with double null terminator

### sp_str_t Creation/Copying

- `SP_LIT(str)` - Create from string literal (compile-time)
- `SP_CSTR(ptr)` - Create from C string (runtime, calculates length)
- `sp_str_copy` - Allocate and copy
- `sp_str_copy_to` - Copy to buffer
- `sp_str_from_cstr` - Allocate and copy from C string
- `sp_str_from_cstr_sized` - Copy with length
- `sp_str_from_cstr_null` - Copy nullable C string
- `sp_str_alloc` - Allocate empty string with capacity
- `sp_str_view` - Create view (alias for SP_CSTR)
- `sp_str_null_terminate` - Ensure null termination

### sp_str_t Comparison

- `sp_str_empty` - Check if empty
- `sp_str_equal` - Compare strings
- `sp_str_equal_cstr` - Compare with C string
- `sp_str_starts_with` - Check prefix
- `sp_str_ends_with` - Check suffix
- `sp_str_contains` - Check substring
- `sp_str_valid` - Check if valid (not NULL)
- `sp_str_at` - Get character at index
- `sp_str_at_reverse` - Get character from end
- `sp_str_back` - Get last character
- `sp_str_compare_alphabetical` - Alphabetical comparison
- `sp_str_sort_kernel_alphabetical` - qsort comparison function
- `sp_str_sub` - Extract substring
- `sp_str_sub_reverse` - Extract substring from end

### sp_str_t Common Operations

- `sp_str_concat` - Concatenate two strings
- `sp_str_replace_c8` - Replace character
- `sp_str_pad` - Pad to width with spaces
- `sp_str_trim` - Trim whitespace
- `sp_str_trim_right` - Trim trailing whitespace
- `sp_str_truncate` - Truncate with trailer
- `sp_str_join` - Join two strings with separator
- `sp_str_join_cstr_n` - Join C string array
- `sp_str_to_upper` - Convert to uppercase
- `sp_str_to_lower` - Convert to lowercase
- `sp_str_capitalize_words` - Capitalize each word
- `sp_str_cleave_c8` - Split into pair at delimiter
- `sp_str_split_c8` - Split into array at delimiter
- `sp_str_contains_n` - Check if array contains string
- `sp_str_join_n` - Join array with separator
- `sp_str_count_n` - Count occurrences in array
- `sp_str_find_longest_n` - Find longest string in array
- `sp_str_find_shortest_n` - Find shortest string in array
- `sp_str_pad_to_longest` - Pad all to match longest

### sp_str_t Reduce/Map

Functional-style operations:

- `sp_str_reduce` - Reduce array with function
- `sp_str_reduce_kernel_join` - Kernel for joining
- `sp_str_reduce_kernel_contains` - Kernel for contains check
- `sp_str_reduce_kernel_count` - Kernel for counting
- `sp_str_reduce_kernel_longest` - Kernel for finding longest
- `sp_str_reduce_kernel_shortest` - Kernel for finding shortest
- `sp_str_map` - Map function over array
- `sp_str_map_kernel_prepend` - Kernel for prepending
- `sp_str_map_kernel_append` - Kernel for appending
- `sp_str_map_kernel_prefix` - Kernel for prefix
- `sp_str_map_kernel_trim` - Kernel for trimming
- `sp_str_map_kernel_pad` - Kernel for padding
- `sp_str_map_kernel_to_upper` - Kernel for uppercase
- `sp_str_map_kernel_to_lower` - Kernel for lowercase
- `sp_str_map_kernel_capitalize_words` - Kernel for capitalization

## Ternary

Tags: ternary, boolean, three-state

- `sp_ternary_to_str` - Convert three-state boolean to string

## Logging

Tags: log, logging, print, output

- `SP_LOG(fmt, ...)` - Log with color formatting (macro)
- `sp_log` - Log function (use macro instead)

## File Monitor

Tags: watch, monitor, file-changes, inotify

Watch files/directories for changes:

- `sp_file_monitor_init` - Initialize monitor
- `sp_file_monitor_init_debounce` - Initialize with debounce
- `sp_file_monitor_add_directory` - Watch directory
- `sp_file_monitor_add_file` - Watch file
- `sp_file_monitor_process_changes` - Poll for changes
- `sp_file_monitor_emit_changes` - Emit pending changes
- `sp_file_monitor_check_cache` - Check cache
- `sp_file_monitor_find_cache_entry` - Find cache entry

## OS Abstractions

Tags: os, platform, filesystem, path

### Platform Info
sp_os_platform_kind_t    sp_os_platform_kind();
sp_str_t                 sp_os_platform_name();
sp_str_t                 sp_os_lib_kind_to_extension(sp_os_lib_kind_t kind);
sp_str_t                 sp_os_lib_to_file_name(sp_str_t lib, sp_os_lib_kind_t kind);


### Memory

void* sp_os_allocate_memory(u32 size);
void* sp_os_reallocate_memory(void* ptr, u32 size);
void  sp_os_free_memory(void* ptr);
void  sp_os_copy_memory(const void* source, void* dest, u32 num_bytes);
void  sp_os_move_memory(const void* source, void* dest, u32 num_bytes);
bool  sp_os_is_memory_equal(const void* a, const void* b, size_t len);
void  sp_os_fill_memory(void* buffer, u32 buffer_size, void* fill, u32 fill_size);
void  sp_os_fill_memory_u8(void* buffer, u32 buffer_size, u8 fill);
void  sp_os_zero_memory(void* buffer, u32 buffer_size);

### Filesystem

bool                     sp_os_is_regular_file(sp_str_t path);
bool                     sp_os_is_directory(sp_str_t path);
bool                     sp_os_is_path_root(sp_str_t path);
bool                     sp_os_is_glob(sp_str_t path);
bool                     sp_os_is_program_on_path(sp_str_t program);
bool                     sp_os_does_path_exist(sp_str_t path);
void                     sp_os_create_directory(sp_str_t path);
void                     sp_os_remove_directory(sp_str_t path);
void                     sp_os_create_file(sp_str_t path);
void                     sp_os_remove_file(sp_str_t path);
void                     sp_os_copy(sp_str_t from, sp_str_t to);
void                     sp_os_copy_glob(sp_str_t from, sp_str_t glob, sp_str_t to);
void                     sp_os_copy_file(sp_str_t from, sp_str_t to);
void                     sp_os_copy_directory(sp_str_t from, sp_str_t to);
sp_da(sp_os_dir_entry_t) sp_os_scan_directory(sp_str_t path);
sp_str_t                 sp_os_normalize_path(sp_str_t path);
void                     sp_os_normalize_path_soft(sp_str_t* path);
sp_str_t                 sp_os_parent_path(sp_str_t path);
sp_str_t                 sp_os_join_path(sp_str_t a, sp_str_t b);
sp_str_t                 sp_os_extract_extension(sp_str_t path);
sp_str_t                 sp_os_extract_stem(sp_str_t path);
sp_str_t                 sp_os_extract_file_name(sp_str_t path);
sp_str_t                 sp_os_get_cwd();
sp_str_t                 sp_os_get_executable_path();
sp_str_t                 sp_os_get_storage_path();
sp_str_t                 sp_os_get_config_path();
sp_str_t                 sp_os_canonicalize_path(sp_str_t path);
sp_tm_epoch_t            sp_os_file_mod_time_precise(sp_str_t path);
sp_os_file_attr_t        sp_os_file_attributes(sp_str_t path);

### Environment Variables

Tags: environment, env, variable

sp_str_t        sp_os_get_env_var(sp_str_t key);
sp_str_t        sp_os_get_env_as_path(sp_str_t key);
void            sp_os_clear_env_var(sp_str_t var);
void            sp_os_export_env_var(sp_str_t key, sp_str_t value, sp_env_export_overwrite_t overwrite);
void            sp_os_export_env(sp_env_t* env, sp_env_export_overwrite_t overwrite);

### Threading

Tags: thread, threading, mutex, atomic

SP_API void         sp_thread_init(sp_thread_t* thread, sp_thread_fn_t fn, void* userdata);
SP_API void         sp_thread_join(sp_thread_t* thread);
SP_API s32          sp_thread_launch(void* userdata);
SP_API void         sp_mutex_init(sp_mutex_t* mutex, sp_mutex_kind_t kind);
SP_API void         sp_mutex_lock(sp_mutex_t* mutex);
SP_API void         sp_mutex_unlock(sp_mutex_t* mutex);
SP_API void         sp_mutex_destroy(sp_mutex_t* mutex);
SP_API s32          sp_mutex_kind_to_c11(sp_mutex_kind_t kind);
SP_API void         sp_semaphore_init(sp_semaphore_t* semaphore);
SP_API void         sp_semaphore_destroy(sp_semaphore_t* semaphore);
SP_API void         sp_semaphore_wait(sp_semaphore_t* semaphore);
SP_API bool         sp_semaphore_wait_for(sp_semaphore_t* semaphore, u32 ms);
SP_API void         sp_semaphore_signal(sp_semaphore_t* semaphore);
SP_API sp_future_t* sp_future_create(u32 size);
SP_API void         sp_future_set_value(sp_future_t* future, void* data);
SP_API void         sp_future_destroy(sp_future_t* future);
SP_API void         sp_spin_pause();
SP_API bool         sp_spin_try_lock(sp_spin_lock_t* lock);
SP_API void         sp_spin_lock(sp_spin_lock_t* lock);
SP_API void         sp_spin_unlock(sp_spin_lock_t* lock);
SP_API bool         sp_atomic_s32_cmp_and_swap(sp_atomic_s32* value, s32 current, s32 desired);
SP_API s32          sp_atomic_s32_set(sp_atomic_s32* value, s32 desired);
SP_API s32          sp_atomic_s32_add(sp_atomic_s32* value, s32 add);
SP_API s32          sp_atomic_s32_get(sp_atomic_s32* value);

## Process

void            sp_env_init(sp_env_t* env);
sp_env_t        sp_env_capture();
sp_env_t        sp_env_copy(sp_env_t* env);
sp_str_t        sp_env_get(sp_env_t* env, sp_str_t name);
void            sp_env_insert(sp_env_t* env, sp_str_t name, sp_str_t value);
void            sp_env_erase(sp_env_t* env, sp_str_t name);
void            sp_env_destroy(sp_env_t* env);
sp_str_t        sp_os_get_env_var(sp_str_t key);
sp_str_t        sp_os_get_env_as_path(sp_str_t key);
void            sp_os_clear_env_var(sp_str_t var);
void            sp_os_export_env_var(sp_str_t key, sp_str_t value, sp_env_export_overwrite_t overwrite);
void            sp_os_export_env(sp_env_t* env, sp_env_export_overwrite_t overwrite);
sp_ps_config_t  sp_ps_config_copy(const sp_ps_config_t* src);
void            sp_ps_config_add_arg(sp_ps_config_t* config, sp_str_t arg);
sp_ps_t         sp_ps_create(sp_ps_config_t config);
sp_ps_output_t  sp_ps_run(sp_ps_config_t config);
sp_io_stream_t* sp_ps_io_in(sp_ps_t* proc);
sp_io_stream_t* sp_ps_io_out(sp_ps_t* proc);
sp_io_stream_t* sp_ps_io_err(sp_ps_t* proc);
sp_ps_status_t  sp_ps_wait(sp_ps_t* proc);
sp_ps_status_t  sp_ps_poll(sp_ps_t* proc, u32 timeout_ms);
sp_ps_output_t  sp_ps_output(sp_ps_t* proc);
bool            sp_ps_kill(sp_ps_t* proc);


## IO Streams

Tags: io, file, read, write

sp_io_stream_t sp_io_from_file(sp_str_t path, sp_io_mode_t mode);
sp_io_stream_t sp_io_from_memory(void* memory, u64 size);
sp_io_stream_t sp_io_from_file_handle(sp_os_file_handle_t handle, sp_io_file_close_mode_t close_mode);
u64            sp_io_read(sp_io_stream_t* stream, void* ptr, u64 size);
u64            sp_io_write(sp_io_stream_t* stream, const void* ptr, u64 size);
u64            sp_io_write_str(sp_io_stream_t* stream, sp_str_t str);
s64            sp_io_seek(sp_io_stream_t* stream, s64 offset, sp_io_whence_t whence);
s64            sp_io_size(sp_io_stream_t* stream);
void           sp_io_close(sp_io_stream_t* stream);
sp_str_t       sp_io_read_file(sp_str_t path);

## Time

Tags: time, timer, clock, timestamp

sp_tm_epoch_t             sp_tm_now_epoch();
sp_str_t                  sp_tm_to_iso8601(sp_tm_epoch_t time);
sp_tm_point_t             sp_tm_now_point();
u64                       sp_tm_point_diff(sp_tm_point_t newer, sp_tm_point_t older);
sp_tm_timer_t             sp_tm_start_timer();
u64                       sp_tm_read_timer(sp_tm_timer_t* timer);
u64                       sp_tm_lap_timer(sp_tm_timer_t* timer);
void                      sp_tm_reset_timer(sp_tm_timer_t* timer);
sp_tm_date_time_t         sp_tm_get_date_time();

## Formatting
Tags: format, formatting, printf, print

### Type Formatters
All use `SP_FMT_TYPE(value)` macros:

#define SP_FMT_PTR(V)           SP_FMT_ARG(ptr, V)
#define SP_FMT_STR(V)           SP_FMT_ARG(str, V)
#define SP_FMT_CSTR(V)          SP_FMT_ARG(cstr, V)
#define SP_FMT_S8(V)            SP_FMT_ARG(s8, V)
#define SP_FMT_S16(V)           SP_FMT_ARG(s16, V)
#define SP_FMT_S32(V)           SP_FMT_ARG(s32, V)
#define SP_FMT_S64(V)           SP_FMT_ARG(s64, V)
#define SP_FMT_U8(V)            SP_FMT_ARG(u8, V)
#define SP_FMT_U16(V)           SP_FMT_ARG(u16, V)
#define SP_FMT_U32(V)           SP_FMT_ARG(u32, V)
#define SP_FMT_U64(V)           SP_FMT_ARG(u64, V)
#define SP_FMT_F32(V)           SP_FMT_ARG(f32, V)
#define SP_FMT_F64(V)           SP_FMT_ARG(f64, V)
#define SP_FMT_C8(V)            SP_FMT_ARG(c8, V)
#define SP_FMT_C16(V)           SP_FMT_ARG(c16, V)
#define SP_FMT_CONTEXT(V)       SP_FMT_ARG(context, V)
#define SP_FMT_HASH(V)          SP_FMT_ARG(hash, V)
#define SP_FMT_SHORT_HASH(V)    SP_FMT_ARG(hash_short, V)
#define SP_FMT_STR_BUILDER(V)   SP_FMT_ARG(str_builder, V)
#define SP_FMT_DATE_TIME(V)     SP_FMT_ARG(date_time, V)
#define SP_FMT_THREAD(V)        SP_FMT_ARG(thread, V)
#define SP_FMT_MUTEX(V)         SP_FMT_ARG(mutex, V)
#define SP_FMT_SEMAPHORE(V)     SP_FMT_ARG(semaphore, V)
#define SP_FMT_FIXED_ARRAY(V)   SP_FMT_ARG(fixed_array, V)
#define SP_FMT_DYNAMIC_ARRAY(V) SP_FMT_ARG(dynamic_array, V)
#define SP_FMT_QUOTED_STR(V)    SP_FMT_ARG(quoted_str, V)
#define SP_FMT_COLOR(V)         SP_FMT_ARG(color, V)
#define SP_FMT_YELLOW()         SP_FMT_COLOR(SP_ANSI_FG_YELLOW)
#define SP_FMT_CYAN()           SP_FMT_COLOR(SP_ANSI_FG_CYAN)
#define SP_FMT_CLEAR()          SP_FMT_COLOR(SP_ANSI_FG_RESET)

sp_str_t sp_format_str(sp_str_t fmt, ...);
sp_str_t sp_format(const c8* fmt, ...);
sp_str_t sp_format_v(sp_str_t fmt, va_list args);

## Parsing

Tags: parse, parsing, convert, number

Parse strings to numbers:

- `sp_parse_u8`, `sp_parse_u16`, `sp_parse_u32`, `sp_parse_u64` - Unsigned
- `sp_parse_s8`, `sp_parse_s16`, `sp_parse_s32`, `sp_parse_s64` - Signed
- `sp_parse_f32`, `sp_parse_f64` - Floats
- `sp_parse_c8`, `sp_parse_c16` - Characters
- `sp_parse_ptr` - Pointer
- `sp_parse_bool` - Boolean
- `sp_parse_hash` - Hash
- `sp_parse_hex` - Hexadecimal
- `sp_parse_is_digit` - Check if digit character

Extended versions with error checking:
- `sp_parse_*_ex` - Same as above but return bool, write to out parameter
