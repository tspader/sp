#define _CRT_SECURE_NO_WARNINGS
#define SP_IMPLEMENTATION
#define SP_TEST
#define SP_NO_STDLIB
#define SP_NO_WINDOW_H
#include "common/sp.h"


#ifdef __cplusplus
  template<typename T>
  T make_sure_we_are_compiling_as_cpp() {
    return T();
  }
#endif

#include "utest/utest.h"

// Test utilities
static c8* sp_test_generate_random_filename() {
  static u32 counter = 0;
  c8* filename = (c8*)sp_alloc(64);
  unsigned int rand_val;
  rand_s(&rand_val);
  snprintf(filename, 64, "test_file_%u_%u.tmp", rand_val, counter++);
  return filename;
}

static void sp_test_create_file(const c8* filename, const c8* content) {
  FILE* file = fopen(filename, "w");
  if (file) {
  fputs(content, file);
  fclose(file);
  }
}

static void sp_test_modify_file(const c8* filename, const c8* new_content) {
  FILE* file = fopen(filename, "w");
  if (file) {
  fputs(new_content, file);
  fclose(file);
  }
}

static void sp_test_delete_file(const c8* filename) {
  DeleteFileA(filename);
}

static bool sp_test_file_exists(const c8* filename) {
  DWORD attrs = GetFileAttributesA(filename);
  return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

void sp_test_use_malloc() {
  static sp_allocator_malloc_t malloc_allocator = SP_ZERO_INITIALIZE();
  static sp_allocator_t allocator = SP_ZERO_INITIALIZE();

  sp_context = &sp_context_stack[0];
  *sp_context = SP_ZERO_STRUCT(sp_context_t);
  allocator = sp_allocator_malloc_init(&malloc_allocator);
  sp_context_push_allocator(&allocator);
}

void sp_test_use_bump_allocator(u32 capacity) {
  static sp_bump_allocator_t bump_allocator;
  static sp_allocator_t allocator;

  bump_allocator = SP_ZERO_STRUCT(sp_bump_allocator_t);
  allocator = SP_ZERO_STRUCT(sp_allocator_t);

  sp_context = &sp_context_stack[0];
  *sp_context = SP_ZERO_STRUCT(sp_context_t);
  allocator = sp_bump_allocator_init(&bump_allocator, capacity);
  sp_context_push_allocator(&allocator);
}

// File monitor test data
typedef struct sp_test_file_monitor_data {
  bool change_detected;
  sp_file_change_event_t last_event;
  c8 last_file_path[SP_MAX_PATH_LEN];
} sp_test_file_monitor_data;

static void sp_test_file_monitor_callback(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
  sp_test_file_monitor_data* data = (sp_test_file_monitor_data*)userdata;
  data->change_detected = true;
  data->last_event = change->events;
  sp_str_copy_to(change->file_path, data->last_file_path, SP_MAX_PATH_LEN);
}

UTEST(file_monitor, detects_file_modifications) {

  sp_test_use_malloc();
  
  // Create a test file
  c8* test_filename = sp_test_generate_random_filename();
  sp_test_create_file(test_filename, "Initial content");
  
  // Set up file monitor
  sp_test_file_monitor_data test_data = {};
  sp_file_monitor_t monitor = {};
  sp_file_monitor_init(&monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_MODIFIED, &test_data);
  
  // Get current directory and add it to monitor
  c8 current_dir[SP_MAX_PATH_LEN] = {};
  GetCurrentDirectoryA(SP_MAX_PATH_LEN, current_dir);
  sp_str_t current_dir_str = sp_str_cstr(current_dir);
  sp_file_monitor_add_directory(&monitor, current_dir_str);
  
  // Process any initial changes
  sp_file_monitor_process_changes(&monitor);
  test_data.change_detected = false;
  
  // Modify the file
  sp_test_modify_file(test_filename, "Modified content");
  
  // Wait a bit for the file system to register the change
  Sleep(100);
  
  // Process changes
  sp_file_monitor_process_changes(&monitor);

  // Check that the change was detected
  ASSERT_TRUE(test_data.change_detected);
  ASSERT_EQ(test_data.last_event, SP_FILE_CHANGE_EVENT_MODIFIED);
  ASSERT_NE(strstr(test_data.last_file_path, test_filename), NULL);
  
  // Clean up
  sp_test_delete_file(test_filename);
  sp_free(test_filename);
}

// Dynamic array test utilities
typedef struct sp_test_memory_tracker {
  sp_bump_allocator_t* bump;
  sp_allocator_t* allocator;
} sp_test_memory_tracker;

static void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity) {
  sp_test_use_bump_allocator(capacity);
  tracker->bump = (sp_bump_allocator_t*)sp_context->allocator->user_data;
  tracker->allocator = sp_context->allocator;
}

static u32 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker) {
  return tracker->bump->bytes_used;
}

static void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker) {
  sp_bump_allocator_clear(tracker->bump);
}

static void sp_test_memory_tracker_destroy(sp_test_memory_tracker* tracker) {
  sp_bump_allocator_destroy(tracker->bump);
}

UTEST(dynamic_array, initialization) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024); // 1MB
  
  // Test default initialization
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    ASSERT_EQ(arr.size, 0);
    ASSERT_EQ(arr.capacity, 2);
    ASSERT_EQ(arr.element_size, sizeof(s32));
    ASSERT_NE(arr.data, NULL);
  }
  
  // Test different element sizes
  {
  struct test_sizes {
      u32 size;
      const c8* name;
    } sizes[] = {
      {1, "u8"},
      {4, "s32"},
      {8, "f64"},
      {16, "vec4"},
      {64, "cache_line"},
      {256, "large_struct"}
    };
    
    for (u32 test_idx = 0; test_idx < sizeof(sizes)/sizeof(sizes[0]); test_idx++) {
      struct test_sizes* test = &sizes[test_idx];
      sp_dynamic_array_t arr;
      sp_dynamic_array_init(&arr, test->size);
      
      ASSERT_EQ(arr.element_size, test->size);
      ASSERT_EQ(arr.capacity, 2);
      ASSERT_EQ(arr.size, 0);
      
      // Verify memory allocation
      u32 expected_alloc = test->size * 2; // capacity * element_size
      ASSERT_GE(sp_test_memory_tracker_bytes_used(&tracker), expected_alloc);
      
      sp_test_memory_tracker_clear(&tracker);
    }
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, push_operations) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);
  
  // Test push single elements
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    // Push first element
    s32 val1 = 42;
    u8* elem1 = sp_dynamic_array_push(&arr, &val1);
    ASSERT_NE(elem1, NULL);
    ASSERT_EQ(arr.size, 1);
    ASSERT_EQ(*(s32*)elem1, 42);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 42);
    
    // Push second element
    s32 val2 = 69;
    u8* elem2 = sp_dynamic_array_push(&arr, &val2);
    ASSERT_NE(elem2, NULL);
    ASSERT_EQ(arr.size, 2);
    ASSERT_EQ(*(s32*)elem2, 69);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 1), 69);
    
    // Verify first element unchanged
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 42);
  }
  
  // Test push with NULL data
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    u8* elem = sp_dynamic_array_push(&arr, NULL);
    ASSERT_NE(elem, NULL);
    ASSERT_EQ(arr.size, 1);
    // Element exists but uninitialized
  }
  
  // Test push_n multiple elements
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    s32 values[] = {10, 20, 30, 40, 50};
    u8* elems = sp_dynamic_array_push_n(&arr, values, 5);
    
    ASSERT_NE(elems, NULL);
    ASSERT_EQ(arr.size, 5);
    
    // Verify all elements
    for (u32 i = 0; i < 5; i++) {
      ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, i), values[i]);
    }
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, growth) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);
  
  // Test automatic growth on push
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    ASSERT_EQ(arr.capacity, 2);
    
    // Push beyond initial capacity
    s32 values[] = {1, 2, 3};
    for (s32 i = 0; i < 3; i++) {
      sp_dynamic_array_push(&arr, &values[i]);
    }
    
    ASSERT_EQ(arr.size, 3);
    ASSERT_GE(arr.capacity, 3);
    
    // Verify data preserved after growth
    for (u32 i = 0; i < 3; i++) {
      ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, i), values[i]);
    }
  }
  
  // Test manual growth
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    u32 old_cap = arr.capacity;
    sp_dynamic_array_grow(&arr, 10);
    
    ASSERT_GE(arr.capacity, 10);
    ASSERT_EQ(arr.size, 0); // Size unchanged
    
    // Growth should at least double or meet requirement
    sp_dynamic_array_grow(&arr, 5);
    ASSERT_GE(arr.capacity, 10); // No shrinking
  }
  
  // Test growth preserves data
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    // Fill array
    for (s32 i = 0; i < 10; i++) {
      sp_dynamic_array_push(&arr, &i);
    }
    
    u32 old_cap = arr.capacity;
    u32 old_bytes = sp_test_memory_tracker_bytes_used(&tracker);
    
    // Force growth
    sp_dynamic_array_grow(&arr, 100);
    
    ASSERT_GE(arr.capacity, 100);
    ASSERT_GT(sp_test_memory_tracker_bytes_used(&tracker), old_bytes);
    
    // Verify all data preserved
    for (u32 i = 0; i < 10; i++) {
      ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, i), i);
    }
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, reserve) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);
  
  sp_dynamic_array_t arr;
  sp_dynamic_array_init(&arr, sizeof(s32));
  
  // Test reserve on empty array
  {
    u8* reserved = sp_dynamic_array_reserve(&arr, 5);
    ASSERT_NE(reserved, NULL);
    ASSERT_EQ(arr.size, 5);
    ASSERT_GE(arr.capacity, 5);
  }
  
  // Test reserve triggers growth
  {
    // Reset array
    sp_dynamic_array_clear(&arr);
    
    // Fill to capacity
    s32 val = 1;
    sp_dynamic_array_push(&arr, &val);
    sp_dynamic_array_push(&arr, &val);
    
    ASSERT_EQ(arr.size, 2);
    ASSERT_GE(arr.capacity, 2);
    
    // Reserve more
    u8* reserved = sp_dynamic_array_reserve(&arr, 3);
    ASSERT_NE(reserved, NULL);
    ASSERT_EQ(arr.size, 5);
    ASSERT_GE(arr.capacity, 5);
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, clear_and_reuse) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);
  
  sp_dynamic_array_t arr;
  sp_dynamic_array_init(&arr, sizeof(s32));
  
  // Add elements
  for (s32 i = 0; i < 10; i++) {
    sp_dynamic_array_push(&arr, &i);
  }
  
  u32 old_cap = arr.capacity;
  
  // Clear
  sp_dynamic_array_clear(&arr);
  ASSERT_EQ(arr.size, 0);
  ASSERT_EQ(arr.capacity, old_cap); // Capacity unchanged
  ASSERT_NE(arr.data, NULL);
  
  // Reuse
  s32 val = 99;
  sp_dynamic_array_push(&arr, &val);
  ASSERT_EQ(arr.size, 1);
  ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 99);
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, byte_size) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);
  
  // Test various element sizes
  {
    struct test_case {
      u32 elem_size;
      u32 count;
    } cases[] = {
      {1, 10},
      {4, 25},
      {8, 13},
      {64, 7}
    };
    
    for (u32 tc_idx = 0; tc_idx < sizeof(cases)/sizeof(cases[0]); tc_idx++) {
      struct test_case* tc = &cases[tc_idx];
      sp_dynamic_array_t arr;
      sp_dynamic_array_init(&arr, tc->elem_size);
      
      for (u32 i = 0; i < tc->count; i++) {
      sp_dynamic_array_push(&arr, NULL);
      }
      
      u32 expected = tc->elem_size * tc->count;
      ASSERT_EQ(sp_dynamic_array_byte_size(&arr), expected);
      
      sp_test_memory_tracker_clear(&tracker);
    }
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, edge_cases) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);
  
  // Test zero elements after clear
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    sp_dynamic_array_clear(&arr);
    ASSERT_EQ(sp_dynamic_array_byte_size(&arr), 0);
  }
  
  // Test access patterns
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    // Add elements
    for (s32 i = 0; i < 10; i++) {
      sp_dynamic_array_push(&arr, &i);
    }
    
    // Sequential access
    for (u32 i = 0; i < 10; i++) {
      s32* elem = (s32*)sp_dynamic_array_at(&arr, i);
      ASSERT_EQ(*elem, i);
    }
    
    // Boundary access
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 0);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 9), 9);
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, stress_test) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 128 * 1024 * 1024); // 128MB for stress test
  
  // Test millions of operations
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    const s32 iterations = 1000000;
    
    // Push a million elements
    for (s32 i = 0; i < iterations; i++) {
      sp_dynamic_array_push(&arr, &i);
    }
    
    ASSERT_EQ(arr.size, iterations);
    
    // Verify sampling
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 0);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, iterations/2), iterations/2);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, iterations-1), iterations-1);
    
    // Clear and reuse
    sp_dynamic_array_clear(&arr);
    ASSERT_EQ(arr.size, 0);
    
    // Push again
    for (s32 i = 0; i < 1000; i++) {
      sp_dynamic_array_push(&arr, &i);
    }
    ASSERT_EQ(arr.size, 1000);
  }
  
  // Test random operations
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));
    
    // Mix of operations
    for (u32 i = 0; i < 10000; i++) {
      u32 op = i % 4;
      switch (op) {
      case 0: { // Push
          s32 val = i;
          sp_dynamic_array_push(&arr, &val);
          break;
      }
      case 1: { // Push_n
          s32 vals[10];
          for (u32 j = 0; j < 10; j++) vals[j] = i + j;
          sp_dynamic_array_push_n(&arr, vals, 10);
          break;
      }
      case 2: { // Reserve
          if (arr.size < 100000) {
          sp_dynamic_array_reserve(&arr, 5);
          }
          break;
      }
      case 3: { // Clear periodically
          if (i % 1000 == 0 && i > 0) {
          sp_dynamic_array_clear(&arr);
          }
          break;
      }
      }
    }
    
    // Array should still be functional
    s32 final_val = 9999;
    sp_dynamic_array_push(&arr, &final_val);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, arr.size - 1), 9999);
  }
  
  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(formatter, basic) {
  sp_test_use_malloc();
  
  // Test simple format with one argument
  u32 answer = 69;
  sp_str_t result = sp_fmt(sp_str_lit("answer: {}"), SP_FMT_U32(answer));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("answer: 69")));
}

UTEST(formatter, numeric_types) {
  sp_test_use_malloc();
  
  // Test unsigned integers
  u8 u8_val = 255;
  sp_str_t result = sp_fmt(sp_str_lit("u8: {}"), SP_FMT_U8(u8_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("u8: 255")));
  
  u16 u16_val = 65535;
  result = sp_fmt(sp_str_lit("u16: {}"), SP_FMT_U16(u16_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("u16: 65535")));
  
  u32 u32_val = 1234567890;
  result = sp_fmt(sp_str_lit("u32: {}"), SP_FMT_U32(u32_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("u32: 1234567890")));
  
  u64 u64_val = 9876543210ULL;
  result = sp_fmt(sp_str_lit("u64: {}"), SP_FMT_U64(u64_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("u64: 9876543210")));
  
  // Test signed integers
  s8 s8_val = -128;
  result = sp_fmt(sp_str_lit("s8: {}"), SP_FMT_S8(s8_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("s8: -128")));
  
  s16 s16_val = -32768;
  result = sp_fmt(sp_str_lit("s16: {}"), SP_FMT_S16(s16_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("s16: -32768")));
  
  s32 s32_val = -2147483647;
  result = sp_fmt(sp_str_lit("s32: {}"), SP_FMT_S32(s32_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("s32: -2147483647")));
  
  s64 s64_val = -9223372036854775807LL;
  result = sp_fmt(sp_str_lit("s64: {}"), SP_FMT_S64(s64_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("s64: -9223372036854775807")));
  
  // Test zero values
  u32 zero = 0;
  result = sp_fmt(sp_str_lit("zero: {}"), SP_FMT_U32(zero));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("zero: 0")));
}

UTEST(formatter, floating_point) {
  sp_test_use_malloc();
  
  f32 f32_val = 3.14159f;
  sp_str_t result = sp_fmt(sp_str_lit("f32: {}"), SP_FMT_F32(f32_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("f32: 3.141")));
  
  f64 f64_val = -2.71828;
  result = sp_fmt(sp_str_lit("f64: {}"), SP_FMT_F64(f64_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("f64: -2.718")));
  
  f32 f32_zero = 0.0f;
  result = sp_fmt(sp_str_lit("f32 zero: {}"), SP_FMT_F32(f32_zero));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("f32 zero: 0.000")));
  
  f32 f32_int = 42.0f;
  result = sp_fmt(sp_str_lit("f32 int: {}"), SP_FMT_F32(f32_int));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("f32 int: 42.000")));
}

UTEST(formatter, string_types) {
  sp_test_use_malloc();
  
  sp_str_t str_val = sp_str_lit("hello world");
  sp_str_t result = sp_fmt(sp_str_lit("str: {}"), SP_FMT_STR(str_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("str: \"hello world\"")));
  
  const c8* cstr_val = "c string";
  result = sp_fmt(sp_str_lit("cstr: {}"), SP_FMT_CSTR(cstr_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("cstr: \"c string\"")));
}

UTEST(formatter, character_types) {
  sp_test_use_malloc();
  
  c8 c8_val = 'A';
  sp_str_t result = sp_fmt(sp_str_lit("c8: {}"), SP_FMT_C8(c8_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("c8: 'A'")));
  
  c16 c16_val = 'Z';
  result = sp_fmt(sp_str_lit("c16: {}"), SP_FMT_C16(c16_val));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("c16: 'Z'")));
  
  // Test non-ASCII c16
  c16 c16_unicode = 0x1234;
  result = sp_fmt(sp_str_lit("c16 unicode: {}"), SP_FMT_C16(c16_unicode));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("c16 unicode: 'U+1234'")));
}

UTEST(formatter, pointer_type) {
  sp_test_use_malloc();
  
  void* ptr = (void*)0xDEADBEEF;
  sp_str_t result = sp_fmt(sp_str_lit("ptr: {}"), SP_FMT_PTR(ptr));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("ptr: 0xDEADBEEF")));
  
  void* null_ptr = NULL;
  result = sp_fmt(sp_str_lit("null: {}"), SP_FMT_PTR(null_ptr));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("null: 0x00000000")));
}

UTEST(formatter, hash_type) {
  sp_test_use_malloc();
  
  sp_hash_t hash = 0xABCDEF12;
  sp_str_t result = sp_fmt(sp_str_lit("hash: {}"), SP_FMT_HASH(hash));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("hash: ABCDEF12")));
  
  sp_hash_t zero_hash = 0;
  result = sp_fmt(sp_str_lit("zero hash: {}"), SP_FMT_HASH(zero_hash));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("zero hash: 0")));
}

UTEST(formatter, array_types) {
  sp_test_use_malloc();
  
  sp_fixed_array_t fixed_arr;
  fixed_arr.size = 10;
  fixed_arr.capacity = 20;
  fixed_arr.element_size = 4;
  fixed_arr.data = NULL;
  
  sp_str_t result = sp_fmt(sp_str_lit("fixed: {}"), SP_FMT_FIXED_ARRAY(fixed_arr));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("fixed: { size: 10, capacity: 20 }")));
  
  sp_dynamic_array_t dyn_arr;
  dyn_arr.size = 5;
  dyn_arr.capacity = 16;
  dyn_arr.element_size = 8;
  dyn_arr.data = NULL;
  
  result = sp_fmt(sp_str_lit("dynamic: {}"), SP_FMT_DYNAMIC_ARRAY(dyn_arr));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("dynamic: { size: 5, capacity: 16 }")));
}

UTEST(formatter, date_time) {
  sp_test_use_malloc();
  
  sp_os_date_time_t dt;
  dt.year = 2024;
  dt.month = 12;
  dt.day = 25;
  dt.hour = 14;
  dt.minute = 30;
  dt.second = 45;
  dt.millisecond = 123;
  
  sp_str_t result = sp_fmt(sp_str_lit("datetime: {}"), SP_FMT_DATE_TIME(dt));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("datetime: 2024-12-25T14:30:45.123")));
  
  // Test without milliseconds
  dt.millisecond = 0;
  result = sp_fmt(sp_str_lit("datetime no ms: {}"), SP_FMT_DATE_TIME(dt));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("datetime no ms: 2024-12-25T14:30:45")));
}

UTEST(formatter, multiple_args) {
  sp_test_use_malloc();
  
  u32 count = 42;
  sp_str_t name = sp_str_lit("test");
  f32 value = 3.14f;
  
  sp_str_t result = sp_fmt(sp_str_lit("Count: {}, Name: {}, Value: {}"), 
    SP_FMT_U32(count), SP_FMT_STR(name), SP_FMT_F32(value));
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("Count: 42, Name: \"test\", Value: 3.140")));
}

UTEST(sp_str_builder, basic_operations) {
  sp_test_use_malloc();
  
  // Test initialization
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  ASSERT_EQ(builder.buffer.data, NULL);
  ASSERT_EQ(builder.buffer.count, 0);
  ASSERT_EQ(builder.buffer.capacity, 0);
  
  // Test grow - should allocate enough capacity
  sp_str_builder_grow(&builder, 10);
  ASSERT_GE(builder.buffer.capacity, 10);
  ASSERT_NE(builder.buffer.data, NULL);
  ASSERT_EQ(builder.buffer.count, 0);
  
  // Test append string
  sp_str_t test_str = sp_str_lit("Hello");
  sp_str_builder_append(&builder, test_str);
  ASSERT_EQ(builder.buffer.count, 5);
  
  // Test append C string
  sp_str_builder_append_cstr(&builder, " World");
  ASSERT_EQ(builder.buffer.count, 11);
  
  // Test append character
  sp_str_builder_append_c8(&builder, '!');
  ASSERT_EQ(builder.buffer.count, 12);
  
  // Test write
  sp_str_t result = sp_str_builder_write(&builder);
  ASSERT_EQ(result.len, 12);
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("Hello World!")));
  
  // Test write_cstr
  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_builder_append_cstr(&builder2, "Test");
  c8* cstr_result = sp_str_builder_write_cstr(&builder2);
  ASSERT_TRUE(sp_cstr_equal(cstr_result, "Test"));
  sp_free(cstr_result);
}

UTEST(sp_str_builder, growth_behavior) {
  sp_test_use_malloc();
  
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  
  // Test multiple grow calls
  sp_str_builder_grow(&builder, 5);
  u32 cap1 = builder.buffer.capacity;
  ASSERT_GE(cap1, 5);
  
  sp_str_builder_grow(&builder, 10);
  u32 cap2 = builder.buffer.capacity;
  ASSERT_GE(cap2, 10);
  ASSERT_GE(cap2, cap1);
  
  // Test automatic growth on append
  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_t long_str = sp_str_lit("This is a much longer string that will trigger growth");
  sp_str_builder_append(&builder2, long_str);
  ASSERT_GE(builder2.buffer.capacity, long_str.len);
  ASSERT_EQ(builder2.buffer.count, long_str.len);
}

UTEST(sp_str_builder, edge_cases) {
  sp_test_use_malloc();
  
  // Test with empty strings
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, sp_str_lit(""));
  ASSERT_EQ(builder.buffer.count, 0);
  
  sp_str_builder_append_cstr(&builder, "");
  ASSERT_EQ(builder.buffer.count, 0);
  
  // Test NULL data string (invalid)
  sp_str_t null_str = {.len = 0, .data = NULL};
  sp_str_builder_append(&builder, null_str);
  ASSERT_EQ(builder.buffer.count, 0);
  
  // Test building a large string
  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  for (int i = 0; i < 100; i++) {
    sp_str_builder_append_cstr(&builder2, "test ");
  }
  ASSERT_EQ(builder2.buffer.count, 500); // 100 * 5
  sp_str_t result = sp_str_builder_write(&builder2);
  ASSERT_EQ(result.len, 500);
}

UTEST(sp_cstr_copy, all_variations) {
  sp_test_use_malloc();
  
  // Test sp_cstr_copy
  const c8* original = "Hello World";
  c8* copy = sp_cstr_copy(original);
  ASSERT_TRUE(sp_cstr_equal(copy, original));
  ASSERT_NE(copy, original); // Different pointers
  sp_free(copy);
  
  // Test sp_cstr_copy_n
  c8* partial = sp_cstr_copy_n(original, 5);
  ASSERT_TRUE(sp_cstr_equal(partial, "Hello"));
  sp_free(partial);
  
  // Test sp_cstr_copy_c8
  c8* copy_c8 = sp_cstr_copy_c8(original, 5);
  ASSERT_TRUE(sp_cstr_equal(copy_c8, "Hello"));
  sp_free(copy_c8);
  
  // Test with empty string
  const c8* empty = "";
  c8* empty_copy = sp_cstr_copy(empty);
  ASSERT_TRUE(sp_cstr_equal(empty_copy, ""));
  sp_free(empty_copy);
  
  // Test with NULL - sp_cstr_len(NULL) returns 0, so it allocates 1 byte for null terminator
  c8* null_copy = sp_cstr_copy(NULL);
  ASSERT_EQ(null_copy[0], '\0');
  sp_free(null_copy);
}

UTEST(sp_cstr_copy_to, buffer_operations) {
  sp_test_use_malloc();
  
  // Test sp_cstr_copy_to with sufficient buffer
  const c8* source = "Hello World";
  c8 buffer[20];
  sp_os_zero_memory(buffer, 20);
  sp_cstr_copy_to(source, buffer, 20);
  ASSERT_TRUE(sp_cstr_equal(buffer, source));
  
  // Test with exact size buffer
  c8 exact[12]; // "Hello World" + null
  sp_os_zero_memory(exact, 12);
  sp_cstr_copy_to(source, exact, 12);
  ASSERT_TRUE(sp_cstr_equal(exact, source));
  
  // Test with insufficient buffer
  char small_buffer[6];
  sp_os_zero_memory(small_buffer, 6);
  sp_cstr_copy_to(source, small_buffer, 6);
  ASSERT_TRUE(sp_cstr_equal(small_buffer, "Hello")); // Should be truncated
   
  // Test sp_cstr_copy_to_n
  c8 partial_buffer[10];
  sp_os_zero_memory(partial_buffer, 10);
  sp_cstr_copy_to_n(source, 5, partial_buffer, 10);
  ASSERT_TRUE(sp_cstr_equal(partial_buffer, "Hello"));
  
  // Test with NULL source - buffer should remain unchanged
  c8 null_buffer[10];
  sp_cstr_copy_to("test", null_buffer, 10);
  sp_cstr_copy_to(NULL, null_buffer, 10);
  ASSERT_TRUE(sp_cstr_equal(null_buffer, "test")); // Should remain unchanged
  
  // Test with NULL buffer
  sp_cstr_copy_to(source, NULL, 10); // Should not crash
  
  // Test with zero buffer length
  c8 zero_buffer[10] = "unchanged";
  sp_cstr_copy_to(source, zero_buffer, 0);
  ASSERT_TRUE(sp_cstr_equal(zero_buffer, "unchanged"));
}

UTEST(sp_cstr_equal, comparison_tests) {
  sp_test_use_malloc();
  
  // Test equal strings
  ASSERT_TRUE(sp_cstr_equal("Hello", "Hello"));
  ASSERT_TRUE(sp_cstr_equal("", ""));
  
  // Test different strings
  ASSERT_FALSE(sp_cstr_equal("Hello", "World"));
  ASSERT_FALSE(sp_cstr_equal("Hello", "Hello!"));
  ASSERT_FALSE(sp_cstr_equal("Hello", "Hell"));
  
  // Test with NULL - both should have len 0 so they're equal
  ASSERT_TRUE(sp_cstr_equal(NULL, NULL));
  // One NULL and one non-NULL will have different lengths
  ASSERT_FALSE(sp_cstr_equal("Hello", NULL));
  ASSERT_FALSE(sp_cstr_equal(NULL, "Hello"));
  
  // Test case sensitivity
  ASSERT_FALSE(sp_cstr_equal("Hello", "hello"));
}

UTEST(sp_cstr_len, length_tests) {
  sp_test_use_malloc();
  
  // Test normal strings
  ASSERT_EQ(sp_cstr_len("Hello"), 5);
  ASSERT_EQ(sp_cstr_len("Hello World!"), 12);
  ASSERT_EQ(sp_cstr_len(""), 0);
  
  // Test with NULL
  ASSERT_EQ(sp_cstr_len(NULL), 0);
  
  // Test with embedded nulls (should stop at first null)
  const c8 embedded[] = {'H', 'e', '\0', 'l', 'o', '\0'};
  ASSERT_EQ(sp_cstr_len(embedded), 2);
}

UTEST(sp_wstr_to_cstr, wide_string_conversion) {
  sp_test_use_malloc();
  
  // Test basic conversion
  c16 wide_str[] = L"Hello";
  c8* converted = sp_wstr_to_cstr(wide_str, 5);
  ASSERT_TRUE(sp_cstr_equal(converted, "Hello"));
  sp_free(converted);
  
  // Test empty string
  c16 empty[] = L"";
  c8* empty_converted = sp_wstr_to_cstr(empty, 0);
  ASSERT_TRUE(sp_cstr_equal(empty_converted, ""));
  sp_free(empty_converted);
  
  // Test with special characters
  c16 special[] = L"Test 123!";
  c8* special_converted = sp_wstr_to_cstr(special, 9);
  ASSERT_TRUE(sp_cstr_equal(special_converted, "Test 123!"));
  sp_free(special_converted);
}

UTEST(sp_str_to, conversion_functions) {
  sp_test_use_malloc();
  
  // Test sp_str_to_cstr
  sp_str_t str = sp_str_lit("Hello World");
  c8* cstr = sp_str_to_cstr(str);
  ASSERT_TRUE(sp_cstr_equal(cstr, "Hello World"));
  sp_free(cstr);
  
  // Test sp_str_to_cstr_ex (same as sp_str_to_cstr)
  c8* cstr_ex = sp_str_to_cstr_ex(str);
  ASSERT_TRUE(sp_cstr_equal(cstr_ex, "Hello World"));
  sp_free(cstr_ex);
  
  // Test sp_str_to_double_null_terminated
  sp_str_t path = sp_str_lit("C:\\test");
  c8* double_null = sp_str_to_double_null_terminated(path);
  ASSERT_EQ(double_null[7], '\0'); // First null
  ASSERT_EQ(double_null[8], '\0'); // Second null
  sp_free(double_null);
  
  // Test with empty string
  sp_str_t empty = sp_str_lit("");
  c8* empty_cstr = sp_str_to_cstr(empty);
  ASSERT_TRUE(sp_cstr_equal(empty_cstr, ""));
  sp_free(empty_cstr);
}

UTEST(sp_str_copy, string_copy_operations) {
  sp_test_use_malloc();
  
  // Test sp_str_copy
  sp_str_t original = sp_str_lit("Hello World");
  sp_str_t copy = sp_str_copy(original);
  ASSERT_EQ(copy.len, original.len);
  ASSERT_TRUE(sp_str_equal(copy, original));
  ASSERT_NE(copy.data, original.data); // Different pointers
  
  // Test sp_str_copy_cstr
  sp_str_t from_cstr = sp_str_copy_cstr("Test String");
  ASSERT_EQ(from_cstr.len, 11);
  ASSERT_TRUE(sp_str_equal_cstr(from_cstr, "Test String"));
  
  // Test sp_str_copy_cstr_n - now correctly uses the length parameter
  sp_str_t partial = sp_str_copy_cstr_n("Hello World", 5);
  ASSERT_EQ(partial.len, 5);
  ASSERT_TRUE(sp_str_equal(partial, sp_str_lit("Hello")));
  
  // Test sp_str_copy_to_str
  sp_str_t dest = sp_str_alloc(20);
  sp_str_copy_to_str(original, &dest, 20);
  ASSERT_EQ(dest.len, original.len);
  ASSERT_TRUE(sp_str_equal(dest, original));
  
  // Test sp_str_copy_to
  c8 buffer[20];
  sp_os_zero_memory(buffer, 20);
  sp_str_copy_to(original, buffer, 20);
  ASSERT_TRUE(sp_os_is_memory_equal(buffer, original.data, original.len));
  
  // Test with truncation
  c8 small_buffer[5];
  sp_os_zero_memory(small_buffer, 5);
  sp_str_copy_to(original, small_buffer, 5);
  ASSERT_TRUE(sp_os_is_memory_equal(small_buffer, "Hello", 5));
}

UTEST(sp_str, string_creation) {
  sp_test_use_malloc();
  
  // Test sp_str macro
  sp_str_t str1 = sp_str("Hello", 5);
  ASSERT_EQ(str1.len, 5);
  ASSERT_EQ(str1.data[0], 'H');
  
  // Test sp_str_lit macro
  sp_str_t str2 = sp_str_lit("World");
  ASSERT_EQ(str2.len, 5);
  ASSERT_TRUE(sp_str_equal_cstr(str2, "World"));
  
  // Test sp_str_cstr macro
  const c8* cstr = "Dynamic";
  sp_str_t str3 = sp_str_cstr(cstr);
  ASSERT_EQ(str3.len, 7);
  ASSERT_TRUE(sp_str_equal_cstr(str3, "Dynamic"));
  
  // Test sp_str_alloc
  sp_str_t allocated = sp_str_alloc(100);
  ASSERT_EQ(allocated.len, 0);
  ASSERT_NE(allocated.data, NULL);
}

UTEST(sp_str_equal, string_comparison) {
  sp_test_use_malloc();
  
  // Test sp_str_equal
  sp_str_t str1 = sp_str_lit("Hello");
  sp_str_t str2 = sp_str_lit("Hello");
  sp_str_t str3 = sp_str_lit("World");
  sp_str_t str4 = sp_str_lit("Hell");
  
  ASSERT_TRUE(sp_str_equal(str1, str2));
  ASSERT_FALSE(sp_str_equal(str1, str3));
  ASSERT_FALSE(sp_str_equal(str1, str4));
  
  // Test sp_str_equal_cstr
  ASSERT_TRUE(sp_str_equal_cstr(str1, "Hello"));
  ASSERT_FALSE(sp_str_equal_cstr(str1, "World"));
  ASSERT_FALSE(sp_str_equal_cstr(str1, "Hell"));
  
  // Test empty strings
  sp_str_t empty1 = sp_str_lit("");
  sp_str_t empty2 = sp_str_lit("");
  ASSERT_TRUE(sp_str_equal(empty1, empty2));
  ASSERT_TRUE(sp_str_equal_cstr(empty1, ""));
  
  // Test with different lengths
  sp_str_t long_str = sp_str_lit("Hello World!");
  ASSERT_FALSE(sp_str_equal(str1, long_str));
}

UTEST(sp_str_sort_kernel_alphabetical, sorting_tests) {
  sp_test_use_malloc();
  
  // Create array of strings
  sp_str_t strings[] = {
    sp_str_lit("zebra"),
    sp_str_lit("apple"),
    sp_str_lit("banana"),
    sp_str_lit("aardvark"),
    sp_str_lit("zoo")
  };
  
  // Sort using qsort
  qsort(strings, 5, sizeof(sp_str_t), sp_str_sort_kernel_alphabetical);
  
  // Verify sorted order
  ASSERT_TRUE(sp_str_equal(strings[0], sp_str_lit("aardvark")));
  ASSERT_TRUE(sp_str_equal(strings[1], sp_str_lit("apple")));
  ASSERT_TRUE(sp_str_equal(strings[2], sp_str_lit("banana")));
  ASSERT_TRUE(sp_str_equal(strings[3], sp_str_lit("zebra")));
  ASSERT_TRUE(sp_str_equal(strings[4], sp_str_lit("zoo")));
  
  // Test sp_str_compare_alphabetical directly
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("a"), sp_str_lit("b")), SP_QSORT_A_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("b"), sp_str_lit("a")), SP_QSORT_B_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("same"), sp_str_lit("same")), SP_QSORT_EQUAL);
  
  // Test with different lengths
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("ab"), sp_str_lit("abc")), SP_QSORT_A_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("abc"), sp_str_lit("ab")), SP_QSORT_B_FIRST);
}

UTEST(sp_str_utilities, valid_and_at) {
  sp_test_use_malloc();
  
  // Test sp_str_valid
  sp_str_t valid = sp_str_lit("Hello");
  sp_str_t invalid = {.len = 5, .data = NULL};
  sp_str_t empty = sp_str_lit("");
  
  ASSERT_TRUE(sp_str_valid(valid));
  ASSERT_FALSE(sp_str_valid(invalid));
  ASSERT_TRUE(sp_str_valid(empty)); // Empty but valid
  
  // Test sp_str_at
  sp_str_t str = sp_str_lit("Hello");
  ASSERT_EQ(sp_str_at(str, 0), 'H');
  ASSERT_EQ(sp_str_at(str, 1), 'e');
  ASSERT_EQ(sp_str_at(str, 4), 'o');
}

// String tests summary:
// - sp_str_builder: basic operations, growth behavior, edge cases
// - sp_cstr_copy*: all variations including NULL handling
// - sp_cstr_copy_to*: buffer operations with various sizes
// - sp_cstr_equal: comparison including NULL cases
// - sp_cstr_len: length calculation including NULL
// - sp_wstr_to_cstr: wide string conversion
// - sp_str_to_*: various conversion functions
// - sp_str_copy*: string copy operations
// - sp_str creation macros: sp_str, sp_str_lit, sp_str_cstr
// - sp_str_equal*: string comparison
// - sp_str_sort_kernel_alphabetical: sorting (has bug in sp_str_compare_alphabetical)
// - sp_str_valid and sp_str_at: utility functions

UTEST(path_functions, normalize_path) {
  sp_test_use_malloc();
  
  // Test path with backslashes
  {
    sp_str_t path = sp_str_lit("C:\\Users\\Test\\file.txt");
    sp_str_t copy = sp_str_copy(path);
    sp_os_normalize_path(copy);
    ASSERT_TRUE(sp_str_equal(copy, sp_str_lit("C:/Users/Test/file.txt")));
  }
  
  // Test path already normalized
  {
    sp_str_t path = sp_str_lit("C:/Users/Test/file.txt");
    sp_str_t copy = sp_str_copy(path);
    sp_os_normalize_path(copy);
    ASSERT_TRUE(sp_str_equal(copy, sp_str_lit("C:/Users/Test/file.txt")));
  }
  
  // Test mixed slashes
  {
    sp_str_t path = sp_str_lit("C:/Users\\Test/sub\\file.txt");
    sp_str_t copy = sp_str_copy(path);
    sp_os_normalize_path(copy);
    ASSERT_TRUE(sp_str_equal(copy, sp_str_lit("C:/Users/Test/sub/file.txt")));
  }
  
  // Test empty string
  {
    sp_str_t path = sp_str_lit("");
    sp_str_t copy = sp_str_copy(path);
    sp_os_normalize_path(copy);
    ASSERT_TRUE(sp_str_equal(copy, sp_str_lit("")));
  }
  
  // Test path ending with backslash
  {
    sp_str_t path = sp_str_lit("C:\\Users\\Test\\");
    sp_str_t copy = sp_str_copy(path);
    sp_os_normalize_path(copy);
    ASSERT_TRUE(sp_str_equal(copy, sp_str_lit("C:/Users/Test/")));
  }
}

UTEST(path_functions, parent_path) {
  sp_test_use_malloc();
  
  // Test normal path
  {
    sp_str_t path = sp_str_lit("C:/Users/Test/file.txt");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_TRUE(sp_str_equal(parent, sp_str_lit("C:/Users/Test")));
  }
  
  // Test path with trailing slash
  {
    sp_str_t path = sp_str_lit("C:/Users/Test/");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_TRUE(sp_str_equal(parent, sp_str_lit("C:/Users")));
  }
  
  // Test multiple trailing slashes
  {
    sp_str_t path = sp_str_lit("C:/Users/Test///");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_TRUE(sp_str_equal(parent, sp_str_lit("C:/Users")));
  }
  
  // Test root path
  {
    sp_str_t path = sp_str_lit("C:/");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }
  
  // Test single directory
  {
    sp_str_t path = sp_str_lit("Test");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }
  
  // Test empty string
  {
    sp_str_t path = sp_str_lit("");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }
  
  // Test Unix-style root
  {
    sp_str_t path = sp_str_lit("/");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }
  
  // Test Unix-style path
  {
    sp_str_t path = sp_str_lit("/home/user/file");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_TRUE(sp_str_equal(parent, sp_str_lit("/home/user")));
  }
}

UTEST(path_functions, canonicalize_path) {
  sp_test_use_malloc();
  
  // Test relative path with single ..
  {
    sp_str_t path = sp_str_lit("test/..");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    // Should resolve to current directory
    ASSERT_GT(canonical.len, 0);
    ASSERT_NE(canonical.data[canonical.len - 1], '/'); // Should NOT end with slash
  }
  
  // Test relative path with multiple ..
  {
    sp_str_t path = sp_str_lit("test/sub/../../another");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    // Should end with "another"
    sp_str_t filename = sp_os_extract_file_name(canonical);
    ASSERT_TRUE(sp_str_equal(filename, sp_str_lit("another")));
  }
  
  // Test absolute path
  {
    sp_str_t exe = sp_os_get_executable_path();
    sp_str_t canonical = sp_os_canonicalize_path(exe);
    ASSERT_TRUE(sp_str_equal(canonical, exe));
  }
  
  // Test path with trailing slash removal
  {
    sp_str_t path = sp_str_lit("test/");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    ASSERT_NE(canonical.data[canonical.len - 1], '/');
  }
  
  // Test empty path
  {
    sp_str_t path = sp_str_lit("");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_EQ(canonical.len, 0);
  }
}

UTEST(path_functions, path_extension) {
  sp_test_use_malloc();
  
  // Create a temporary test file for extension testing
  c8* test_filename = sp_test_generate_random_filename();
  sp_test_create_file(test_filename, "test");
  sp_str_t test_path = sp_str_cstr(test_filename);
  
  // Test normal file extension
  {
    sp_str_t ext = sp_os_path_extension(test_path);
    ASSERT_TRUE(sp_str_equal(ext, sp_str_lit(".tmp")));
  }
  
  sp_test_delete_file(test_filename);
  sp_free(test_filename);
  
  // Test path without extension (directory)
  {
    sp_str_t path = sp_str_lit(".");
    sp_str_t ext = sp_os_path_extension(path);
    ASSERT_EQ(ext.len, 0);
  }
  
  // Test non-existent file
  {
    sp_str_t path = sp_str_lit("nonexistent.txt");
    sp_str_t ext = sp_os_path_extension(path);
    ASSERT_EQ(ext.len, 0);
  }
}

UTEST(path_functions, extract_file_name) {
  sp_test_use_malloc();
  
  // Test normal path
  {
    sp_str_t path = sp_str_lit("C:/Users/Test/file.txt");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_TRUE(sp_str_equal(filename, sp_str_lit("file.txt")));
  }
  
  // Test path with trailing slash
  {
    sp_str_t path = sp_str_lit("C:/Users/Test/");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_EQ(filename.len, 0);
  }
  
  // Test path with backslashes
  {
    sp_str_t path = sp_str_lit("C:\\Users\\Test\\file.txt");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_TRUE(sp_str_equal(filename, sp_str_lit("file.txt")));
  }
  
  // Test filename only
  {
    sp_str_t path = sp_str_lit("file.txt");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_TRUE(sp_str_equal(filename, sp_str_lit("file.txt")));
  }
  
  // Test empty string
  {
    sp_str_t path = sp_str_lit("");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_EQ(filename.len, 0);
  }
  
  // Test Unix-style path
  {
    sp_str_t path = sp_str_lit("/home/user/document.pdf");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_TRUE(sp_str_equal(filename, sp_str_lit("document.pdf")));
  }
}

UTEST(path_functions, get_executable_path) {
  sp_test_use_malloc();
  
  sp_str_t exe_path = sp_os_get_executable_path();
  
  // Should not be empty
  ASSERT_GT(exe_path.len, 0);
  
  // Should be normalized (no backslashes)
  bool has_backslash = false;
  for (u32 i = 0; i < exe_path.len; i++) {
    if (exe_path.data[i] == '\\') {
      has_backslash = true;
      break;
    }
  }
  ASSERT_FALSE(has_backslash);
  
  // Should not end with slash
  ASSERT_NE(exe_path.data[exe_path.len - 1], '/');
  
  // Should end with an executable name
  sp_str_t filename = sp_os_extract_file_name(exe_path);
  ASSERT_GT(filename.len, 0);
}

UTEST(path_functions, integration_test) {
  sp_test_use_malloc();
  
  // Test the complete workflow used in main.cpp
  sp_str_t exe = sp_os_get_executable_path();
  sp_str_t parent1 = sp_os_parent_path(exe);
  sp_str_t parent2 = sp_os_parent_path(parent1);
  sp_str_t parent3 = sp_os_parent_path(parent2);
  sp_str_t install = sp_os_canonicalize_path(parent3);
  
  // Verify we got a valid install path
  ASSERT_GT(install.len, 0);
  ASSERT_NE(install.data[install.len - 1], '/');
  
  // Build a path similar to how it's done in main.cpp
  sp_str_builder_t builder = {0};
  sp_str_builder_append(&builder, install);
  sp_str_builder_append(&builder, sp_str_lit("/build/space-dll.bat"));
  sp_str_t dll_path = sp_str_builder_write(&builder);
  
  // Verify the path doesn't have double slashes
  bool has_double_slash = false;
  for (u32 i = 1; i < dll_path.len; i++) {
    if (dll_path.data[i-1] == '/' && dll_path.data[i] == '/') {
      has_double_slash = true;
      break;
    }
  }
  ASSERT_FALSE(has_double_slash);
}

#ifdef __cplusplus
UTEST(string_cpp, path_concatenation_operator) {
  sp_test_use_malloc();
  
  // Test basic concatenation
  sp_str_t path1 = sp_str_lit("home");
  sp_str_t path2 = sp_str_lit("user");
  sp_str_t result = path1 / path2;
  
  ASSERT_EQ(result.len, 9); // "home/user"
  ASSERT_TRUE(sp_str_equal(result, sp_str_lit("home/user")));
  
  // Test with backslashes (should be normalized)
  sp_str_t win_path1 = sp_str_lit("C:\\Windows");
  sp_str_t win_path2 = sp_str_lit("System32");
  sp_str_t win_result = win_path1 / win_path2;
  
  ASSERT_TRUE(sp_str_equal(win_result, sp_str_lit("C:/Windows/System32")));
  
  // Test empty paths
  sp_str_t empty = sp_str_lit("");
  sp_str_t filename = sp_str_lit("file.txt");
  sp_str_t empty_result = empty / filename;
  
  ASSERT_TRUE(sp_str_equal(empty_result, sp_str_lit("/file.txt")));
  
  // Test chaining
  sp_str_t base = sp_str_lit("root");
  sp_str_t dir = sp_str_lit("subdir");
  sp_str_t file = sp_str_lit("file.txt");
  sp_str_t chained = base / dir / file;
  
  ASSERT_TRUE(sp_str_equal(chained, sp_str_lit("root/subdir/file.txt")));
  
  // Test operator/ with C string literals
  sp_str_t path = sp_str_lit("home");
  sp_str_t result_cstr = path / "documents";
  
  ASSERT_EQ(result_cstr.len, 14); // "home/documents"
  ASSERT_TRUE(sp_str_equal(result_cstr, sp_str_lit("home/documents")));
  
  // Test chaining with C string literals
  sp_str_t chained_cstr = base / "data" / "files";
  ASSERT_TRUE(sp_str_equal(chained_cstr, sp_str_lit("root/data/files")));
}
#endif

UTEST_MAIN()
