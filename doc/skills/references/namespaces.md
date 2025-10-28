# sp.h API Namespaces

This document lists all public APIs in sp.h organized by namespace/category. See api_categories.md in the doc/ directory for function line numbers.

## Memory Management

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

- `sp_hash_cstr` - Hash a C string
- `sp_hash_combine` - Combine multiple hashes
- `sp_hash_bytes` - Hash arbitrary bytes

## Containers

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

- `sp_ternary_to_str` - Convert three-state boolean to string

## Logging

- `SP_LOG(fmt, ...)` - Log with color formatting (macro)
- `sp_log` - Log function (use macro instead)

## File Monitor

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

### Platform Info

- `sp_os_platform_kind` - Get platform enum
- `sp_os_platform_name` - Get platform name string
- `sp_os_lib_kind_to_extension` - Get library extension (.so, .dll, etc.)
- `sp_os_lib_to_file_name` - Convert lib name to filename

### Memory

- `sp_os_allocate_memory` - Allocate from OS
- `sp_os_reallocate_memory` - Reallocate
- `sp_os_free_memory` - Free to OS
- `sp_os_copy_memory` - Copy memory (memmove)
- `sp_os_move_memory` - Move memory
- `sp_os_is_memory_equal` - Compare memory
- `sp_os_fill_memory` - Fill with pattern
- `sp_os_fill_memory_u8` - Fill with byte
- `sp_os_zero_memory` - Zero memory

### Filesystem

- `sp_os_does_path_exist` - Check if path exists
- `sp_os_is_regular_file` - Check if file
- `sp_os_is_directory` - Check if directory
- `sp_os_is_path_root` - Check if root path
- `sp_os_is_glob` - Check if path contains glob patterns
- `sp_os_is_program_on_path` - Check if program in PATH
- `sp_os_create_directory` - Create directory (recursive)
- `sp_os_remove_directory` - Remove directory
- `sp_os_create_file` - Create empty file
- `sp_os_remove_file` - Delete file
- `sp_os_copy` - Copy file or directory
- `sp_os_copy_glob` - Copy matching glob pattern
- `sp_os_copy_file` - Copy file
- `sp_os_copy_directory` - Copy directory recursively
- `sp_os_scan_directory` - List directory contents
- `sp_os_normalize_path` - Normalize path separators
- `sp_os_normalize_path_soft` - Normalize in place
- `sp_os_parent_path` - Get parent directory
- `sp_os_join_path` - Join path components
- `sp_os_extract_extension` - Get file extension
- `sp_os_extract_stem` - Get filename without extension
- `sp_os_extract_file_name` - Get filename from path
- `sp_os_get_cwd` - Get current working directory
- `sp_os_get_executable_path` - Get executable path
- `sp_os_get_storage_path` - Get user storage path
- `sp_os_get_config_path` - Get config directory
- `sp_os_canonicalize_path` - Resolve to absolute path
- `sp_os_file_attributes` - Get file attributes
- `sp_os_sleep_ms` - Sleep for milliseconds

### Environment Variables

- `sp_os_get_env_var` - Get environment variable
- `sp_os_get_env_as_path` - Get env var as path
- `sp_os_clear_env_var` - Clear environment variable
- `sp_os_export_env_var` - Set environment variable
- `sp_os_export_env` - Export environment table

### Windows-Specific

- `sp_os_wstr_to_cstr` - Convert wide string
- `sp_os_winapi_attr_to_sp_attr` - Convert attributes

### Threading

- `sp_thread_init` - Initialize thread
- `sp_thread_join` - Wait for thread
- `sp_thread_launch` - Thread entry point callback
- `sp_mutex_init` - Initialize mutex
- `sp_mutex_lock` - Lock mutex
- `sp_mutex_unlock` - Unlock mutex
- `sp_mutex_destroy` - Destroy mutex
- `sp_mutex_kind_to_c11` - Convert mutex kind
- `sp_semaphore_init` - Initialize semaphore
- `sp_semaphore_destroy` - Destroy semaphore
- `sp_semaphore_wait` - Wait on semaphore
- `sp_semaphore_wait_for` - Wait with timeout
- `sp_semaphore_signal` - Signal semaphore
- `sp_spin_pause` - CPU pause hint
- `sp_spin_try_lock` - Try to acquire spinlock
- `sp_spin_lock` - Acquire spinlock
- `sp_spin_unlock` - Release spinlock
- `sp_atomic_s32_cmp_and_swap` - Atomic compare-and-swap
- `sp_atomic_s32_set` - Atomic set
- `sp_atomic_s32_add` - Atomic add
- `sp_atomic_s32_get` - Atomic get
- `sp_future_create` - Create future
- `sp_future_set_value` - Set future value
- `sp_future_destroy` - Destroy future

## I/O Streams

- `sp_io_from_file` - Open file stream
- `sp_io_from_memory` - Create memory stream
- `sp_io_from_file_handle` - Wrap file handle
- `sp_io_read` - Read bytes
- `sp_io_write` - Write bytes
- `sp_io_write_str` - Write sp_str_t
- `sp_io_seek` - Seek in stream
- `sp_io_size` - Get stream size
- `sp_io_close` - Close stream
- `sp_io_read_file` - Read entire file to string

## Time

### Epoch Time

- `sp_tm_now_epoch` - Get current time as epoch
- `sp_tm_to_iso8601` - Format epoch as ISO8601 string

### Timers

- `sp_tm_start_timer` - Start a timer
- `sp_tm_read_timer` - Read elapsed time
- `sp_tm_lap_timer` - Read and reset lap time
- `sp_tm_reset_timer` - Reset timer

### Time Points

- `sp_tm_now_point` - Get current time point
- `sp_tm_point_diff` - Diff between time points

### Date/Time

- `sp_tm_get_date_time` - Get current date/time
- `sp_os_file_mod_time_precise` - Get file modification time

## Formatting

### Type Formatters

All use `SP_FMT_TYPE(value)` macros:

- `SP_FMT_PTR(v)` - Pointer
- `SP_FMT_STR(v)` - sp_str_t
- `SP_FMT_CSTR(v)` - C string
- `SP_FMT_S8(v)`, `SP_FMT_S16(v)`, `SP_FMT_S32(v)`, `SP_FMT_S64(v)` - Signed integers
- `SP_FMT_U8(v)`, `SP_FMT_U16(v)`, `SP_FMT_U32(v)`, `SP_FMT_U64(v)` - Unsigned integers
- `SP_FMT_F32(v)`, `SP_FMT_F64(v)` - Floats
- `SP_FMT_C8(v)`, `SP_FMT_C16(v)` - Characters
- `SP_FMT_HASH(v)` - Hash (full)
- `SP_FMT_SHORT_HASH(v)` - Hash (short)
- `SP_FMT_QUOTED_STR(v)` - Quoted string
- `SP_FMT_COLOR(v)` - ANSI color code
- `SP_FMT_YELLOW()`, `SP_FMT_CYAN()`, `SP_FMT_CLEAR()` - Color shortcuts

### Format Functions

- `sp_format` - Format with C string template
- `sp_format_str` - Format with sp_str_t template
- `sp_format_v` - Format with va_list

## Parsing

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
