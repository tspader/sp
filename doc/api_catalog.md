# sp.h API and Pattern Catalog (ITERATION 2)

## Fundamental APIs and Paradigms

### Core Macros and Initialization
- SP_ZERO_INITIALIZE() - zero initialization for any type (sp.h:168)
- SP_ZERO_STRUCT() - zero initialize struct value (sp.h:186)
- SP_NULLPTR - null pointer constant (sp.h:161,170)
- SP_ASSERT() - assertion macro (sp.h:191)
- SP_FATAL() - fatal error with formatted message (sp.h:201)
- SP_EXIT_SUCCESS()/SP_EXIT_FAILURE() - exit macros (sp.h:189-190)
- SP_LVAL() - lvalue cast for C compatibility (sp.h:165)
- SP_UNUSED() - mark unused parameter (sp.h:183)
- SP_BROKEN() - mark broken code (sp.h:204)

### Memory and Context  
- sp_context_push_allocator() - push allocator to context stack (sp.h:2540, spn.h:113)
- sp_context_push() - push context (sp.h:2533)
- sp_context_pop() - pop context (sp.h:2547)
- sp_context_set() - set context (sp.h:2528)
- sp_alloc() - allocate memory (sp.h:2554)
- sp_realloc() - reallocate memory (sp.h:2558)
- sp_free() - free memory (sp.h:2562)
- sp_malloc_allocator_init() - initialize malloc allocator (sp.h:2677)
- sp_malloc_allocator_on_alloc() - malloc allocator callback (sp.h:2642)
- sp_bump_allocator_init() - initialize bump allocator (sp.h:2582)
- sp_bump_allocator_clear() - clear bump allocator (sp.h:2593)
- sp_bump_allocator_destroy() - destroy bump allocator (sp.h:2598)
- sp_bump_allocator_on_alloc() - bump allocator callback (sp.h:2607)

### String Creation and Literals
- SP_LIT() - string literal macro (sp.h:647)
- SP_CSTR() - create string from C string (sp.h:643)
- sp_str() - create string with data and length (sp.h:645)
- sp_str_from_cstr() - string from C string (sp.h:675)
- sp_str_to_cstr() - convert to C string (sp.h:671)
- sp_str_copy() - copy a string (sp.h:673)

### String Builder
- sp_str_builder_append() - append string (sp.h:655)
- sp_str_builder_append_fmt() - append formatted (sp.h:659, spn.h:433-451)
- sp_str_builder_new_line() - add newline (sp.h:660, spn.h:430-437)
- sp_str_builder_indent()/dedent() - manage indentation (sp.h:653-654)
- sp_str_builder_write() - write to string (sp.h:661)

### Formatting
- sp_format() - format string with args (sp.h:1062, spn.h:379-383)
- SP_FMT_U32() - format u32 (sp.h:1035, spn.h:381)
- SP_FMT_STR() - format string (sp.h:1027, spn.h:381)
- SP_FMT_CSTR() - format C string (sp.h:1028)
- SP_FMT_QUOTED_STR() - format quoted string (sp.h:1051, spn.h:440)

### Dynamic Arrays (gunslinger style)
- sp_dyn_array() - declare dynamic array type (spn.h:477)
- sp_dyn_array_push() - push element (spn.h:487, space/main.cpp:1385)
- sp_dyn_array_size() - get size (spn.h:479)
- sp_dyn_array_for() - iterate array (spn.h:492, spn.h:576)
- sp_dyn_array_clear() - clear array (spn.h:483)
- sp_dyn_array_free() - free array (spn.h:484)

### Hash Tables
- sp_hash_table() - declare hash table type (sp.h:521)
- sp_hash_table_insert() - insert key-value (sp.h:529)
- sp_hash_table_get() - get value (sp.h:530)
- sp_hash_table_exists() - check key exists (sp.h:532)
- sp_hash_table_iter_advance() - iterate table (sp.h:536)

### OS and File Operations
- sp_os_get_executable_path() - get exe path (sp.h:931, spn.h:1415)
- sp_os_parent_path() - get parent directory (sp.h:925, space/main.cpp:253)
- sp_os_join_path() - join paths (sp.h:926, spn.h:1154)
- sp_os_does_path_exist() - check path exists (sp.h:915, spn.h:536)
- sp_os_create_directory() - create directory (sp.h:918, spn.h:1167)
- sp_os_canonicalize_path() - canonicalize path (sp.h:932, space/main.cpp:255)

### Threading and Synchronization
- sp_thread_init() - initialize thread (sp.h:940, spn.h:1824)
- sp_mutex_init() - initialize mutex (sp.h:945, spn.h:1199)
- sp_mutex_lock()/unlock() - lock/unlock mutex (sp.h:946-947)
- sp_semaphore_init() - initialize semaphore (sp.h:950)

## APIs with Edge Cases and Special Handling

### String Parsing
- sp_parse_u32() - parse unsigned 32-bit (sp.h:1068, spn.h:915)
- sp_parse_u32_ex() - parse with error checking (sp.h:1085)
- sp_parse_hex() - parse hexadecimal (sp.h:1081)
- sp_parse_f32() - parse float (sp.h:1074)

### String Manipulation  
- sp_str_trim() - trim whitespace (sp.h:3067)
- sp_str_trim_right() - trim right whitespace (sp.h:3046)
- sp_str_split_c8() - split by character (sp.h:3023)
- sp_str_equal() - compare strings (sp.h:2753)
- sp_str_equal_cstr() - compare string to C string (sp.h:2759)
- sp_str_ends_with() - check suffix (sp.h:2766)
- sp_str_join() - join strings with separator (sp.h:2819)
- sp_str_concat() - concatenate strings (sp.h:2815)
- sp_str_replace_c8() - replace character (sp.h:3008)
- sp_str_to_upper() - convert to uppercase (sp.h:3086)
- sp_str_to_lower() - convert to lowercase (sp.h:3101)
- sp_str_capitalize_words() - capitalize words (sp.h:3116)
- sp_str_pad() - pad string (sp.h:2995)
- sp_str_at() - get character at index (sp.h:2797)
- sp_str_at_reverse() - get character from end (sp.h:2804)
- sp_str_back() - get last character (sp.h:2811)
- sp_str_compare_alphabetical() - alphabetical compare (sp.h:2776)
- sp_str_sort_kernel_alphabetical() - qsort kernel (sp.h:2772)

### Process and Shell
- SDL_CreateProcess() - create process (spn.h:22, 374)
- SDL_WaitProcess() - wait for process (spn.h:892)
- SDL_ReadProcess() - read process output (spn.h:884)

### File Monitoring
- sp_file_monitor_init() - init file monitor (sp.h:763, space/main.cpp:450)
- sp_file_monitor_add_directory() - watch directory (sp.h:765, space/main.cpp:451)
- sp_file_monitor_process_changes() - process changes (sp.h:767)

## Exceptional Ergonomics Patterns

### Control Flow Macros
- SP_CARR_FOR() - iterate C array (sp.h:248, spn.h:1238)
- SP_UNREACHABLE_CASE() - unreachable switch case (sp.h:209, spn.h:694)
- SP_FALLTHROUGH() - switch fallthrough (sp.h:184)

### Type Conversions and Utilities
- SP_RVAL() - rvalue cast (sp.h:156,164)
- SP_UNIQUE_ID() - generate unique identifier (sp.h:231)
- SP_MAX()/SP_MIN() - min/max macros (sp.h:233-234)

### SDL Integration Patterns
- SP_SDL_PIPE_STDIO - SDL process flag (sp.h:257, spn.h:374)
- SDL_GetEnvironmentVariable() - get env var (spn.h:1429)
- SDL_SetEnvironmentVariable() - set env var (spn.h:1132)

### ANSI Color Constants
- SP_ANSI_FG_* - foreground colors (sp.h:272-295)
- SP_ANSI_BG_* - background colors (sp.h:280-303)

### Format Extensions
- SP_FMT_COLOR() - format with color (sp.h:1052)
- {:color red} - inline color formatting (spn.h:197,204)
- {:fg brightcyan} - foreground color (spn.h:549)

### TOML Integration
- toml_parse() - parse TOML (spn.h:1599,1768)
- toml_table_table() - get table (spn.h:727,1604)
- toml_table_string() - get string value (spn.h:1736)
- TOML_READ_BOOL() - read bool macro (spn.h:422-428)

### Builder Pattern Extensions
- sp_toml_writer_t - TOML writer (spn.h:329-340)
- sp_toml_writer_add_header() - add TOML header (spn.h:434)
- sp_toml_writer_add_string() - add string value (spn.h:439)

### Path Operations (C++ style)
- operator/ for paths - path joining (space/main.cpp:256-262, test.c:2376)
- paths.asset / "fonts" - path concatenation pattern

### Ring Buffer Pattern
- sp_ring_buffer_init() - initialize ring buffer (sp.h:4831)
- sp_ring_buffer_push() - push to buffer (sp.h:4845)
- sp_ring_buffer_push_zero() - push zeroed element (sp.h:4854)
- sp_ring_buffer_push_overwrite() - push with overwrite (sp.h:4863)
- sp_ring_buffer_pop() - pop from buffer (sp.h:4873)
- sp_ring_buffer_at() - get element at index (sp.h:4827)
- sp_ring_buffer_back() - get last element (sp.h:4840)
- sp_ring_buffer_clear() - clear buffer (sp.h:4886)
- sp_ring_buffer_destroy() - destroy buffer (sp.h:4892)
- sp_ring_buffer_is_full() - check if full (sp.h:4902)
- sp_ring_buffer_is_empty() - check if empty (sp.h:4906)
- sp_ring_buffer_for() - iterate forward macro (test.c:2068)
- sp_ring_buffer_rfor() - iterate reverse macro (test.c:2089)
- sp_rb_it() - dereference iterator macro (test.c:2069)

### String Functional Programming
- sp_str_map() - map function over strings (sp.h:3142)
- sp_str_reduce() - reduce strings with function (sp.h:2964)
- sp_str_map_kernel_trim() - trim kernel (sp.h:3161, test.c:2427)
- sp_str_map_kernel_to_upper() - uppercase kernel (sp.h:3166, test.c:2447)
- sp_str_map_kernel_to_lower() - lowercase kernel (sp.h:3171, test.c:2455)
- sp_str_map_kernel_capitalize_words() - capitalize kernel (sp.h:3176, test.c:2470)
- sp_str_map_kernel_pad() - pad kernel (sp.h:3157)
- sp_str_map_kernel_prefix() - prefix kernel (sp.h:3153)
- sp_str_reduce_kernel_join() - join kernel (sp.h:2983)
- sp_str_reduce_kernel_contains() - contains kernel (sp.h:3181)
- sp_str_reduce_kernel_count() - count kernel (sp.h:3196)
- sp_str_reduce_kernel_longest() - find longest kernel (sp.h:3209)
- sp_str_reduce_kernel_shortest() - find shortest kernel (sp.h:3216)
- sp_str_contains_n() - check if any string contains (sp.h:3224, test.c:2489)
- sp_str_count_n() - count occurrences in strings (sp.h:3233)
- sp_str_find_longest_n() - find longest string (sp.h:3242)
- sp_str_find_shortest_n() - find shortest string (sp.h:3249)
- sp_str_pad_to_longest() - pad all to longest (sp.h:3256)

### Format System Implementation
- sp_fmt_format_unsigned() - format unsigned int (sp.h:1997)
- sp_fmt_format_signed() - format signed int (sp.h:2021)
- sp_fmt_format_hex() - format hexadecimal (sp.h:2039)
- sp_fmt_format_color() - format color (sp.h:2075)
- sp_fmt_format_str() - format string (sp.h:2080)
- sp_fmt_format_cstr() - format C string (sp.h:2087)
- sp_fmt_format_ptr() - format pointer (sp.h:2095)
- sp_fmt_format_f32() - format float (sp.h:2141)
- sp_fmt_format_f64() - format double (sp.h:2184)
- sp_fmt_format_c8() - format char (sp.h:2227)
- sp_fmt_format_c16() - format wide char (sp.h:2233)
- sp_fmt_format_quoted_str() - format quoted string (sp.h:2311)
- sp_format_v() - variadic format (sp.h:2457)
- sp_format_parser_t - format parser (sp.h:2354)
- sp_format_specifier_t - format specifier (sp.h:2359)

### Fixed Array Operations
- sp_fixed_array_init() - initialize fixed array (sp.h:3325)
- sp_fixed_array_at() - get element at index (sp.h:3334)
- sp_fixed_array_push() - push elements (sp.h:3339)
- sp_fixed_array_reserve() - reserve space (sp.h:3348)
- sp_fixed_array_clear() - clear array (sp.h:3356)
- sp_fixed_array_byte_size() - get byte size (sp.h:3362)

### Dynamic Array Implementation  
- sp_dynamic_array_init() - initialize dynamic array (sp.h:3265)
- sp_dynamic_array_at() - get element at index (sp.h:3274)
- sp_dynamic_array_push() - push element (sp.h:3279)
- sp_dynamic_array_push_n() - push multiple elements (sp.h:3283)
- sp_dynamic_array_reserve() - reserve space (sp.h:3291)
- sp_dynamic_array_clear() - clear array (sp.h:3301)
- sp_dynamic_array_byte_size() - get byte size (sp.h:3307)
- sp_dynamic_array_grow() - grow capacity (sp.h:3313)

### Future/Promise Pattern
- sp_future_create() - create future (sp.h:4945)
- sp_future_destroy() - destroy future (sp.h:4954)
- sp_future_set_value() - set future value (sp.h:4960)

### Asset Registry System
- sp_asset_registry_init() - initialize registry (sp.h:4973)
- sp_asset_registry_process_completions() - process completions (sp.h:4999)