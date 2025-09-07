All subheadings within top-level headings are prioritized.

# style
## indentation
EVERYTHING should be indented with two spaces

## NULL usage
- Line 165: Using raw NULL instead of SP_NULLPTR
- Line 229: Using raw NULL instead of SP_NULLPTR
- Line 391: Using raw NULL instead of SP_NULLPTR
- Multiple instances throughout test.c need NULL → SP_NULLPTR conversion

## Allocator initialization
- Tests use sp_test_use_malloc() repeatedly instead of calling sp_init() once in utest_main
- Static allocators at lines 105-106 should use context stack pattern
- Tests that need specific allocators should push new context as needed

## Memory patterns
- Inconsistent use of SP_ZERO_INITIALIZE() vs SP_ZERO_STRUCT() vs neither (lines 105-106, 116)
- Missing SP_ZERO_INITIALIZE() in many struct initializations


# untested
## Format functions
- All sp_format_* color/style functions
- sp_format_v (variadic version)

## String utilities
- sp_str_trim
- sp_str_trim_right
- sp_str_split_c8
- sp_str_pad
- sp_str_pad_to_longest
- sp_str_starts_with
- sp_str_view (used but not tested directly)
- sp_str_from_cstr (used but not tested directly)

## Parser functions
- sp_parse_u8
- sp_parse_u16
- sp_parse_u32
- sp_parse_u64
- sp_parse_s8
- sp_parse_s16
- sp_parse_s32
- sp_parse_s64
- sp_parse_f32
- sp_parse_f64
- sp_parse_bool
- sp_parse_hex
- sp_parse_hash
- sp_parse_u8_ex
- sp_parse_u16_ex
- sp_parse_u32_ex
- sp_parse_u64_ex
- sp_parse_s8_ex
- sp_parse_s16_ex
- sp_parse_s32_ex
- sp_parse_s64_ex
- sp_parse_f32_ex
- sp_parse_f64_ex
- sp_parse_bool_ex


## Memory operations
- sp_os_reallocate_memory
- sp_os_fill_memory
- sp_os_fill_memory_u8
- sp_os_is_memory_equal

## Hash functions
We don't want to test literal hash values; just ensure that identical values are hashed identically
- sp_hash_str
- sp_hash_combine
- sp_hash_bytes

## Thread primitives
- sp_thread_init
- sp_thread_join
- sp_thread_destroy
- sp_atomic_load
- sp_atomic_store
- sp_atomic_exchange
- sp_atomic_compare_exchange
- sp_atomic_fetch_add
- sp_atomic_fetch_sub

## Allocator functions
- sp_malloc_allocator_init (direct testing)
- sp_bump_allocator_init (direct testing)
- sp_bump_allocator_clear
- sp_bump_allocator_destroy
- sp_context_push_allocator (direct testing)
- sp_context_pop_allocator (direct testing)


# edge cases
## sp_dyn_array_push_f tests (lines 1538-1830)
- Zero-sized element push
- Odd-sized struct alignment (only tests power-of-2 sizes)
- Push after manual capacity manipulation
- Thread safety during concurrent pushes
- NULL pointer dereference protection when array is NULL

## Dynamic array tests (lines 154-434)
- Out-of-bounds access behavior verification
- Shrinking/compacting operations
- Move semantics for large elements
- Performance characteristics validation
- Mixed operations with macro and function versions

## Path function tests (lines 1085-1380)
- Unicode path handling
- Network path handling (UNC paths on Windows)
- Symbolic link resolution edge cases
- Path length exceeding system limits (>260 chars Windows, >4096 Linux)
- Permission-denied scenarios
- Relative path resolution with ../ sequences
- Drive letter edge cases on Windows

## String manipulation tests
- Empty string edge cases for all functions
- Strings with embedded nulls
- UTF-8 multibyte character handling
- Very large string performance (>1MB)
- String operations near memory boundaries

## Thread safety tests (line 2644)
- Only tests SP_MUTEX_PLAIN, not SP_MUTEX_RECURSIVE or SP_MUTEX_TIMED
- No mutex contention scenarios
- No deadlock detection tests
- No thread pool patterns
- No reader-writer lock patterns

## Error handling
- Line 69: fputs without error checking in sp_test_create_file
- Missing tests for all error paths in API functions
- No out-of-memory simulation tests
- No partial failure recovery tests

## Specific issues
- Line 1681: Comment states "size == capacity - 1 will trigger growth" but should be "size == capacity will trigger"
- Line 1506: Magic number 5 without constant definition
- Line 2645: SP_MUTEX_PLAIN used without testing other mutex types
