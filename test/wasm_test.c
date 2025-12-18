/*
 * WASM Test Program for sp.h
 *
 * This test program exercises all available features on each WASM target:
 * - Emscripten: Full sp.h with file I/O, timers
 * - WASI: File I/O, environment variables
 * - Freestanding: Core features only (memory, strings, math, hash)
 *
 * Build with the accompanying wasm_build.sh script.
 */

#ifndef SP_IMPLEMENTATION
#define SP_IMPLEMENTATION
#endif
#include "../sp.h"

// For freestanding WASM, we need minimal support functions
#if defined(SP_WASM_FREESTANDING)
// Minimal memset/memcpy for freestanding
void* memset(void* s, int c, size_t n) {
  unsigned char* p = (unsigned char*)s;
  while (n--) *p++ = (unsigned char)c;
  return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
  unsigned char* d = (unsigned char*)dest;
  const unsigned char* s = (const unsigned char*)src;
  while (n--) *d++ = *s++;
  return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
  unsigned char* d = (unsigned char*)dest;
  const unsigned char* s = (const unsigned char*)src;
  if (d < s) {
    while (n--) *d++ = *s++;
  } else {
    d += n;
    s += n;
    while (n--) *--d = *--s;
  }
  return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  const unsigned char* p1 = (const unsigned char*)s1;
  const unsigned char* p2 = (const unsigned char*)s2;
  while (n--) {
    if (*p1 != *p2) return *p1 - *p2;
    p1++;
    p2++;
  }
  return 0;
}

size_t strlen(const char* s) {
  const char* p = s;
  while (*p) p++;
  return p - s;
}
#endif

// ============================================================================
// Test functions for each feature area
// ============================================================================

static int test_count = 0;
static int pass_count = 0;

#if !defined(SP_WASM_FREESTANDING)
#include <stdio.h>
#define TEST_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
// For freestanding, we'll just track counts
#define TEST_PRINT(fmt, ...) ((void)0)
#endif

#define TEST_ASSERT(cond, name) do { \
  test_count++; \
  if (cond) { \
    pass_count++; \
    TEST_PRINT("[PASS] %s\n", name); \
  } else { \
    TEST_PRINT("[FAIL] %s\n", name); \
  } \
} while(0)

// ============================================================================
// Core Tests (available on all targets)
// ============================================================================

void test_memory_allocation(void) {
  TEST_PRINT("\n=== Memory Allocation Tests ===\n");

  // Test basic allocation
  void* ptr = sp_mem_os_alloc(1024);
  TEST_ASSERT(ptr != NULL, "sp_mem_os_alloc returns non-null");

  // Test realloc
  void* ptr2 = sp_mem_os_realloc(ptr, 2048);
  TEST_ASSERT(ptr2 != NULL, "sp_mem_os_realloc returns non-null");

  // Test free (shouldn't crash)
  sp_mem_os_free(ptr2);
  TEST_ASSERT(1, "sp_mem_os_free completes without crash");

  // Test calloc equivalent
  void* ptr3 = sp_mem_os_alloc_zero(512);
  TEST_ASSERT(ptr3 != NULL, "sp_mem_os_alloc_zero returns non-null");

  // Verify zeroed
  unsigned char* bytes = (unsigned char*)ptr3;
  int all_zero = 1;
  for (int i = 0; i < 512; i++) {
    if (bytes[i] != 0) { all_zero = 0; break; }
  }
  TEST_ASSERT(all_zero, "sp_mem_os_alloc_zero returns zeroed memory");
  sp_mem_os_free(ptr3);
}

void test_context_and_allocator(void) {
  TEST_PRINT("\n=== Context and Allocator Tests ===\n");

  // Test context get
  sp_context_t* ctx = sp_context_get();
  TEST_ASSERT(ctx != NULL, "sp_context_get returns non-null");

  // Test allocator through context
  void* ptr = sp_alloc(256);
  TEST_ASSERT(ptr != NULL, "sp_alloc through context works");
  sp_free(ptr);

  // Test arena allocator
  sp_mem_arena_t* arena = sp_mem_arena_new(4096);
  TEST_ASSERT(arena != NULL, "sp_mem_arena_new works");

  sp_allocator_t arena_alloc = sp_mem_arena_as_allocator(arena);
  void* arena_ptr = sp_mem_allocator_alloc(arena_alloc, 128);
  TEST_ASSERT(arena_ptr != NULL, "Arena allocation works");

  sp_mem_arena_clear(arena);
  TEST_ASSERT(arena->bytes_used == 0, "Arena clear works");

  sp_mem_arena_destroy(arena);
  TEST_ASSERT(1, "Arena destroy completes");
}

void test_string_operations(void) {
  TEST_PRINT("\n=== String Operations Tests ===\n");

  // Test string literal
  sp_str_t str1 = SP_LIT("Hello, WASM!");
  TEST_ASSERT(str1.len == 12, "SP_LIT creates correct length");
  TEST_ASSERT(str1.data != NULL, "SP_LIT creates non-null data");

  // Test string view
  sp_str_t str2 = sp_str_view("Test string");
  TEST_ASSERT(str2.len == 11, "sp_str_view creates correct length");

  // Test string equality
  sp_str_t str3 = SP_LIT("Test string");
  TEST_ASSERT(sp_str_equal(str2, str3), "sp_str_equal works for equal strings");

  sp_str_t str4 = SP_LIT("Different");
  TEST_ASSERT(!sp_str_equal(str2, str4), "sp_str_equal works for different strings");

  // Test substring
  sp_str_t sub = sp_str_sub(str1, 0, 5);
  TEST_ASSERT(sub.len == 5, "sp_str_sub creates correct length");
  TEST_ASSERT(sp_str_equal(sub, SP_LIT("Hello")), "sp_str_sub extracts correct content");

  // Test string contains
  TEST_ASSERT(sp_str_contains(str1, SP_LIT("WASM")), "sp_str_contains finds substring");
  TEST_ASSERT(!sp_str_contains(str1, SP_LIT("foo")), "sp_str_contains returns false for missing");

  // Test starts_with/ends_with
  TEST_ASSERT(sp_str_starts_with(str1, SP_LIT("Hello")), "sp_str_starts_with works");
  TEST_ASSERT(sp_str_ends_with(str1, SP_LIT("!")), "sp_str_ends_with works");
}

void test_hash_functions(void) {
  TEST_PRINT("\n=== Hash Function Tests ===\n");

  // Test cstr hash
  sp_hash_t h1 = sp_hash_cstr("test");
  sp_hash_t h2 = sp_hash_cstr("test");
  TEST_ASSERT(h1 == h2, "sp_hash_cstr is deterministic");

  sp_hash_t h3 = sp_hash_cstr("different");
  TEST_ASSERT(h1 != h3, "sp_hash_cstr differs for different strings");

  // Test bytes hash
  u8 data[] = {1, 2, 3, 4, 5};
  sp_hash_t h4 = sp_hash_bytes(data, sizeof(data), 0);
  sp_hash_t h5 = sp_hash_bytes(data, sizeof(data), 0);
  TEST_ASSERT(h4 == h5, "sp_hash_bytes is deterministic");

  // Test hash combine
  sp_hash_t hashes[] = {h1, h3, h4};
  sp_hash_t combined = sp_hash_combine(hashes, 3);
  TEST_ASSERT(combined != 0, "sp_hash_combine produces non-zero result");
}

void test_math_functions(void) {
  TEST_PRINT("\n=== Math Function Tests ===\n");

  // Test vectors
  sp_vec2_t v1 = sp_vec2(1.0f, 2.0f);
  sp_vec2_t v2 = sp_vec2(3.0f, 4.0f);
  sp_vec2_t v3 = sp_vec2_add(v1, v2);
  TEST_ASSERT(v3.x == 4.0f && v3.y == 6.0f, "sp_vec2_add works");

  sp_vec3_t u1 = sp_vec3(1.0f, 0.0f, 0.0f);
  sp_vec3_t u2 = sp_vec3(0.0f, 1.0f, 0.0f);
  sp_vec3_t cross = sp_vec3_cross(u1, u2);
  TEST_ASSERT(cross.z == 1.0f, "sp_vec3_cross works");

  // Test lerp
  f32 lerped = sp_lerp(0.0f, 0.5f, 10.0f);
  TEST_ASSERT(lerped == 5.0f, "sp_lerp works");

  // Test clamp
  f32 clamped = sp_clamp(0.0f, 5.0f, 10.0f);
  TEST_ASSERT(clamped == 5.0f, "sp_clamp in range works");
  clamped = sp_clamp(0.0f, -5.0f, 10.0f);
  TEST_ASSERT(clamped == 0.0f, "sp_clamp below min works");

  // Test color conversion
  sp_color_t rgb = { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
  sp_color_t hsv = sp_color_rgb_to_hsv(rgb);
  TEST_ASSERT(hsv.s > 90.0f, "sp_color_rgb_to_hsv saturation correct for red");
}

void test_dynamic_array(void) {
  TEST_PRINT("\n=== Dynamic Array Tests ===\n");

  sp_da(int) arr = NULL;

  // Test push
  sp_da_push(arr, 10);
  sp_da_push(arr, 20);
  sp_da_push(arr, 30);

  TEST_ASSERT(sp_da_size(arr) == 3, "Dynamic array size correct after pushes");
  TEST_ASSERT(arr[0] == 10, "Dynamic array element 0 correct");
  TEST_ASSERT(arr[1] == 20, "Dynamic array element 1 correct");
  TEST_ASSERT(arr[2] == 30, "Dynamic array element 2 correct");

  // Test pop
  sp_da_pop(arr);
  TEST_ASSERT(sp_da_size(arr) == 2, "Dynamic array size correct after pop");

  // Test clear
  sp_da_clear(arr);
  TEST_ASSERT(sp_da_size(arr) == 0, "Dynamic array empty after clear");
  TEST_ASSERT(sp_da_empty(arr), "sp_da_empty returns true");

  // Test free
  sp_da_free(arr);
  TEST_ASSERT(arr == NULL, "Dynamic array null after free");
}

void test_fixed_array(void) {
  TEST_PRINT("\n=== Fixed Array Tests ===\n");

  sp_fixed_array_t arr;
  sp_fixed_array_init(&arr, 10, sizeof(int));
  TEST_ASSERT(arr.capacity == 10, "Fixed array capacity correct");

  int val1 = 42;
  int val2 = 99;
  sp_fixed_array_push(&arr, &val1, 1);
  sp_fixed_array_push(&arr, &val2, 1);

  TEST_ASSERT(arr.size == 2, "Fixed array size correct");

  int* at0 = (int*)sp_fixed_array_at(&arr, 0);
  int* at1 = (int*)sp_fixed_array_at(&arr, 1);
  TEST_ASSERT(*at0 == 42, "Fixed array element 0 correct");
  TEST_ASSERT(*at1 == 99, "Fixed array element 1 correct");

  sp_fixed_array_clear(&arr);
  TEST_ASSERT(arr.size == 0, "Fixed array size 0 after clear");
}

void test_hash_table(void) {
  TEST_PRINT("\n=== Hash Table Tests ===\n");

  sp_ht(sp_str_t, int) ht = NULL;
  sp_ht_set_fns(ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_str_t key1 = SP_LIT("one");
  sp_str_t key2 = SP_LIT("two");
  sp_str_t key3 = SP_LIT("three");

  sp_ht_insert(ht, key1, 1);
  sp_ht_insert(ht, key2, 2);
  sp_ht_insert(ht, key3, 3);

  TEST_ASSERT(sp_ht_size(ht) == 3, "Hash table size correct");

  int* val1 = sp_ht_getp(ht, key1);
  TEST_ASSERT(val1 != NULL && *val1 == 1, "Hash table get for key1 correct");

  int* val2 = sp_ht_getp(ht, key2);
  TEST_ASSERT(val2 != NULL && *val2 == 2, "Hash table get for key2 correct");

  sp_str_t missing = SP_LIT("missing");
  int* val_missing = sp_ht_getp(ht, missing);
  TEST_ASSERT(val_missing == NULL, "Hash table returns NULL for missing key");

  sp_ht_free(ht);
  TEST_ASSERT(1, "Hash table free completes");
}

// ============================================================================
// File I/O Tests (Emscripten, WASI only)
// ============================================================================

#if defined(SP_ENABLE_IO_FILE)
void test_file_io(void) {
  TEST_PRINT("\n=== File I/O Tests ===\n");

  // Test memory stream (always available)
  char buffer[256] = {0};
  sp_io_stream_t mem_stream = sp_io_from_memory(buffer, sizeof(buffer));

  const char* test_data = "Hello, sp_io!";
  u64 written = sp_io_write(&mem_stream, test_data, 13);
  TEST_ASSERT(written == 13, "Memory stream write works");

  sp_io_seek(&mem_stream, 0, SP_IO_SEEK_SET);
  char read_buffer[64] = {0};
  u64 read_count = sp_io_read(&mem_stream, read_buffer, 13);
  TEST_ASSERT(read_count == 13, "Memory stream read works");
  TEST_ASSERT(memcmp(read_buffer, test_data, 13) == 0, "Memory stream data correct");

  s64 size = sp_io_size(&mem_stream);
  TEST_ASSERT(size == 256, "Memory stream size correct");

  sp_io_close(&mem_stream);
  TEST_ASSERT(1, "Memory stream close works");

#if !defined(SP_WASM_FREESTANDING)
  // Test file stream (if filesystem available)
  sp_str_t test_path = SP_LIT("/tmp/sp_wasm_test.txt");
  sp_io_stream_t file_out = sp_io_from_file(test_path, SP_IO_MODE_WRITE);

  if (file_out.file.fd >= 0) {
    const char* file_data = "File test data";
    sp_io_write(&file_out, file_data, 14);
    sp_io_close(&file_out);

    sp_io_stream_t file_in = sp_io_from_file(test_path, SP_IO_MODE_READ);
    if (file_in.file.fd >= 0) {
      char file_buffer[64] = {0};
      sp_io_read(&file_in, file_buffer, 14);
      TEST_ASSERT(memcmp(file_buffer, file_data, 14) == 0, "File I/O round-trip works");
      sp_io_close(&file_in);
    }
  } else {
    TEST_PRINT("[SKIP] File I/O tests (no filesystem access)\n");
  }
#endif
}
#endif

// ============================================================================
// Timer Tests (Native and Emscripten)
// ============================================================================

#if defined(SP_ENABLE_TIMER)
void test_timer(void) {
  TEST_PRINT("\n=== Timer Tests ===\n");

  sp_tm_timer_t timer = sp_tm_start_timer();
  TEST_ASSERT(1, "sp_tm_start_timer works");

  // Read timer
  u64 elapsed1 = sp_tm_read_timer(&timer);
  // Do some work
  volatile int sum = 0;
  for (int i = 0; i < 10000; i++) sum += i;
  u64 elapsed2 = sp_tm_read_timer(&timer);

  TEST_ASSERT(elapsed2 >= elapsed1, "Timer elapsed increases");

  // Test lap timer
  u64 lap = sp_tm_lap_timer(&timer);
  TEST_ASSERT(lap >= 0, "Lap timer returns value");

  // Test reset
  sp_tm_reset_timer(&timer);
  u64 after_reset = sp_tm_read_timer(&timer);
  TEST_ASSERT(after_reset < elapsed2, "Timer reset works");

  // Test time conversions
  u64 ms = sp_tm_s_to_ms(1);
  TEST_ASSERT(ms == 1000, "sp_tm_s_to_ms works");

  u64 ns = sp_tm_ms_to_ns(1);
  TEST_ASSERT(ns == 1000000, "sp_tm_ms_to_ns works");
}
#endif

// ============================================================================
// String Builder and Format Tests
// ============================================================================

void test_string_builder(void) {
  TEST_PRINT("\n=== String Builder Tests ===\n");

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_str_builder_append(&builder, SP_LIT("Hello"));
  sp_str_builder_append(&builder, SP_LIT(", "));
  sp_str_builder_append(&builder, SP_LIT("World!"));

  sp_str_t result = sp_str_builder_write(&builder);
  TEST_ASSERT(result.len == 13, "String builder length correct");
  TEST_ASSERT(sp_str_equal(result, SP_LIT("Hello, World!")), "String builder content correct");

  // Test append_cstr
  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_builder_append_cstr(&builder2, "C string");
  sp_str_t result2 = sp_str_builder_write(&builder2);
  TEST_ASSERT(sp_str_equal(result2, SP_LIT("C string")), "String builder append_cstr works");

  // Test append_c8
  sp_str_builder_t builder3 = SP_ZERO_INITIALIZE();
  sp_str_builder_append_c8(&builder3, 'A');
  sp_str_builder_append_c8(&builder3, 'B');
  sp_str_builder_append_c8(&builder3, 'C');
  sp_str_t result3 = sp_str_builder_write(&builder3);
  TEST_ASSERT(sp_str_equal(result3, SP_LIT("ABC")), "String builder append_c8 works");
}

#if !defined(SP_WASM_FREESTANDING)
void test_format(void) {
  TEST_PRINT("\n=== Format Tests ===\n");

  sp_str_t str1 = sp_format("Value: {}", SP_FMT_S32(42));
  TEST_ASSERT(sp_str_contains(str1, SP_LIT("42")), "sp_format with s32 works");

  sp_str_t str2 = sp_format("Name: {}", SP_FMT_CSTR("test"));
  TEST_ASSERT(sp_str_contains(str2, SP_LIT("test")), "sp_format with cstr works");

  sp_str_t hello = SP_LIT("Hello");
  sp_str_t str3 = sp_format("Greeting: {}", SP_FMT_STR(hello));
  TEST_ASSERT(sp_str_contains(str3, SP_LIT("Hello")), "sp_format with str works");
}
#endif

// ============================================================================
// Main Entry Point
// ============================================================================

#if defined(SP_WASM_FREESTANDING)
// For freestanding, export test function
__attribute__((export_name("run_tests")))
int run_tests(void) {
  TEST_PRINT("cum");
#else
int main(void) {
#endif
  TEST_PRINT("==============================================\n");
  TEST_PRINT("sp.h WASM Test Suite\n");
  TEST_PRINT("==============================================\n");

#if defined(SP_WASM_EMSCRIPTEN)
  TEST_PRINT("Target: Emscripten\n");
#elif defined(SP_WASM_WASI)
  TEST_PRINT("Target: WASI\n");
#elif defined(SP_WASM_FREESTANDING)
  TEST_PRINT("Target: Freestanding WASM\n");
#else
  TEST_PRINT("Target: Native\n");
#endif

  // Core tests (all targets)
  test_memory_allocation();
  test_context_and_allocator();
  test_string_operations();
  test_hash_functions();
  test_math_functions();
  test_dynamic_array();
  test_fixed_array();
  test_hash_table();
  test_string_builder();

  // Feature-specific tests
#if defined(SP_ENABLE_IO_FILE)
  test_file_io();
#endif

#if defined(SP_ENABLE_TIMER)
  test_timer();
#endif

#if !defined(SP_WASM_FREESTANDING)
  test_format();
#endif

  TEST_PRINT("\n==============================================\n");
  TEST_PRINT("Results: %d/%d tests passed\n", pass_count, test_count);
  TEST_PRINT("==============================================\n");

  return (pass_count == test_count) ? 0 : 1;
}
