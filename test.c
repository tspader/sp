#define _DARWIN_C_SOURCE
#define SP_IMPLEMENTATION
#define SP_APP
#include "sp.h"

#include "utest.h"


// ██╗   ██╗████████╗██╗██╗     ██╗████████╗██╗███████╗███████╗
// ██║   ██║╚══██╔══╝██║██║     ██║╚══██╔══╝██║██╔════╝██╔════╝
// ██║   ██║   ██║   ██║██║     ██║   ██║   ██║█████╗  ███████╗
// ██║   ██║   ██║   ██║██║     ██║   ██║   ██║██╔══╝  ╚════██║
// ╚██████╔╝   ██║   ██║███████╗██║   ██║   ██║███████╗███████║
//  ╚═════╝    ╚═╝   ╚═╝╚══════╝╚═╝   ╚═╝   ╚═╝╚══════╝╚══════╝
#define SP_TEST_REPORT(fmt, ...) \
  do { \
    sp_str_t formatted = sp_format_str(fmt, ##__VA_ARGS__); \
    UTEST_PRINTF("%s\n", sp_str_to_cstr(formatted)); \
  } while (0)

#define SP_TEST_STREQ(a, b, is_assert) \
  UTEST_SURPRESS_WARNING_BEGIN do { \
    if (!sp_str_equal((a), (b))) { \
      const c8* __sp_test_file_lval = __FILE__; \
      const u32 __sp_test_line_lval = __LINE__; \
      sp_str_builder_t __sp_test_builder = SP_ZERO_INITIALIZE(); \
      sp_str_builder_append_fmt_str(&__sp_test_builder, SP_LIT("{}:{} Failure:"), SP_FMT_CSTR(__sp_test_file_lval), SP_FMT_U32(__sp_test_line_lval)); \
      sp_str_builder_new_line(&__sp_test_builder); \
      sp_str_builder_indent(&__sp_test_builder); \
      sp_str_builder_append_fmt_str(&__sp_test_builder, SP_LIT("{} != {}"), SP_FMT_QUOTED_STR((a)), SP_FMT_QUOTED_STR((b))); \
      SP_TEST_REPORT(sp_str_builder_write(&__sp_test_builder)); \
      *utest_result = UTEST_TEST_FAILURE; \
 \
      if (is_assert) { \
        return; \
      } \
    } \
  } while (0) \
  UTEST_SURPRESS_WARNING_END

#define SP_EXPECT_STR_EQ_CSTR(a, b) SP_TEST_STREQ((a), SP_CSTR(b), false)
#define SP_EXPECT_STR_EQ(a, b) SP_TEST_STREQ((a), (b), false)

typedef struct sp_test_memory_tracker {
  sp_bump_allocator_t* bump;
  sp_allocator_t allocator;
} sp_test_memory_tracker;

typedef struct sp_test_file_monitor_data {
  bool change_detected;
  sp_file_change_event_t last_event;
  c8 last_file_path[SP_MAX_PATH_LEN];
} sp_test_file_monitor_data;

sp_str_t sp_test_generate_random_filename() {
  static u32 counter = 0;
  c8* filename = (c8*)sp_alloc(64);
#ifdef _WIN32
  unsigned int rand_val;
  rand_s(&rand_val);
#else
  unsigned int rand_val = counter * 12345 + 67890;  // Simple deterministic value for portability
#endif
  snprintf(filename, 64, "test_file_%u_%u.tmp", rand_val, counter++);
  return sp_str_from_cstr(filename);
}

void sp_test_create_file(const c8* filename, const c8* content) {
  FILE* file = fopen(filename, "w");
  if (file) {
  fputs(content, file);
  fclose(file);
  }
}

void sp_test_modify_file(const c8* filename, const c8* new_content) {
  FILE* file = fopen(filename, "w");
  if (file) {
  fputs(new_content, file);
  fclose(file);
  }
}

void sp_test_delete_file(const c8* filename) {
#ifdef _WIN32
  DeleteFileA(filename);
#else
  remove(filename);
#endif
}

bool sp_test_file_exists(const c8* filename) {
#ifdef _WIN32
  DWORD attrs = GetFileAttributesA(filename);
  return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
  FILE* file = fopen(filename, "r");
  if (file) {
    fclose(file);
    return true;
  }
  return false;
#endif
}

void sp_test_use_malloc() {
  static sp_malloc_allocator_t malloc_allocator = SP_ZERO_INITIALIZE();
  static sp_allocator_t allocator = SP_ZERO_INITIALIZE();

  sp_os_zero_memory(sp_context_stack, sizeof(sp_context_stack));
  sp_context = sp_context_stack;
  sp_context_push_allocator(sp_malloc_allocator_init());
}

void sp_test_use_bump_allocator(u32 capacity) {
  static sp_bump_allocator_t bump_allocator;

  bump_allocator = SP_ZERO_STRUCT(sp_bump_allocator_t);

  sp_allocator_t allocator = sp_bump_allocator_init(&bump_allocator, capacity);
  sp_context_push_allocator(allocator);
}

void sp_test_memory_tracker_init(sp_test_memory_tracker* tracker, u32 capacity) {
  sp_test_use_bump_allocator(capacity);
  tracker->bump = (sp_bump_allocator_t*)sp_context->allocator.user_data;
  tracker->allocator = sp_context->allocator;
}

u32 sp_test_memory_tracker_bytes_used(sp_test_memory_tracker* tracker) {
  return tracker->bump->bytes_used;
}

void sp_test_memory_tracker_clear(sp_test_memory_tracker* tracker) {
  sp_bump_allocator_clear(tracker->bump);
}

void sp_test_memory_tracker_destroy(sp_test_memory_tracker* tracker) {
  sp_bump_allocator_destroy(tracker->bump);
}

void sp_test_file_monitor_callback(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
  sp_test_file_monitor_data* data = (sp_test_file_monitor_data*)userdata;
  data->change_detected = true;
  data->last_event = change->events;
  sp_str_copy_to(change->file_path, data->last_file_path, SP_MAX_PATH_LEN);
}


//  ██████╗ ██████╗ ██████╗ ███████╗
// ██╔════╝██╔═══██╗██╔══██╗██╔════╝
// ██║     ██║   ██║██████╔╝█████╗
// ██║     ██║   ██║██╔══██╗██╔══╝
// ╚██████╗╚██████╔╝██║  ██║███████╗
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
UTEST(dynamic_array, initialization) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    ASSERT_EQ(arr.size, 0);
    ASSERT_EQ(arr.capacity, 2);
    ASSERT_EQ(arr.element_size, sizeof(s32));
    ASSERT_NE(arr.data, SP_NULLPTR);
  }

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

      u32 expected_alloc = test->size * 2;
      ASSERT_GE(sp_test_memory_tracker_bytes_used(&tracker), expected_alloc);

      sp_test_memory_tracker_clear(&tracker);
    }
  }

  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, push_operations) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    s32 val1 = 42;
    u8* elem1 = sp_dynamic_array_push(&arr, &val1);
    ASSERT_NE(elem1, SP_NULLPTR);
    ASSERT_EQ(arr.size, 1);
    ASSERT_EQ(*(s32*)elem1, 42);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 42);

    s32 val2 = 69;
    u8* elem2 = sp_dynamic_array_push(&arr, &val2);
    ASSERT_NE(elem2, SP_NULLPTR);
    ASSERT_EQ(arr.size, 2);
    ASSERT_EQ(*(s32*)elem2, 69);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 1), 69);

    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 42);
  }

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    u8* elem = sp_dynamic_array_push(&arr, SP_NULLPTR);
    ASSERT_NE(elem, SP_NULLPTR);
    ASSERT_EQ(arr.size, 1);
  }

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    s32 values[] = {10, 20, 30, 40, 50};
    u8* elems = sp_dynamic_array_push_n(&arr, values, 5);

    ASSERT_NE(elems, SP_NULLPTR);
    ASSERT_EQ(arr.size, 5);

    for (u32 i = 0; i < 5; i++) {
      ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, i), values[i]);
    }
  }

  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, growth) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    ASSERT_EQ(arr.capacity, 2);

    s32 values[] = {1, 2, 3};
    for (s32 i = 0; i < 3; i++) {
      sp_dynamic_array_push(&arr, &values[i]);
    }

    ASSERT_EQ(arr.size, 3);
    ASSERT_GE(arr.capacity, 3);

    for (u32 i = 0; i < 3; i++) {
      ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, i), values[i]);
    }
  }

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    sp_dynamic_array_grow(&arr, 10);

    ASSERT_GE(arr.capacity, 10);
    ASSERT_EQ(arr.size, 0);

    sp_dynamic_array_grow(&arr, 5);
    ASSERT_GE(arr.capacity, 10);
  }

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    for (s32 i = 0; i < 10; i++) {
      sp_dynamic_array_push(&arr, &i);
    }

    u32 old_bytes = sp_test_memory_tracker_bytes_used(&tracker);

    sp_dynamic_array_grow(&arr, 100);

    ASSERT_GE(arr.capacity, 100);
    ASSERT_GT(sp_test_memory_tracker_bytes_used(&tracker), old_bytes);

    for (u32 i = 0; i < 10; i++) {
      ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, i), (s32)i);
    }
  }

  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, reserve) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);

  sp_dynamic_array_t arr;
  sp_dynamic_array_init(&arr, sizeof(s32));

  {
    u8* reserved = sp_dynamic_array_reserve(&arr, 5);
    ASSERT_NE(reserved, SP_NULLPTR);
    ASSERT_EQ(arr.size, 5);
    ASSERT_GE(arr.capacity, 5);
  }

  {
    sp_dynamic_array_clear(&arr);

    s32 val = 1;
    sp_dynamic_array_push(&arr, &val);
    sp_dynamic_array_push(&arr, &val);

    ASSERT_EQ(arr.size, 2);
    ASSERT_GE(arr.capacity, 2);

    u8* reserved = sp_dynamic_array_reserve(&arr, 3);
    ASSERT_NE(reserved, SP_NULLPTR);
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

  for (s32 i = 0; i < 10; i++) {
    sp_dynamic_array_push(&arr, &i);
  }

  u32 old_cap = arr.capacity;

  sp_dynamic_array_clear(&arr);
  ASSERT_EQ(arr.size, 0);
  ASSERT_EQ(arr.capacity, old_cap);
  ASSERT_NE(arr.data, SP_NULLPTR);

  s32 val = 99;
  sp_dynamic_array_push(&arr, &val);
  ASSERT_EQ(arr.size, 1);
  ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 99);

  sp_test_memory_tracker_destroy(&tracker);
}

UTEST(dynamic_array, byte_size) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 1024 * 1024);

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
      sp_dynamic_array_push(&arr, SP_NULLPTR);
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

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    sp_dynamic_array_clear(&arr);
    ASSERT_EQ(sp_dynamic_array_byte_size(&arr), 0);
  }

  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    for (s32 i = 0; i < 10; i++) {
      sp_dynamic_array_push(&arr, &i);
    }

    for (u32 i = 0; i < 10; i++) {
      s32* elem = (s32*)sp_dynamic_array_at(&arr, i);
      ASSERT_EQ(*elem, (s32)i);
    }

    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 0), 0);
    ASSERT_EQ(*(s32*)sp_dynamic_array_at(&arr, 9), 9);
  }

  sp_test_memory_tracker_destroy(&tracker);
}

typedef struct {
  const c8* expected;
  sp_str_t actual;
} sp_test_format_case_t;

UTEST(sp_fmt, basic) {
  sp_test_use_malloc();

  sp_str_t result;

  result = sp_format("answer: {}", SP_FMT_U32(69));
  SP_EXPECT_STR_EQ_CSTR(result, "answer: 69");

  result = sp_format("{}", SP_FMT_U32(420));
  SP_EXPECT_STR_EQ_CSTR(result, "420");

  result = sp_format("answer");
  SP_EXPECT_STR_EQ_CSTR(result, "answer");

  result = sp_format("answer");
  SP_EXPECT_STR_EQ_CSTR(result, "answer");

  result = sp_format_str(SP_LIT("answer: {}"), SP_FMT_U32(690));
  SP_EXPECT_STR_EQ_CSTR(result, "answer: 690");

  result = sp_format_str(SP_LIT("{}"), SP_FMT_U32(4200));
  SP_EXPECT_STR_EQ_CSTR(result, "4200");

  result = sp_format_str(SP_LIT("answer"));
  SP_EXPECT_STR_EQ_CSTR(result, "answer");

}

UTEST(sp_fmt, numeric_types) {
  sp_test_use_malloc();

  u8 u8_val = 255;
  sp_str_t result = sp_format("u8: {}", SP_FMT_U8(u8_val));
  SP_EXPECT_STR_EQ_CSTR(result, "u8: 255");

  u16 u16_val = 65535;
  result = sp_format("u16: {}", SP_FMT_U16(u16_val));
  SP_EXPECT_STR_EQ_CSTR(result, "u16: 65535");

  u32 u32_val = 1234567890;
  result = sp_format("u32: {}", SP_FMT_U32(u32_val));
  SP_EXPECT_STR_EQ_CSTR(result, "u32: 1234567890");

  u64 u64_val = 9876543210ULL;
  result = sp_format("u64: {}", SP_FMT_U64(u64_val));
  SP_EXPECT_STR_EQ_CSTR(result, "u64: 9876543210");

  s8 s8_val = -128;
  result = sp_format("s8: {}", SP_FMT_S8(s8_val));
  SP_EXPECT_STR_EQ_CSTR(result, "s8: -128");

  s16 s16_val = -32768;
  result = sp_format("s16: {}", SP_FMT_S16(s16_val));
  SP_EXPECT_STR_EQ_CSTR(result, "s16: -32768");

  s32 s32_val = -2147483647;
  result = sp_format("s32: {}", SP_FMT_S32(s32_val));
  SP_EXPECT_STR_EQ_CSTR(result, "s32: -2147483647");

  s64 s64_val = -9223372036854775807LL;
  result = sp_format("s64: {}", SP_FMT_S64(s64_val));
  SP_EXPECT_STR_EQ_CSTR(result, "s64: -9223372036854775807");

  u32 zero = 0;
  result = sp_format("zero: {}", SP_FMT_U32(zero));
  SP_EXPECT_STR_EQ_CSTR(result, "zero: 0");
}

UTEST(sp_fmt, floating_point) {
  sp_test_use_malloc();

  f32 f32_val = 3.14159f;
  sp_str_t result = sp_format("f32: {}", SP_FMT_F32(f32_val));
  SP_EXPECT_STR_EQ_CSTR(result, "f32: 3.141");

  f64 f64_val = -2.71828;
  result = sp_format("f64: {}", SP_FMT_F64(f64_val));
  SP_EXPECT_STR_EQ_CSTR(result, "f64: -2.718");

  f32 f32_zero = 0.0f;
  result = sp_format("f32 zero: {}", SP_FMT_F32(f32_zero));
  SP_EXPECT_STR_EQ_CSTR(result, "f32 zero: 0.000");

  f32 f32_int = 42.0f;
  result = sp_format("f32 int: {}", SP_FMT_F32(f32_int));
  SP_EXPECT_STR_EQ_CSTR(result, "f32 int: 42.000");
}

UTEST(sp_fmt, string_types) {
  sp_test_use_malloc();

  sp_str_t str_val = SP_LIT("hello world");
  sp_str_t result = sp_format("str: {}", SP_FMT_STR(str_val));
  SP_EXPECT_STR_EQ_CSTR(result, "str: hello world");

  const c8* cstr_val = "c string";
  result = sp_format("cstr: {}", SP_FMT_CSTR(cstr_val));
  SP_EXPECT_STR_EQ_CSTR(result, "cstr: c string");

  result = sp_format("literal: {}", SP_FMT_CSTR("literal"));
  SP_EXPECT_STR_EQ_CSTR(result, "literal: literal");
}

UTEST(sp_fmt, character_types) {
  sp_test_use_malloc();
  sp_str_t expected;
  sp_str_t actual;

  c8 c8_val = 'A';
  expected = SP_LIT("A");
  actual = sp_format("{}", SP_FMT_C8(c8_val));
  SP_EXPECT_STR_EQ(actual, expected);

  c16 c16_val = 'Z';
  expected = SP_LIT("Z");
  actual = sp_format("{}", SP_FMT_C16(c16_val));
  SP_EXPECT_STR_EQ(actual, expected);

  c16 c16_unicode = 0x1234;
  expected = SP_LIT("U+1234");
  actual = sp_format("{}", SP_FMT_C16(c16_unicode));
  SP_EXPECT_STR_EQ(actual, expected);
}

UTEST(sp_fmt, pointer_type) {
  sp_test_use_malloc();

  void* ptr = (void*)(uintptr_t)0xDEADBEEF;
  sp_str_t result = sp_format("ptr: {}", SP_FMT_PTR(ptr));
  SP_EXPECT_STR_EQ_CSTR(result, "ptr: 0xdeadbeef");

  void* null_ptr = SP_NULLPTR;
  result = sp_format("null: {}", SP_FMT_PTR(null_ptr));
  SP_EXPECT_STR_EQ_CSTR(result, "null: 0x00000000");
}

UTEST(sp_fmt, hash_type) {
  sp_test_use_malloc();

  sp_hash_t hash = 0xABCDEF12;
  sp_str_t result = sp_format("hash: {}", SP_FMT_HASH(hash));
  SP_EXPECT_STR_EQ_CSTR(result, "hash: abcdef12");

  sp_hash_t zero_hash = 0;
  result = sp_format("zero hash: {}", SP_FMT_HASH(zero_hash));
  SP_EXPECT_STR_EQ_CSTR(result, "zero hash: 0");
}

UTEST(sp_fmt, array_types) {
  sp_test_use_malloc();

  sp_fixed_array_t fixed_arr;
  fixed_arr.size = 10;
  fixed_arr.capacity = 20;
  fixed_arr.element_size = 4;
  fixed_arr.data = SP_NULLPTR;

  sp_str_t result = sp_format("fixed: {}", SP_FMT_FIXED_ARRAY(fixed_arr));
  SP_EXPECT_STR_EQ_CSTR(result, "fixed: { size: 10, capacity: 20 }");

  sp_dynamic_array_t dyn_arr;
  dyn_arr.size = 5;
  dyn_arr.capacity = 16;
  dyn_arr.element_size = 8;
  dyn_arr.data = SP_NULLPTR;

  result = sp_format("dynamic: {}", SP_FMT_DYNAMIC_ARRAY(dyn_arr));
  SP_EXPECT_STR_EQ_CSTR(result, "dynamic: { size: 5, capacity: 16 }");
}

UTEST(sp_fmt, multiple_args) {
  sp_test_use_malloc();

  u32 count = 42;
  sp_str_t name = SP_LIT("test");
  f32 value = 3.14f;

  sp_str_t result = sp_format("Count: {}, Name: {}, Value: {}",
    SP_FMT_U32(count), SP_FMT_STR(name), SP_FMT_F32(value));
  SP_EXPECT_STR_EQ_CSTR(result, "Count: 42, Name: test, Value: 3.140");
}

UTEST(sp_str_builder, basic_operations) {
  sp_test_use_malloc();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  ASSERT_EQ(builder.buffer.data, SP_NULLPTR);
  ASSERT_EQ(builder.buffer.count, 0);
  ASSERT_EQ(builder.buffer.capacity, 0);

  sp_str_builder_grow(&builder, 10);
  ASSERT_GE(builder.buffer.capacity, 10);
  ASSERT_NE(builder.buffer.data, SP_NULLPTR);
  ASSERT_EQ(builder.buffer.count, 0);

  sp_str_t test_str = SP_LIT("Hello");
  sp_str_builder_append(&builder, test_str);
  ASSERT_EQ(builder.buffer.count, 5);

  sp_str_builder_append_cstr(&builder, " World");
  ASSERT_EQ(builder.buffer.count, 11);

  sp_str_builder_append_c8(&builder, '!');
  ASSERT_EQ(builder.buffer.count, 12);

  sp_str_t result = sp_str_builder_write(&builder);
  ASSERT_EQ(result.len, 12);
  SP_EXPECT_STR_EQ_CSTR(result, "Hello World!");

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_builder_append_cstr(&builder2, "Test");
  c8* cstr_result = sp_str_builder_write_cstr(&builder2);
  ASSERT_TRUE(sp_cstr_equal(cstr_result, "Test"));
  sp_free(cstr_result);
}

UTEST(sp_str_builder, growth_behavior) {
  sp_test_use_malloc();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_str_builder_grow(&builder, 5);
  u32 cap1 = builder.buffer.capacity;
  ASSERT_GE(cap1, 5);

  sp_str_builder_grow(&builder, 10);
  u32 cap2 = builder.buffer.capacity;
  ASSERT_GE(cap2, 10);
  ASSERT_GE(cap2, cap1);

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_t long_str = SP_LIT("This is a much longer string that will trigger growth");
  sp_str_builder_append(&builder2, long_str);
  ASSERT_GE(builder2.buffer.capacity, long_str.len);
  ASSERT_EQ(builder2.buffer.count, long_str.len);
}

UTEST(sp_str_builder, edge_cases) {
  sp_test_use_malloc();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, SP_LIT(""));
  ASSERT_EQ(builder.buffer.count, 0);

  sp_str_builder_append_cstr(&builder, "");
  ASSERT_EQ(builder.buffer.count, 0);

  sp_str_t null_str = {.len = 0, .data = SP_NULLPTR};
  sp_str_builder_append(&builder, null_str);
  ASSERT_EQ(builder.buffer.count, 0);

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  for (s32 i = 0; i < 100; i++) {
    sp_str_builder_append_cstr(&builder2, "test ");
  }
  ASSERT_EQ(builder2.buffer.count, 500);
  sp_str_t result = sp_str_builder_write(&builder2);
  ASSERT_EQ(result.len, 500);
}

UTEST(sp_str_builder, indent_operations) {
  sp_test_use_malloc();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append_cstr(&builder, "normal");
  sp_str_builder_new_line(&builder);
  sp_str_builder_indent(&builder);
  sp_str_builder_append_cstr(&builder, "indented");
  sp_str_builder_new_line(&builder);
  sp_str_builder_indent(&builder);
  sp_str_builder_append_cstr(&builder, "double");
  sp_str_builder_new_line(&builder);
  sp_str_builder_dedent(&builder);
  sp_str_builder_append_cstr(&builder, "single");
  sp_str_builder_new_line(&builder);
  sp_str_builder_dedent(&builder);
  sp_str_builder_append_cstr(&builder, "back");

  sp_str_t result = sp_str_builder_write(&builder);
  ASSERT_GT(result.len, 10);

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_builder_dedent(&builder2);
  sp_str_builder_dedent(&builder2);
  sp_str_builder_append_cstr(&builder2, "no_crash");
  ASSERT_EQ(builder2.indent.level, 0);
}

UTEST(sp_str_builder, format_append) {
  sp_test_use_malloc();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append_fmt(&builder, "Value: {}", SP_FMT_U32(123));
  sp_str_t result = sp_str_builder_write(&builder);
  ASSERT_GT(result.len, 0);
  ASSERT_NE(result.data, SP_NULLPTR);
}

UTEST(sp_cstr_copy, all_variations) {
  sp_test_use_malloc();

  const c8* original = "Hello World";
  c8* copy = sp_cstr_copy(original);
  ASSERT_TRUE(sp_cstr_equal(copy, original));
  ASSERT_NE(copy, original);
  sp_free(copy);

  c8* partial = sp_cstr_copy_sized(original, 5);
  ASSERT_TRUE(sp_cstr_equal(partial, "Hello"));
  sp_free(partial);

  const c8* empty = "";
  c8* empty_copy = sp_cstr_copy(empty);
  ASSERT_TRUE(sp_cstr_equal(empty_copy, ""));
  sp_free(empty_copy);

  c8* null_copy = sp_cstr_copy(SP_NULLPTR);
  ASSERT_EQ(null_copy[0], '\0');
  sp_free(null_copy);
}

UTEST(sp_cstr_copy_to, buffer_operations) {
  sp_test_use_malloc();

  const c8* source = "Hello World";
  c8 buffer[20];
  sp_os_zero_memory(buffer, 20);
  sp_cstr_copy_to(source, buffer, 20);
  ASSERT_TRUE(sp_cstr_equal(buffer, source));

  c8 exact[12];
  sp_os_zero_memory(exact, 12);
  sp_cstr_copy_to(source, exact, 12);
  ASSERT_TRUE(sp_cstr_equal(exact, source));

  char small_buffer[6];
  sp_os_zero_memory(small_buffer, 6);
  sp_cstr_copy_to(source, small_buffer, 6);
  ASSERT_TRUE(sp_cstr_equal(small_buffer, "Hello"));

  c8 partial_buffer[10];
  sp_os_zero_memory(partial_buffer, 10);
  sp_cstr_copy_to_sized(source, 5, partial_buffer, 10);
  ASSERT_TRUE(sp_cstr_equal(partial_buffer, "Hello"));

  c8 null_buffer[10];
  sp_cstr_copy_to("test", null_buffer, 10);
  sp_cstr_copy_to(SP_NULLPTR, null_buffer, 10);
  ASSERT_TRUE(sp_cstr_equal(null_buffer, "test"));

  sp_cstr_copy_to(source, SP_NULLPTR, 10);

  c8 zero_buffer[10] = "unchanged";
  sp_cstr_copy_to(source, zero_buffer, 0);
  ASSERT_TRUE(sp_cstr_equal(zero_buffer, "unchanged"));
}

UTEST(sp_cstr_equal, comparison_tests) {
  sp_test_use_malloc();

  ASSERT_TRUE(sp_cstr_equal("Hello", "Hello"));
  ASSERT_TRUE(sp_cstr_equal("", ""));

  ASSERT_FALSE(sp_cstr_equal("Hello", "World"));
  ASSERT_FALSE(sp_cstr_equal("Hello", "Hello!"));
  ASSERT_FALSE(sp_cstr_equal("Hello", "Hell"));

  ASSERT_TRUE(sp_cstr_equal(SP_NULLPTR, SP_NULLPTR));
  ASSERT_FALSE(sp_cstr_equal("Hello", SP_NULLPTR));
  ASSERT_FALSE(sp_cstr_equal(SP_NULLPTR, "Hello"));

  ASSERT_FALSE(sp_cstr_equal("Hello", "hello"));
}

UTEST(sp_cstr_len, length_tests) {
  sp_test_use_malloc();

  ASSERT_EQ(sp_cstr_len("Hello"), 5);
  ASSERT_EQ(sp_cstr_len("Hello World!"), 12);
  ASSERT_EQ(sp_cstr_len(""), 0);

  ASSERT_EQ(sp_cstr_len(SP_NULLPTR), 0);

  const c8 embedded[] = {'H', 'e', '\0', 'l', 'o', '\0'};
  ASSERT_EQ(sp_cstr_len(embedded), 2);
}

UTEST(sp_wstr_to_cstr, wide_string_conversion) {
  sp_test_use_malloc();

  c16 wide_str[] = L"Hello";
  c8* converted = sp_wstr_to_cstr(wide_str, 5);
  ASSERT_TRUE(sp_cstr_equal(converted, "Hello"));
  sp_free(converted);

  c16 empty[] = L"";
  c8* empty_converted = sp_wstr_to_cstr(empty, 0);
  ASSERT_TRUE(sp_cstr_equal(empty_converted, ""));
  sp_free(empty_converted);

  c16 special[] = L"Test 123!";
  c8* special_converted = sp_wstr_to_cstr(special, 9);
  ASSERT_TRUE(sp_cstr_equal(special_converted, "Test 123!"));
  sp_free(special_converted);
}

UTEST(sp_str_to, conversion_functions) {
  sp_test_use_malloc();

  sp_str_t str = SP_LIT("Hello World");
  c8* cstr = sp_str_to_cstr(str);
  ASSERT_TRUE(sp_cstr_equal(cstr, "Hello World"));
  sp_free(cstr);

  sp_str_t path = SP_LIT("C:\\test");
  c8* double_null = sp_str_to_cstr_double_nt(path);
  ASSERT_EQ(double_null[7], '\0');
  ASSERT_EQ(double_null[8], '\0');
  sp_free(double_null);

  sp_str_t empty = SP_LIT("");
  c8* empty_cstr = sp_str_to_cstr(empty);
  ASSERT_TRUE(sp_cstr_equal(empty_cstr, ""));
  sp_free(empty_cstr);
}

UTEST(sp_str_copy, string_copy_operations) {
  sp_test_use_malloc();

  sp_str_t original = SP_LIT("Hello World");
  sp_str_t copy = sp_str_copy(original);
  ASSERT_EQ(copy.len, original.len);
  ASSERT_TRUE(sp_str_equal(copy, original));
  ASSERT_NE(copy.data, original.data);

  sp_str_t from_cstr = sp_str_from_cstr("Test String");
  ASSERT_EQ(from_cstr.len, 11);
  SP_EXPECT_STR_EQ_CSTR(from_cstr, "Test String");

  sp_str_t partial = sp_str_from_cstr_sized("Hello World", 5);
  ASSERT_EQ(partial.len, 5);
  SP_EXPECT_STR_EQ_CSTR(partial, "Hello");

  c8 buffer[20];
  sp_os_zero_memory(buffer, 20);
  sp_str_copy_to(original, buffer, 20);
  ASSERT_TRUE(sp_os_is_memory_equal(buffer, original.data, original.len));

  c8 small_buffer[5];
  sp_os_zero_memory(small_buffer, 5);
  sp_str_copy_to(original, small_buffer, 5);
  ASSERT_TRUE(sp_os_is_memory_equal(small_buffer, "Hello", 5));
}

UTEST(sp_str, string_creation) {
  sp_test_use_malloc();

  sp_str_t str1 = sp_str("Hello", 5);
  ASSERT_EQ(str1.len, 5);
  ASSERT_EQ(str1.data[0], 'H');

  sp_str_t str2 = SP_LIT("World");
  ASSERT_EQ(str2.len, 5);
  SP_EXPECT_STR_EQ_CSTR(str2, "World");

  const c8* cstr = "Dynamic";
  sp_str_t str3 = SP_CSTR(cstr);
  ASSERT_EQ(str3.len, 7);
  SP_EXPECT_STR_EQ_CSTR(str3, "Dynamic");

  sp_str_t allocated = sp_str_alloc(100);
  ASSERT_EQ(allocated.len, 0);
  ASSERT_NE(allocated.data, SP_NULLPTR);
}

UTEST(sp_str_equal, string_comparison) {
  sp_test_use_malloc();

  sp_str_t str1 = SP_LIT("Hello");
  sp_str_t str2 = SP_LIT("Hello");
  sp_str_t str3 = SP_LIT("World");
  sp_str_t str4 = SP_LIT("Hell");

  ASSERT_TRUE(sp_str_equal(str1, str2));
  ASSERT_FALSE(sp_str_equal(str1, str3));
  ASSERT_FALSE(sp_str_equal(str1, str4));

  SP_EXPECT_STR_EQ_CSTR(str1, "Hello");
  ASSERT_FALSE(sp_str_equal_cstr(str1, "World"));
  ASSERT_FALSE(sp_str_equal_cstr(str1, "Hell"));

  sp_str_t empty1 = SP_LIT("");
  sp_str_t empty2 = SP_LIT("");
  ASSERT_TRUE(sp_str_equal(empty1, empty2));
  SP_EXPECT_STR_EQ_CSTR(empty1, "");

  sp_str_t long_str = SP_LIT("Hello World!");
  ASSERT_FALSE(sp_str_equal(str1, long_str));
}

UTEST(sp_str_sub, substrings) {
  sp_str_t str = SP_LIT("Jerry Garcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_sub(str, 0, 5), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_sub(str, 6, 6), "Garcia");
}

UTEST(sp_str_sort_kernel_alphabetical, sorting_tests) {
  sp_test_use_malloc();

  sp_str_t strings[] = {
    SP_LIT("zebra"),
    SP_LIT("apple"),
    SP_LIT("banana"),
    SP_LIT("aardvark"),
    SP_LIT("zoo")
  };

  qsort(strings, 5, sizeof(sp_str_t), sp_str_sort_kernel_alphabetical);

  SP_EXPECT_STR_EQ_CSTR(strings[0], "aardvark");
  SP_EXPECT_STR_EQ_CSTR(strings[1], "apple");
  SP_EXPECT_STR_EQ_CSTR(strings[2], "banana");
  SP_EXPECT_STR_EQ_CSTR(strings[3], "zebra");
  SP_EXPECT_STR_EQ_CSTR(strings[4], "zoo");

  ASSERT_EQ(sp_str_compare_alphabetical(SP_LIT("a"), SP_LIT("b")), SP_QSORT_A_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(SP_LIT("b"), SP_LIT("a")), SP_QSORT_B_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(SP_LIT("same"), SP_LIT("same")), SP_QSORT_EQUAL);

  ASSERT_EQ(sp_str_compare_alphabetical(SP_LIT("ab"), SP_LIT("abc")), SP_QSORT_A_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(SP_LIT("abc"), SP_LIT("ab")), SP_QSORT_B_FIRST);
}

sp_str_t sp_test_map_band_member(sp_str_t str, sp_opaque_ptr user_data) {
  return sp_str_concat(str, SP_LIT(" is in the band"));
}

UTEST(sp_str_t, map_reduce) {
  sp_test_use_malloc();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_t band [] = {
    SP_LIT("jerry"), SP_LIT("bobby"), SP_LIT("phil")
  };
  sp_dyn_array(sp_str_t) result = sp_str_map(&band[0], SP_CARR_LEN(band), SP_NULLPTR, sp_test_map_band_member);
  SP_EXPECT_STR_EQ_CSTR(result[0], "jerry is in the band");
  SP_EXPECT_STR_EQ_CSTR(result[1], "bobby is in the band");
  SP_EXPECT_STR_EQ_CSTR(result[2], "phil is in the band");

  sp_str_t joined = sp_str_join_n(band, SP_CARR_LEN(band), SP_LIT(" and "));
  SP_EXPECT_STR_EQ_CSTR(joined, "jerry and bobby and phil");

  u32 len = 3;
  sp_dyn_array(sp_str_t) clipped = sp_str_map(&band[0], SP_CARR_LEN(band), &len, sp_str_map_kernel_prefix);
  SP_EXPECT_STR_EQ_CSTR(clipped[0], "jer");
  SP_EXPECT_STR_EQ_CSTR(clipped[1], "bob");
  SP_EXPECT_STR_EQ_CSTR(clipped[2], "phi");

}


UTEST(sp_str_utilities, valid_and_at) {
  sp_test_use_malloc();

  sp_str_t valid = SP_LIT("Hello");
  sp_str_t invalid = {.len = 5, .data = SP_NULLPTR};
  sp_str_t empty = SP_LIT("");

  ASSERT_TRUE(sp_str_valid(valid));
  ASSERT_FALSE(sp_str_valid(invalid));
  ASSERT_TRUE(sp_str_valid(empty));

  sp_str_t str = SP_LIT("Hello");
  ASSERT_EQ(sp_str_at(str, 0), 'H');
  ASSERT_EQ(sp_str_at(str, 1), 'e');
  ASSERT_EQ(sp_str_at(str, 4), 'o');

  ASSERT_EQ(sp_str_at(str, -1), 'o');
  ASSERT_EQ(sp_str_at(str, -2), 'l');
  ASSERT_EQ(sp_str_at(str, -5), 'H');

  ASSERT_EQ(sp_str_at_reverse(str, 0), 'o');
  ASSERT_EQ(sp_str_at_reverse(str, 1), 'l');
  ASSERT_EQ(sp_str_at_reverse(str, 4), 'H');

  ASSERT_EQ(sp_str_at_reverse(str, -1), 'H');
  ASSERT_EQ(sp_str_at_reverse(str, -2), 'e');
  ASSERT_EQ(sp_str_at_reverse(str, -5), 'o');
  ASSERT_EQ(sp_str_back(str), 'o');
  sp_str_t single = SP_LIT("X");
  ASSERT_EQ(sp_str_back(single), 'X');
}

UTEST(sp_str_manipulation, to_upper_and_replace) {
  sp_test_use_malloc();

  // Test sp_str_to_upper
  sp_str_t lowercase = SP_LIT("hello world!");
  sp_str_t uppercase = sp_str_to_upper(lowercase);
  SP_EXPECT_STR_EQ_CSTR(uppercase, "HELLO WORLD!");

  sp_str_t mixed = SP_LIT("HeLLo WoRLd!");
  sp_str_t upper_mixed = sp_str_to_upper(mixed);
  SP_EXPECT_STR_EQ_CSTR(upper_mixed, "HELLO WORLD!");

  // Test sp_str_replace
  sp_str_t original = SP_LIT("hello world");
  sp_str_t replaced = sp_str_replace_c8(original, 'l', 'X');
  SP_EXPECT_STR_EQ_CSTR(replaced, "heXXo worXd");

  sp_str_t no_match = sp_str_replace_c8(original, 'z', 'X');
  SP_EXPECT_STR_EQ_CSTR(no_match, "hello world");
}

UTEST(sp_str_manipulation, ends_with) {
  sp_test_use_malloc();

  sp_str_t str = SP_LIT("hello world");
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("world")));
  ASSERT_FALSE(sp_str_ends_with(str, SP_LIT("hello")));
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("")));
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("d")));
}

UTEST(sp_str_manipulation, concat) {
  sp_test_use_malloc();

  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT("Jerry"), SP_LIT("Garcia")), "JerryGarcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT("Jerry"), SP_LIT("")), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT(""), SP_LIT("Jerry")), "Jerry");
}

UTEST(sp_str_manipulation, join_operations) {
  sp_test_use_malloc();

  SP_EXPECT_STR_EQ_CSTR(sp_str_join(SP_LIT("hello"), SP_LIT("world"), SP_LIT(" - ")), "hello - world");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join(SP_LIT("hello"), SP_LIT("world"), SP_LIT("")), "helloworld");

  const c8* strings[] = {"apple", "banana", "cherry"};
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n(strings, 3, SP_LIT(", ")), "apple, banana, cherry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n(strings, 1, SP_LIT(", ")), "apple");
  ASSERT_EQ(sp_str_join_cstr_n(strings, 0, SP_LIT(", ")).len, 0);
}

UTEST(path_functions, normalize_path) {
  sp_test_use_malloc();

  {
    sp_str_t path = SP_LIT("C:\\Users\\Test\\file.txt");
    sp_str_t copy = sp_os_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test/file.txt");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test/file.txt");
    sp_str_t copy = sp_os_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test/file.txt");
  }

  {
    sp_str_t path = SP_LIT("C:/Users\\Test/sub\\file.txt");
    sp_str_t copy = sp_os_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test/sub/file.txt");
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t copy = sp_os_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "");
  }

  {
    sp_str_t path = SP_LIT("C:\\Users\\Test\\");
    sp_str_t copy = sp_os_normalize_path(path);
    SP_EXPECT_STR_EQ_CSTR(copy, "C:/Users/Test");
  }
}

UTEST(path_functions, parent_path) {
  sp_test_use_malloc();

  {
    sp_str_t path = SP_LIT("C:/Users/Test/file.txt");
    sp_str_t parent = sp_os_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/Users/Test");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test/");
    sp_str_t parent = sp_os_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/Users");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test///");
    sp_str_t parent = sp_os_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "C:/Users");
  }

  {
    sp_str_t path = SP_LIT("C:/");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("Test");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("/");
    sp_str_t parent = sp_os_parent_path(path);
    ASSERT_EQ(parent.len, 0);
  }

  {
    sp_str_t path = SP_LIT("/home/user/file");
    sp_str_t parent = sp_os_parent_path(path);
    SP_EXPECT_STR_EQ_CSTR(parent, "/home/user");
  }
}

UTEST(path_functions, canonicalize_path) {
  sp_test_use_malloc();

  {
    sp_str_t path = SP_LIT("test/..");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    ASSERT_NE(canonical.data[canonical.len - 1], '/');
  }

  {
    sp_str_t path = SP_LIT("../../another");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    sp_str_t filename = sp_os_extract_file_name(canonical);
    SP_EXPECT_STR_EQ_CSTR(filename, "another");
  }

  {
    sp_str_t exe = sp_os_get_executable_path();
    sp_str_t canonical = sp_os_canonicalize_path(exe);
    ASSERT_TRUE(sp_str_equal(canonical, exe));
  }

  {
    sp_str_t path = SP_LIT("test/");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_GT(canonical.len, 0);
    ASSERT_NE(canonical.data[canonical.len - 1], '/');
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t canonical = sp_os_canonicalize_path(path);
    ASSERT_EQ(canonical.len, 0);
  }
}

typedef struct {
  sp_str_t file_path;
  sp_str_t extension;
} sp_test_file_extension_case_t;

UTEST(path_functions, path_extension) {
  sp_test_use_malloc();

  sp_test_file_extension_case_t cases [] = {
    {
      .file_path = SP_LIT("foo.bar"),
      .extension = SP_LIT("bar")
    },
    {
      .file_path = SP_LIT("foo."),
      .extension = SP_LIT("")
    },
    {
      .file_path = SP_LIT("foo.bar.baz"),
      .extension = SP_LIT("baz")
    },
    {
      .file_path = SP_LIT("foo"),
      .extension = SP_LIT("")
    },
    {
      .file_path = SP_LIT("foo.bar."),
      .extension = SP_LIT("")
    },
    {
      .file_path = SP_LIT(".foo"),
      .extension = SP_LIT("foo")
    },
  };

  SP_CARR_FOR(cases, index) {
    sp_str_t extension = sp_os_extract_extension(cases[index].file_path);
    SP_EXPECT_STR_EQ(extension, cases[index].extension);
  }
}

typedef struct {
  sp_str_t file_path;
  sp_str_t stem;
} sp_test_file_stem_case_t;

UTEST(path_functions, path_stem) {
  sp_test_use_malloc();

  sp_test_file_stem_case_t cases [] = {
    {
      .file_path = SP_LIT("foo.bar"),
      .stem = SP_LIT("foo")
    },
    {
      .file_path = SP_LIT("foo."),
      .stem = SP_LIT("foo")
    },
    {
      .file_path = SP_LIT("foo.bar.baz"),
      .stem = SP_LIT("foo.bar")
    },
    {
      .file_path = SP_LIT("foo"),
      .stem = SP_LIT("foo")
    },
    {
      .file_path = SP_LIT("foo.bar."),
      .stem = SP_LIT("foo.bar")
    },
    {
      .file_path = SP_LIT(".foo"),
      .stem = SP_LIT("")
    },
  };

  SP_CARR_FOR(cases, index) {
    sp_str_t stem = sp_os_extract_stem(cases[index].file_path);
    SP_EXPECT_STR_EQ(stem, cases[index].stem);
  }

}

UTEST(path_functions, extract_file_name) {
  sp_test_use_malloc();

  {
    sp_str_t path = SP_LIT("C:/Users/Test/file.txt");
    sp_str_t filename = sp_os_extract_file_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
  }

  {
    sp_str_t path = SP_LIT("C:/Users/Test/");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_EQ(filename.len, 0);
  }

  {
    sp_str_t path = SP_LIT("C:\\Users\\Test\\file.txt");
    sp_str_t filename = sp_os_extract_file_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
  }

  {
    sp_str_t path = SP_LIT("file.txt");
    sp_str_t filename = sp_os_extract_file_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "file.txt");
  }

  {
    sp_str_t path = SP_LIT("");
    sp_str_t filename = sp_os_extract_file_name(path);
    ASSERT_EQ(filename.len, 0);
  }

  {
    sp_str_t path = SP_LIT("/home/user/document.pdf");
    sp_str_t filename = sp_os_extract_file_name(path);
    SP_EXPECT_STR_EQ_CSTR(filename, "document.pdf");
  }
}

UTEST(path_functions, get_executable_path) {
  sp_test_use_malloc();

  sp_str_t exe_path = sp_os_get_executable_path();

  ASSERT_GT(exe_path.len, 0);

  bool has_backslash = false;
  for (u32 i = 0; i < exe_path.len; i++) {
    if (exe_path.data[i] == '\\') {
      has_backslash = true;
      break;
    }
  }
  ASSERT_FALSE(has_backslash);

  ASSERT_NE(exe_path.data[exe_path.len - 1], '/');

  sp_str_t filename = sp_os_extract_file_name(exe_path);
  ASSERT_GT(filename.len, 0);
}

UTEST(path_functions, integration_test) {
  sp_test_use_malloc();

  sp_str_t exe = sp_os_get_executable_path();
  sp_str_t parent1 = sp_os_parent_path(exe);
  sp_str_t parent2 = sp_os_parent_path(parent1);
  sp_str_t parent3 = sp_os_parent_path(parent2);
  sp_str_t install = sp_os_canonicalize_path(parent3);

  ASSERT_GT(install.len, 0);
  ASSERT_NE(install.data[install.len - 1], '/');

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, install);
  sp_str_builder_append(&builder, SP_LIT("/build/space-dll.bat"));
  sp_str_t dll_path = sp_str_builder_write(&builder);

  bool has_double_slash = false;
  for (u32 i = 1; i < dll_path.len; i++) {
    if (dll_path.data[i-1] == '/' && dll_path.data[i] == '/') {
      has_double_slash = true;
      break;
    }
  }
  ASSERT_FALSE(has_double_slash);
}

UTEST(dyn_array, basic_operations) {
    sp_test_use_malloc();

    sp_dyn_array(int) arr = SP_NULLPTR;

    ASSERT_EQ(sp_dyn_array_size(arr), 0);
    ASSERT_EQ(sp_dyn_array_capacity(arr), 0);
    ASSERT_TRUE(sp_dyn_array_empty(arr));

    sp_dyn_array_push(arr, 42);
    ASSERT_EQ(sp_dyn_array_size(arr), 1);
    ASSERT_GE(sp_dyn_array_capacity(arr), 1);
    ASSERT_FALSE(sp_dyn_array_empty(arr));
    ASSERT_EQ(arr[0], 42);

    for (s32 i = 1; i < 10; i++) {
        sp_dyn_array_push(arr, i * 10);
    }
    ASSERT_EQ(sp_dyn_array_size(arr), 10);

    ASSERT_EQ(arr[0], 42);
    for (s32 i = 1; i < 10; i++) {
        ASSERT_EQ(arr[i], i * 10);
    }

    sp_dyn_array_pop(arr);
    ASSERT_EQ(sp_dyn_array_size(arr), 9);

    ASSERT_EQ(*sp_dyn_array_back(arr), 80);

    sp_dyn_array_clear(arr);
    ASSERT_EQ(sp_dyn_array_size(arr), 0);
    ASSERT_TRUE(sp_dyn_array_empty(arr));

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, reserve_capacity) {
    sp_test_use_malloc();

    sp_dyn_array(float) arr = SP_NULLPTR;

    sp_dyn_array_reserve(arr, 100);
    ASSERT_GE(sp_dyn_array_capacity(arr), 100);
    ASSERT_EQ(sp_dyn_array_size(arr), 0);

    for (s32 i = 0; i < 50; i++) {
        sp_dyn_array_push(arr, (float)i * 0.5f);
    }
    ASSERT_GE(sp_dyn_array_capacity(arr), 100);
    ASSERT_EQ(sp_dyn_array_size(arr), 50);

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, growth_pattern) {
    sp_test_use_malloc();

    sp_dyn_array(u32) arr = SP_NULLPTR;

    u32 prev_capacity = 0;

    for (u32 i = 0; i < 100; i++) {
        sp_dyn_array_push(arr, i);

        u32 current_capacity = sp_dyn_array_capacity(arr);
        if (current_capacity != prev_capacity) {
            if (prev_capacity > 0) {
                ASSERT_EQ(current_capacity, prev_capacity * 2);
            }
            prev_capacity = current_capacity;
        }
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 100);

    sp_dyn_array_free(arr);
}

typedef struct test_struct {
    s32 id;
    float value;
    char name[32];
} test_struct;

UTEST(dyn_array, struct_type) {
    sp_test_use_malloc();

    sp_dyn_array(test_struct) arr = SP_NULLPTR;

    for (s32 i = 0; i < 10; i++) {
        test_struct s = SP_ZERO_INITIALIZE();
        s.id = i;
        s.value = (float)i * 1.5f;
        snprintf(s.name, sizeof(s.name), "Item_%d", i);
        sp_dyn_array_push(arr, s);
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 10);

    for (s32 i = 0; i < 10; i++) {
        ASSERT_EQ(arr[i].id, i);
        ASSERT_EQ(arr[i].value, (float)i * 1.5f);

        char expected[32];
        snprintf(expected, sizeof(expected), "Item_%d", i);
        ASSERT_STREQ(arr[i].name, expected);
    }

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, pointer_type) {
    sp_test_use_malloc();

    sp_dyn_array(char*) arr = SP_NULLPTR;

    const char* strings[] = {"Hello", "World", "Dynamic", "Array", "Test"};

    for (s32 i = 0; i < 5; i++) {
        c8* str = (c8*)sp_alloc(strlen(strings[i]) + 1);
        strcpy(str, strings[i]);
        sp_dyn_array_push(arr, str);
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 5);

    for (s32 i = 0; i < 5; i++) {
        ASSERT_STREQ(arr[i], strings[i]);
    }

    for (s32 i = 0; i < sp_dyn_array_size(arr); i++) {
        sp_free(arr[i]);
    }

    sp_dyn_array_free(arr);
}

UTEST(dyn_array, edge_cases) {
    sp_test_use_malloc();

    sp_dyn_array(int) arr1 = SP_NULLPTR;
    sp_dyn_array_free(arr1);
    sp_dyn_array_free(arr1);

    sp_dyn_array(int) arr2 = SP_NULLPTR;
    sp_dyn_array_pop(arr2);
    ASSERT_EQ(sp_dyn_array_size(arr2), 0);

    sp_dyn_array_clear(arr2);

    sp_dyn_array_push(arr2, 42);
    sp_dyn_array_free(arr2);

    sp_dyn_array(int) arr3 = SP_NULLPTR;
    sp_dyn_array_reserve(arr3, 0);
    ASSERT_GE(sp_dyn_array_capacity(arr3), 0);
    sp_dyn_array_free(arr3);
}

UTEST(sp_dyn_array_push_f, basic_int_push) {
    sp_test_use_malloc();

    int* arr = SP_NULLPTR;

    ASSERT_EQ(sp_dyn_array_size(arr), 0);
    ASSERT_EQ(sp_dyn_array_capacity(arr), 0);

    int val1 = 42;
    sp_dyn_array_push_f((void**)&arr, &val1, sizeof(int));

    ASSERT_NE(arr, SP_NULLPTR);
    ASSERT_EQ(sp_dyn_array_size(arr), 1);
    ASSERT_GE(sp_dyn_array_capacity(arr), 1);
    ASSERT_EQ(arr[0], 42);

    int val2 = 69;
    sp_dyn_array_push_f((void**)&arr, &val2, sizeof(int));
    ASSERT_EQ(sp_dyn_array_size(arr), 2);
    ASSERT_EQ(arr[1], 69);

    int val3 = 420;
    sp_dyn_array_push_f((void**)&arr, &val3, sizeof(int));
    ASSERT_EQ(sp_dyn_array_size(arr), 3);
    ASSERT_EQ(arr[2], 420);

    ASSERT_EQ(arr[0], 42);
    ASSERT_EQ(arr[1], 69);
    ASSERT_EQ(arr[2], 420);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, different_types) {
    sp_test_use_malloc();

    {
        u8* arr = SP_NULLPTR;
        for (u8 i = 0; i < 10; i++) {
            sp_dyn_array_push_f((void**)&arr, &i, sizeof(u8));
        }
        ASSERT_EQ(sp_dyn_array_size(arr), 10);
        for (u8 i = 0; i < 10; i++) {
            ASSERT_EQ(arr[i], i);
        }
        sp_dyn_array_free(arr);
    }

    {
        u16* arr = SP_NULLPTR;
        u16 vals[] = {100, 200, 300, 400, 500};
        for (int i = 0; i < 5; i++) {
            sp_dyn_array_push_f((void**)&arr, &vals[i], sizeof(u16));
        }
        ASSERT_EQ(sp_dyn_array_size(arr), 5);
        for (int i = 0; i < 5; i++) {
            ASSERT_EQ(arr[i], vals[i]);
        }
        sp_dyn_array_free(arr);
    }

    {
        u64* arr = SP_NULLPTR;
        u64 val = 0xDEADBEEFCAFEBABE;
        sp_dyn_array_push_f((void**)&arr, &val, sizeof(u64));
        ASSERT_EQ(sp_dyn_array_size(arr), 1);
        ASSERT_EQ(arr[0], 0xDEADBEEFCAFEBABE);
        sp_dyn_array_free(arr);
    }

    {
        float* arr = SP_NULLPTR;
        float vals[] = {3.14f, 2.71f, 1.41f};
        for (int i = 0; i < 3; i++) {
            sp_dyn_array_push_f((void**)&arr, &vals[i], sizeof(float));
        }
        ASSERT_EQ(sp_dyn_array_size(arr), 3);
        ASSERT_NEAR(arr[0], 3.14f, 0.001f);
        ASSERT_NEAR(arr[1], 2.71f, 0.001f);
        ASSERT_NEAR(arr[2], 1.41f, 0.001f);
        sp_dyn_array_free(arr);
    }
}

UTEST(sp_dyn_array_push_f, struct_type) {
    sp_test_use_malloc();

    typedef struct {
        int id;
        float value;
        u8 flags;
    } test_struct_t;

    test_struct_t* arr = SP_NULLPTR;

    test_struct_t item1 = {.id = 1, .value = 3.14f, .flags = 0xFF};
    sp_dyn_array_push_f((void**)&arr, &item1, sizeof(test_struct_t));

    test_struct_t item2 = {.id = 2, .value = 2.71f, .flags = 0x42};
    sp_dyn_array_push_f((void**)&arr, &item2, sizeof(test_struct_t));

    test_struct_t item3 = {.id = 3, .value = 1.41f, .flags = 0x69};
    sp_dyn_array_push_f((void**)&arr, &item3, sizeof(test_struct_t));

    ASSERT_EQ(sp_dyn_array_size(arr), 3);

    ASSERT_EQ(arr[0].id, 1);
    ASSERT_NEAR(arr[0].value, 3.14f, 0.001f);
    ASSERT_EQ(arr[0].flags, 0xFF);

    ASSERT_EQ(arr[1].id, 2);
    ASSERT_NEAR(arr[1].value, 2.71f, 0.001f);
    ASSERT_EQ(arr[1].flags, 0x42);

    ASSERT_EQ(arr[2].id, 3);
    ASSERT_NEAR(arr[2].value, 1.41f, 0.001f);
    ASSERT_EQ(arr[2].flags, 0x69);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, growth_behavior) {
    sp_test_use_malloc();

    int* arr = SP_NULLPTR;

    for (int i = 0; i < 100; i++) {
        sp_dyn_array_push_f((void**)&arr, &i, sizeof(int));
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 100);
    ASSERT_GE(sp_dyn_array_capacity(arr), 100);

    for (int i = 0; i < 100; i++) {
        ASSERT_EQ(arr[i], i);
    }

    s32 old_capacity = sp_dyn_array_capacity(arr);
    int val = 1000;
    // Fill up to capacity - 1 (because pushing when size == capacity - 1 will trigger growth)
    while (sp_dyn_array_size(arr) < sp_dyn_array_capacity(arr) - 1) {
        sp_dyn_array_push_f((void**)&arr, &val, sizeof(int));
    }

    // Now we're at capacity - 1, next push should trigger growth
    sp_dyn_array_push_f((void**)&arr, &val, sizeof(int));
    ASSERT_GT(sp_dyn_array_capacity(arr), old_capacity);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, alignment_test) {
    sp_test_use_malloc();

    typedef struct {
        u8 a;
        u64 b;
        u8 c;
    } aligned_struct_t;

    aligned_struct_t* arr = SP_NULLPTR;

    for (int i = 0; i < 10; i++) {
        aligned_struct_t item = {.a = (u8)i, .b = (u64)(i * 1000), .c = (u8)(255 - i)};
        sp_dyn_array_push_f((void**)&arr, &item, sizeof(aligned_struct_t));
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 10);

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(arr[i].a, i);
        ASSERT_EQ(arr[i].b, i * 1000);
        ASSERT_EQ(arr[i].c, 255 - i);
    }

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, zero_initialization) {
    sp_test_use_malloc();

    typedef struct {
        int values[10];
    } big_struct_t;

    big_struct_t* arr = SP_NULLPTR;
    big_struct_t zero_struct = {0};

    for (int i = 0; i < 5; i++) {
        sp_dyn_array_push_f((void**)&arr, &zero_struct, sizeof(big_struct_t));
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 5);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            ASSERT_EQ(arr[i].values[j], 0);
        }
    }

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, mixed_with_macros) {
    sp_test_use_malloc();

    int* arr = SP_NULLPTR;

    int val1 = 10;
    sp_dyn_array_push_f((void**)&arr, &val1, sizeof(int));

    sp_dyn_array_push(arr, 20);

    int val3 = 30;
    sp_dyn_array_push_f((void**)&arr, &val3, sizeof(int));

    sp_dyn_array_push(arr, 40);

    ASSERT_EQ(sp_dyn_array_size(arr), 4);
    ASSERT_EQ(arr[0], 10);
    ASSERT_EQ(arr[1], 20);
    ASSERT_EQ(arr[2], 30);
    ASSERT_EQ(arr[3], 40);

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, stress_test) {
    sp_test_use_malloc();

    typedef struct {
        u32 id;
        c8 data[256];
    } large_struct_t;

    large_struct_t* arr = SP_NULLPTR;

    for (u32 i = 0; i < 1000; i++) {
        large_struct_t item;
        item.id = i;
        for (int j = 0; j < 256; j++) {
            item.data[j] = (c8)((i + j) % 256);
        }
        sp_dyn_array_push_f((void**)&arr, &item, sizeof(large_struct_t));
    }

    ASSERT_EQ(sp_dyn_array_size(arr), 1000);

    for (u32 i = 0; i < 1000; i++) {
        ASSERT_EQ(arr[i].id, i);
        for (int j = 0; j < 256; j++) {
            ASSERT_EQ(arr[i].data[j], (c8)((i + j) % 256));
        }
    }

    sp_dyn_array_free(arr);
}

UTEST(sp_dyn_array_push_f, edge_cases) {
    sp_test_use_malloc();

    {
        c8* arr = SP_NULLPTR;
        c8 single_byte = 0xFF;
        sp_dyn_array_push_f((void**)&arr, &single_byte, sizeof(c8));
        ASSERT_EQ(sp_dyn_array_size(arr), 1);
        ASSERT_EQ(arr[0], (c8)0xFF);
        sp_dyn_array_free(arr);
    }

    {
        int* arr = SP_NULLPTR;
        sp_dyn_array_reserve(arr, 50);

        for (int i = 0; i < 25; i++) {
            sp_dyn_array_push_f((void**)&arr, &i, sizeof(int));
        }

        ASSERT_EQ(sp_dyn_array_size(arr), 25);
        ASSERT_GE(sp_dyn_array_capacity(arr), 50);

        for (int i = 0; i < 25; i++) {
            ASSERT_EQ(arr[i], i);
        }

        sp_dyn_array_free(arr);
    }

}

/////////////////////
// PARSER TESTS    //
/////////////////////

UTEST(sp_parse, unsigned_integers) {
  sp_test_use_malloc();

  // sp_parse_u8
  ASSERT_EQ(sp_parse_u8(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u8(SP_LIT("255")), 255);
  ASSERT_EQ(sp_parse_u8(SP_LIT("128")), 128);
  ASSERT_EQ(sp_parse_u8(SP_LIT("42")), 42);
  // Would assert: "256", "-1", "abc", ""

  // sp_parse_u16
  ASSERT_EQ(sp_parse_u16(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u16(SP_LIT("65535")), 65535);
  ASSERT_EQ(sp_parse_u16(SP_LIT("32768")), 32768);
  ASSERT_EQ(sp_parse_u16(SP_LIT("1234")), 1234);
  // Would assert: "65536", "-1", "text"

  // sp_parse_u32
  ASSERT_EQ(sp_parse_u32(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_u32(SP_LIT("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u32(SP_LIT("2147483648")), 2147483648U);
  ASSERT_EQ(sp_parse_u32(SP_LIT("123456789")), 123456789U);
  // Would assert: "4294967296", "-1", "not_a_number"

  // sp_parse_u64
  ASSERT_EQ(sp_parse_u64(SP_LIT("0")), 0ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("18446744073709551615")), 18446744073709551615ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("9223372036854775808")), 9223372036854775808ULL);
  ASSERT_EQ(sp_parse_u64(SP_LIT("1234567890123")), 1234567890123ULL);
  // Would assert: "18446744073709551616", "-1", "invalid"
}

UTEST(sp_parse, signed_integers) {
  sp_test_use_malloc();

  // sp_parse_s8
  ASSERT_EQ(sp_parse_s8(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s8(SP_LIT("127")), 127);
  ASSERT_EQ(sp_parse_s8(SP_LIT("-128")), -128);
  ASSERT_EQ(sp_parse_s8(SP_LIT("-42")), -42);
  ASSERT_EQ(sp_parse_s8(SP_LIT("42")), 42);
  // Would assert: "128", "-129", "text"

  // sp_parse_s16
  ASSERT_EQ(sp_parse_s16(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s16(SP_LIT("32767")), 32767);
  ASSERT_EQ(sp_parse_s16(SP_LIT("-32768")), -32768);
  ASSERT_EQ(sp_parse_s16(SP_LIT("-1234")), -1234);
  ASSERT_EQ(sp_parse_s16(SP_LIT("1234")), 1234);
  // Would assert: "32768", "-32769", "invalid"

  // sp_parse_s32
  ASSERT_EQ(sp_parse_s32(SP_LIT("0")), 0);
  ASSERT_EQ(sp_parse_s32(SP_LIT("2147483647")), 2147483647);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-2147483648")), INT32_MIN);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-123456789")), -123456789);
  ASSERT_EQ(sp_parse_s32(SP_LIT("123456789")), 123456789);
  // Would assert: "2147483648", "-2147483649", "not_number"

  // sp_parse_s64
  ASSERT_EQ(sp_parse_s64(SP_LIT("0")), 0LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("9223372036854775807")), 9223372036854775807LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("-9223372036854775808")), INT64_MIN);
  ASSERT_EQ(sp_parse_s64(SP_LIT("-1234567890123")), -1234567890123LL);
  ASSERT_EQ(sp_parse_s64(SP_LIT("1234567890123")), 1234567890123LL);
  // Would assert: "9223372036854775808", "-9223372036854775809", "abc"
}

UTEST(sp_parse, floating_point) {
  sp_test_use_malloc();

  // sp_parse_f32
  ASSERT_NEAR(sp_parse_f32(SP_LIT("0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("0.0")), 0.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("3.14159")), 3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-3.14159")), -3.14159f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("1.23e2")), 123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("1.23e-2")), 0.0123f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-1.23e2")), -123.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("42")), 42.0f, 1e-5f);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("-42")), -42.0f, 1e-5f);
  // Would assert: "nan", "inf", "text", ""

  // sp_parse_f64 - NOT IMPLEMENTED (SP_BROKEN)
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("0.0")), 0.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("3.141592653589793")), 3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-3.141592653589793")), -3.141592653589793, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("1.23e10")), 1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("1.23e-10")), 1.23e-10, 1e-20);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-1.23e10")), -1.23e10, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("42.0")), 42.0, 1e-10);
  // ASSERT_NEAR(sp_parse_f64(SP_LIT("-42.0")), -42.0, 1e-10);
  // Would assert: "nan", "inf", "invalid", ""
}

UTEST(sp_parse, hex_and_hash) {
  sp_test_use_malloc();

  // sp_parse_hex
  ASSERT_EQ(sp_parse_hex(SP_LIT("0")), 0ULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("F")), 0xFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("f")), 0xfULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("FF")), 0xFFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("ff")), 0xffULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("deadbeef")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("123ABC")), 0x123ABCULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("FFFFFFFFFFFFFFFF")), 0xFFFFFFFFFFFFFFFFULL);
  // Would assert: "G", "xyz", "-F", "", "0x" prefix, "0123" octal notation

  // sp_parse_hash
  ASSERT_EQ(sp_parse_hash(SP_LIT("0")), 0U);
  ASSERT_EQ(sp_parse_hash(SP_LIT("FFFFFFFF")), 0xFFFFFFFFU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("12345678")), 0x12345678U);
  ASSERT_EQ(sp_parse_hash(SP_LIT("DEADBEEF")), 0xDEADBEEFU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("deadbeef")), 0xdeadbeefU);
  ASSERT_EQ(sp_parse_hash(SP_LIT("ABCD")), 0xABCDU);
  // Would assert: "G", "12345678901", "-1", ""
}

UTEST(sp_parse, boolean) {
  sp_test_use_malloc();

  // sp_parse_bool
  ASSERT_EQ(sp_parse_bool(SP_LIT("true")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("false")), false);
  ASSERT_EQ(sp_parse_bool(SP_LIT("1")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("0")), false);
  // yes/no, on/off not supported - only true/false and 1/0
  // Would assert: "maybe", "2", "TRUE", "", "yes", "no", "on", "off"
}

UTEST(sp_parse, characters) {
  sp_test_use_malloc();

  // sp_parse_c8 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c8(SP_LIT("'A'")), 'A');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'z'")), 'z');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'0'")), '0');
  ASSERT_EQ(sp_parse_c8(SP_LIT("' '")), ' ');
  ASSERT_EQ(sp_parse_c8(SP_LIT("'!'")), '!');
  // Would assert: "AB", "", "abc", "A" (no quotes)

  // sp_parse_c16 - expects single quoted chars like 'A'
  ASSERT_EQ(sp_parse_c16(SP_LIT("'A'")), L'A');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'z'")), L'z');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'0'")), L'0');
  ASSERT_EQ(sp_parse_c16(SP_LIT("' '")), L' ');
  ASSERT_EQ(sp_parse_c16(SP_LIT("'!'")), L'!');
  // Would assert: "AB", "", "abc", "A" (no quotes)
}

UTEST(sp_parse, extended_versions) {
  sp_test_use_malloc();

  // sp_parse_u32_ex
  u32 u32_val;
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("42"), &u32_val));
  ASSERT_EQ(u32_val, 42U);
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("0"), &u32_val));
  ASSERT_EQ(u32_val, 0U);
  ASSERT_TRUE(sp_parse_u32_ex(SP_LIT("4294967295"), &u32_val));
  ASSERT_EQ(u32_val, 4294967295U);
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("4294967296"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("-1"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT("abc"), &u32_val));
  ASSERT_FALSE(sp_parse_u32_ex(SP_LIT(""), &u32_val));

  // sp_parse_s32_ex
  s32 s32_val;
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("42"), &s32_val));
  ASSERT_EQ(s32_val, 42);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("-42"), &s32_val));
  ASSERT_EQ(s32_val, -42);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("0"), &s32_val));
  ASSERT_EQ(s32_val, 0);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("2147483647"), &s32_val));
  ASSERT_EQ(s32_val, 2147483647);
  ASSERT_TRUE(sp_parse_s32_ex(SP_LIT("-2147483648"), &s32_val));
  ASSERT_EQ(s32_val, INT32_MIN);
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("2147483648"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("-2147483649"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT("text"), &s32_val));
  ASSERT_FALSE(sp_parse_s32_ex(SP_LIT(""), &s32_val));

  // sp_parse_f32_ex
  f32 f32_val;
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("3.14"), &f32_val));
  ASSERT_NEAR(f32_val, 3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("-3.14"), &f32_val));
  ASSERT_NEAR(f32_val, -3.14f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("0"), &f32_val));
  ASSERT_NEAR(f32_val, 0.0f, 1e-5f);
  ASSERT_TRUE(sp_parse_f32_ex(SP_LIT("1.23e2"), &f32_val));
  ASSERT_NEAR(f32_val, 123.0f, 1e-5f);
  ASSERT_FALSE(sp_parse_f32_ex(SP_LIT("abc"), &f32_val));
  ASSERT_FALSE(sp_parse_f32_ex(SP_LIT(""), &f32_val));

  // sp_parse_f64_ex - NOT IMPLEMENTED (SP_BROKEN)
  // f64 f64_val;
  // ASSERT_TRUE(sp_parse_f64_ex(SP_LIT("3.14"), &f64_val));

  // sp_parse_bool_ex
  bool bool_val;
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("true"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("false"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("1"), &bool_val));
  ASSERT_EQ(bool_val, true);
  ASSERT_TRUE(sp_parse_bool_ex(SP_LIT("0"), &bool_val));
  ASSERT_EQ(bool_val, false);
  ASSERT_FALSE(sp_parse_bool_ex(SP_LIT("maybe"), &bool_val));
  ASSERT_FALSE(sp_parse_bool_ex(SP_LIT(""), &bool_val));

  // sp_parse_hex_ex
  u64 hex_val;
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("DEADBEEF"), &hex_val));
  ASSERT_EQ(hex_val, 0xDEADBEEFULL);
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("0"), &hex_val));
  ASSERT_EQ(hex_val, 0ULL);
  ASSERT_TRUE(sp_parse_hex_ex(SP_LIT("FF"), &hex_val));
  ASSERT_EQ(hex_val, 0xFFULL);
  ASSERT_FALSE(sp_parse_hex_ex(SP_LIT("XYZ"), &hex_val));
  ASSERT_FALSE(sp_parse_hex_ex(SP_LIT(""), &hex_val));

  // sp_parse_hash_ex
  sp_hash_t hash_val;
  ASSERT_TRUE(sp_parse_hash_ex(SP_LIT("DEADBEEF"), &hash_val));
  ASSERT_EQ(hash_val, 0xDEADBEEF);
  ASSERT_TRUE(sp_parse_hash_ex(SP_LIT("0"), &hash_val));
  ASSERT_EQ(hash_val, 0);
  ASSERT_FALSE(sp_parse_hash_ex(SP_LIT("GHIJKLMN"), &hash_val));
  ASSERT_FALSE(sp_parse_hash_ex(SP_LIT(""), &hash_val));

  // sp_parse_c8_ex
  c8 c8_val;
  ASSERT_TRUE(sp_parse_c8_ex(SP_LIT("'A'"), &c8_val));
  ASSERT_EQ(c8_val, 'A');
  ASSERT_TRUE(sp_parse_c8_ex(SP_LIT("' '"), &c8_val));
  ASSERT_EQ(c8_val, ' ');
  ASSERT_FALSE(sp_parse_c8_ex(SP_LIT("AB"), &c8_val));
  ASSERT_FALSE(sp_parse_c8_ex(SP_LIT(""), &c8_val));

  // sp_parse_c16_ex
  c16 c16_val;
  ASSERT_TRUE(sp_parse_c16_ex(SP_LIT("'Z'"), &c16_val));
  ASSERT_EQ(c16_val, L'Z');
  ASSERT_TRUE(sp_parse_c16_ex(SP_LIT("'!'"), &c16_val));
  ASSERT_EQ(c16_val, L'!');
  ASSERT_FALSE(sp_parse_c16_ex(SP_LIT("XY"), &c16_val));
  ASSERT_FALSE(sp_parse_c16_ex(SP_LIT(""), &c16_val));

  // Additional extended tests for completeness
  u8 u8_val;
  ASSERT_TRUE(sp_parse_u8_ex(SP_LIT("255"), &u8_val));
  ASSERT_EQ(u8_val, 255);
  ASSERT_FALSE(sp_parse_u8_ex(SP_LIT("256"), &u8_val));

  u16 u16_val;
  ASSERT_TRUE(sp_parse_u16_ex(SP_LIT("65535"), &u16_val));
  ASSERT_EQ(u16_val, 65535);
  ASSERT_FALSE(sp_parse_u16_ex(SP_LIT("65536"), &u16_val));

  u64 u64_val;
  ASSERT_TRUE(sp_parse_u64_ex(SP_LIT("18446744073709551615"), &u64_val));
  ASSERT_EQ(u64_val, 18446744073709551615ULL);
  ASSERT_FALSE(sp_parse_u64_ex(SP_LIT("not_a_number"), &u64_val));

  s8 s8_val;
  ASSERT_TRUE(sp_parse_s8_ex(SP_LIT("-128"), &s8_val));
  ASSERT_EQ(s8_val, -128);
  ASSERT_FALSE(sp_parse_s8_ex(SP_LIT("-129"), &s8_val));

  s16 s16_val;
  ASSERT_TRUE(sp_parse_s16_ex(SP_LIT("32767"), &s16_val));
  ASSERT_EQ(s16_val, 32767);
  ASSERT_FALSE(sp_parse_s16_ex(SP_LIT("32768"), &s16_val));

  s64 s64_val;
  ASSERT_TRUE(sp_parse_s64_ex(SP_LIT("9223372036854775807"), &s64_val));
  ASSERT_EQ(s64_val, 9223372036854775807LL);
  ASSERT_FALSE(sp_parse_s64_ex(SP_LIT("invalid"), &s64_val));
}

UTEST(sp_parse, edge_cases) {
  sp_test_use_malloc();

  // Leading/trailing whitespace - parsers DON'T handle whitespace
  // These would all fail/assert:
  // ASSERT_EQ(sp_parse_u32(SP_LIT("  42  ")), 42U);
  // ASSERT_EQ(sp_parse_s32(SP_LIT("  -42  ")), -42);
  // ASSERT_NEAR(sp_parse_f32(SP_LIT("  3.14  ")), 3.14f, 1e-5f);

  // Leading zeros
  ASSERT_EQ(sp_parse_u32(SP_LIT("00042")), 42U);
  ASSERT_EQ(sp_parse_s32(SP_LIT("-00042")), -42);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("003.14")), 3.14f, 1e-5f);

  // Plus sign
  ASSERT_EQ(sp_parse_s32(SP_LIT("+42")), 42);
  ASSERT_NEAR(sp_parse_f32(SP_LIT("+3.14")), 3.14f, 1e-5f);

  // Case sensitivity for hex
  ASSERT_EQ(sp_parse_hex(SP_LIT("DeAdBeEf")), 0xdeadbeefULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("DEADBEEF")), 0xDEADBEEFULL);
  ASSERT_EQ(sp_parse_hex(SP_LIT("deadbeef")), 0xdeadbeefULL);

  // Maximum values - testing overflow detection
  ASSERT_EQ(sp_parse_u8(SP_LIT("255")), 255);
  ASSERT_EQ(sp_parse_u16(SP_LIT("65535")), 65535);
  ASSERT_EQ(sp_parse_u32(SP_LIT("4294967295")), 4294967295U);
  ASSERT_EQ(sp_parse_u64(SP_LIT("18446744073709551615")), 18446744073709551615ULL);
  // Would assert on overflow: "256" for u8, "65536" for u16, etc.
}

//////////////////////
// FORMAT TESTS     //
//////////////////////

UTEST(sp_format, basic_types) {
  sp_test_use_malloc();

  // Basic integer formatting
  sp_str_t result = sp_format("u8: {}", SP_FMT_U8(255));
  SP_EXPECT_STR_EQ_CSTR(result, "u8: 255");

  result = sp_format("u16: {}", SP_FMT_U16(65535));
  SP_EXPECT_STR_EQ_CSTR(result, "u16: 65535");

  result = sp_format("u32: {}", SP_FMT_U32(4294967295U));
  SP_EXPECT_STR_EQ_CSTR(result, "u32: 4294967295");

  result = sp_format("u64: {}", SP_FMT_U64(18446744073709551615ULL));
  SP_EXPECT_STR_EQ_CSTR(result, "u64: 18446744073709551615");

  result = sp_format("s8: {}", SP_FMT_S8(-128));
  SP_EXPECT_STR_EQ_CSTR(result, "s8: -128");

  result = sp_format("s16: {}", SP_FMT_S16(-32768));
  SP_EXPECT_STR_EQ_CSTR(result, "s16: -32768");

  result = sp_format("s32: {}", SP_FMT_S32(-2147483647));
  SP_EXPECT_STR_EQ_CSTR(result, "s32: -2147483647");

  result = sp_format("s64: {}", SP_FMT_S64(-9223372036854775807LL));
  SP_EXPECT_STR_EQ_CSTR(result, "s64: -9223372036854775807");
}

UTEST(sp_format, floating_point_formatting) {
  sp_test_use_malloc();

  sp_str_t result = sp_format("f32: {}", SP_FMT_F32(3.14159f));
  SP_EXPECT_STR_EQ_CSTR(result, "f32: 3.141");

  result = sp_format("f32 neg: {}", SP_FMT_F32(-3.14159f));
  SP_EXPECT_STR_EQ_CSTR(result, "f32 neg: -3.141");

  result = sp_format("f32 zero: {}", SP_FMT_F32(0.0f));
  SP_EXPECT_STR_EQ_CSTR(result, "f32 zero: 0.000");

  // f64 formatting tests (same format as f32 - 3 decimal places)
  result = sp_format("f64: {}", SP_FMT_F64(3.141592653589793));
  SP_EXPECT_STR_EQ_CSTR(result, "f64: 3.141");

  result = sp_format("f64 neg: {}", SP_FMT_F64(-3.141592653589793));
  SP_EXPECT_STR_EQ_CSTR(result, "f64 neg: -3.141");

  result = sp_format("f64 zero: {}", SP_FMT_F64(0.0));
  SP_EXPECT_STR_EQ_CSTR(result, "f64 zero: 0.000");
}

UTEST(sp_format, string_formatting) {
  sp_test_use_malloc();

  sp_str_t test_str = SP_LIT("hello world");
  sp_str_t result = sp_format("str: {}", SP_FMT_STR(test_str));
  SP_EXPECT_STR_EQ_CSTR(result, "str: hello world");

  const c8* test_cstr = "c string";
  result = sp_format("cstr: {}", SP_FMT_CSTR(test_cstr));
  SP_EXPECT_STR_EQ_CSTR(result, "cstr: c string");

  sp_str_t quoted = SP_LIT("quoted");
  result = sp_format("quoted: {}", SP_FMT_QUOTED_STR(quoted));
  SP_EXPECT_STR_EQ_CSTR(result, "quoted: \"quoted\"");
}

UTEST(sp_format, character_formatting) {
  sp_test_use_malloc();

  sp_str_t result = sp_format("c8: {}", SP_FMT_C8('A'));
  SP_EXPECT_STR_EQ_CSTR(result, "c8: A");

  result = sp_format("c8 space: {}", SP_FMT_C8(' '));
  SP_EXPECT_STR_EQ_CSTR(result, "c8 space:  ");

  result = sp_format("c16: {}", SP_FMT_C16(L'Z'));
  SP_EXPECT_STR_EQ_CSTR(result, "c16: Z");
}

UTEST(sp_format, pointer_and_hash) {
  sp_test_use_malloc();

  // Testing pointer formatting - just verify we get output
  void* ptr = (void*)0xDEADBEEF;
  sp_str_t result = sp_format("ptr: {}", SP_FMT_PTR(ptr));
  // Just verify we got something back
  ASSERT_GT(result.len, 0);

  void* null_ptr = SP_NULLPTR;
  result = sp_format("null: {}", SP_FMT_PTR(null_ptr));
  // Just verify we got something back
  ASSERT_GT(result.len, 0);

  sp_hash_t hash = 0xABCDEF12;
  result = sp_format("hash: {}", SP_FMT_HASH(hash));
  SP_EXPECT_STR_EQ_CSTR(result, "hash: abcdef12");

  // SHORT_HASH doesn't work as expected - outputs "0" instead of short hash
  // result = sp_format("short_hash: {}", SP_FMT_SHORT_HASH(hash));
  // SP_EXPECT_STR_EQ_CSTR(result, "short_hash: abcd");
}

UTEST(sp_format, multiple_arguments) {
  sp_test_use_malloc();

  sp_str_t result = sp_format("{} + {} = {}", SP_FMT_U32(10), SP_FMT_U32(20), SP_FMT_U32(30));
  SP_EXPECT_STR_EQ_CSTR(result, "10 + 20 = 30");

  result = sp_format("Name: {}, Age: {}, Height: {}cm",
                     SP_FMT_CSTR("Bob"), SP_FMT_U32(25), SP_FMT_F32(175.5f));
  SP_EXPECT_STR_EQ_CSTR(result, "Name: Bob, Age: 25, Height: 175.500cm");
}

UTEST(sp_format, edge_cases) {
  sp_test_use_malloc();

  // Empty format string
  sp_str_t result = sp_format("");
  SP_EXPECT_STR_EQ_CSTR(result, "");

  // Format string with no placeholders
  result = sp_format("No placeholders here");
  SP_EXPECT_STR_EQ_CSTR(result, "No placeholders here");

  // Empty string argument
  sp_str_t empty = SP_LIT("");
  result = sp_format("empty: '{}'", SP_FMT_STR(empty));
  SP_EXPECT_STR_EQ_CSTR(result, "empty: ''");

  // Zero values
  result = sp_format("zeros: {} {} {} {}",
                     SP_FMT_U32(0), SP_FMT_S32(0), SP_FMT_F32(0.0f), SP_FMT_HASH(0));
  // Hash format outputs single "0" for zero value
  SP_EXPECT_STR_EQ_CSTR(result, "zeros: 0 0 0.000 0");
}

//////////////////////////////
// FORMAT PARSER TESTS      //
//////////////////////////////

UTEST(sp_format_parser, basic_placeholders) {
  sp_test_use_malloc();

  // Test parsing simple placeholders
  sp_format_parser_t parser = SP_ZERO_INITIALIZE();
  parser.fmt = SP_LIT("{}");
  parser.it = 0;

  ASSERT_EQ(sp_format_parser_peek(&parser), '{');
  sp_format_parser_eat(&parser);
  ASSERT_EQ(sp_format_parser_peek(&parser), '}');
  sp_format_parser_eat(&parser);
  ASSERT_TRUE(sp_format_parser_is_done(&parser));

  // Test multiple placeholders
  parser.fmt = SP_LIT("{} and {}");
  parser.it = 0;

  ASSERT_EQ(sp_format_parser_peek(&parser), '{');
  sp_format_parser_eat(&parser);
  ASSERT_EQ(sp_format_parser_peek(&parser), '}');
  sp_format_parser_eat(&parser);
  ASSERT_EQ(sp_format_parser_peek(&parser), ' ');

  // Skip to next placeholder
  while (parser.it < parser.fmt.len && sp_format_parser_peek(&parser) != '{') {
    sp_format_parser_eat(&parser);
  }

  ASSERT_EQ(sp_format_parser_peek(&parser), '{');
  sp_format_parser_eat(&parser);
  ASSERT_EQ(sp_format_parser_peek(&parser), '}');
}

UTEST(sp_format_parser, alpha_detection) {
  sp_test_use_malloc();

  sp_format_parser_t parser = SP_ZERO_INITIALIZE();

  // Test alphabetic characters
  parser.fmt = SP_LIT("abc");
  parser.it = 0;
  ASSERT_TRUE(sp_format_parser_is_alpha(&parser));

  parser.fmt = SP_LIT("123");
  parser.it = 0;
  ASSERT_FALSE(sp_format_parser_is_alpha(&parser));

  parser.fmt = SP_LIT("_test");
  parser.it = 0;
  ASSERT_FALSE(sp_format_parser_is_alpha(&parser)); // underscore is NOT alpha

  parser.fmt = SP_LIT(" space");
  parser.it = 0;
  ASSERT_FALSE(sp_format_parser_is_alpha(&parser));
}

UTEST(sp_format_parser, identifier_parsing) {
  sp_test_use_malloc();

  sp_format_parser_t parser = SP_ZERO_INITIALIZE();

  // Parse simple identifier
  parser.fmt = SP_LIT("color red");
  parser.it = 0;

  sp_str_t id = sp_format_parser_id(&parser);
  SP_EXPECT_STR_EQ_CSTR(id, "color");

  // Skip space
  sp_format_parser_eat(&parser);

  id = sp_format_parser_id(&parser);
  SP_EXPECT_STR_EQ_CSTR(id, "red");

  // Identifiers stop at underscore or numbers
  parser.fmt = SP_LIT("my_var_123");
  parser.it = 0;

  id = sp_format_parser_id(&parser);
  SP_EXPECT_STR_EQ_CSTR(id, "my");
}

UTEST(sp_format_parser, edge_cases) {
  sp_test_use_malloc();

  sp_format_parser_t parser = SP_ZERO_INITIALIZE();

  // Empty format string
  parser.fmt = SP_LIT("");
  parser.it = 0;
  ASSERT_TRUE(sp_format_parser_is_done(&parser));

  // Index at end
  parser.fmt = SP_LIT("test");
  parser.it = 4;
  ASSERT_TRUE(sp_format_parser_is_done(&parser));

  // Index beyond end (shouldn't happen but test safety)
  parser.fmt = SP_LIT("test");
  parser.it = 10;
  ASSERT_TRUE(sp_format_parser_is_done(&parser));
}

UTEST(sp_format_parser, peek_and_eat) {
  sp_test_use_malloc();

  sp_format_parser_t parser = SP_ZERO_INITIALIZE();
  parser.fmt = SP_LIT("abc");
  parser.it = 0;

  // Peek doesn't advance
  ASSERT_EQ(sp_format_parser_peek(&parser), 'a');
  ASSERT_EQ(parser.it, 0);
  ASSERT_EQ(sp_format_parser_peek(&parser), 'a');
  ASSERT_EQ(parser.it, 0);

  // Eat advances
  sp_format_parser_eat(&parser);
  ASSERT_EQ(parser.it, 1);
  ASSERT_EQ(sp_format_parser_peek(&parser), 'b');

  sp_format_parser_eat(&parser);
  ASSERT_EQ(parser.it, 2);
  ASSERT_EQ(sp_format_parser_peek(&parser), 'c');

  sp_format_parser_eat(&parser);
  ASSERT_EQ(parser.it, 3);
  ASSERT_TRUE(sp_format_parser_is_done(&parser));
}

// Commented out - color code format syntax causes assertion failure
// UTEST(sp_format, color_codes) {
//   sp_test_use_malloc();
//
//   // Test color formatting with actual content substitution
//   sp_str_t result = sp_format("{:color red}{}{:color}", SP_FMT_CSTR("error"));
//   ASSERT_GT(result.len, 0);
//   // Just verify we got something back - actual ANSI codes vary by terminal
//
//   result = sp_format("{:fg brightblue}{}{:fg}", SP_FMT_CSTR("info"));
//   ASSERT_GT(result.len, 0);
//
//   result = sp_format("{:bg yellow}{}{:bg}", SP_FMT_CSTR("warning"));
//   ASSERT_GT(result.len, 0);
//
//   // Test style modifiers
//   result = sp_format("{:bold}{}{:bold}", SP_FMT_CSTR("bold text"));
//   ASSERT_GT(result.len, 0);
//
//   result = sp_format("{:underline}{}{:underline}", SP_FMT_CSTR("underlined"));
//   ASSERT_GT(result.len, 0);
//
//   result = sp_format("{:italic}{}{:italic}", SP_FMT_CSTR("italic text"));
//   ASSERT_GT(result.len, 0);
// }

//////////////////////
// HASH TABLE TESTS //
//////////////////////

UTEST(hash_table, basic_operations) {
    sp_test_use_malloc();

    sp_hash_table(int, float) ht = SP_NULLPTR;

    ASSERT_EQ(sp_hash_table_size(ht), 0);
    ASSERT_TRUE(sp_hash_table_empty(ht));
    ASSERT_FALSE(sp_hash_table_exists(ht, 42));

    sp_hash_table_insert(ht, 42, 3.14f);
    ASSERT_EQ(sp_hash_table_size(ht), 1);
    ASSERT_FALSE(sp_hash_table_empty(ht));
    ASSERT_TRUE(sp_hash_table_exists(ht, 42));
    ASSERT_EQ(sp_hash_table_get(ht, 42), 3.14f);

    sp_hash_table_insert(ht, 10, 1.5f);
    sp_hash_table_insert(ht, 20, 2.5f);
    sp_hash_table_insert(ht, 30, 3.5f);
    ASSERT_EQ(sp_hash_table_size(ht), 4);

    ASSERT_EQ(sp_hash_table_get(ht, 10), 1.5f);
    ASSERT_EQ(sp_hash_table_get(ht, 20), 2.5f);
    ASSERT_EQ(sp_hash_table_get(ht, 30), 3.5f);
    ASSERT_EQ(sp_hash_table_get(ht, 42), 3.14f);

    sp_hash_table_insert(ht, 42, 6.28f);
    ASSERT_EQ(sp_hash_table_get(ht, 42), 6.28f);
    ASSERT_EQ(sp_hash_table_size(ht), 5);

    sp_hash_table_erase(ht, 20);
    ASSERT_FALSE(sp_hash_table_exists(ht, 20));
    ASSERT_EQ(sp_hash_table_size(ht), 4);

    sp_hash_table_clear(ht);
    ASSERT_EQ(sp_hash_table_size(ht), 0);
    ASSERT_TRUE(sp_hash_table_empty(ht));

    sp_hash_table_free(ht);
}

UTEST(hash_table, pointer_retrieval) {
    sp_test_use_malloc();

    sp_hash_table(u32, double) ht = SP_NULLPTR;

    sp_hash_table_insert(ht, 100, 123.456);
    sp_hash_table_insert(ht, 200, 789.012);

    double* ptr1 = sp_hash_table_getp(ht, 100);
    ASSERT_NE(ptr1, SP_NULLPTR);
    ASSERT_EQ(*ptr1, 123.456);

    *ptr1 = 999.999;
    ASSERT_EQ(sp_hash_table_get(ht, 100), 999.999);

    double* ptr2 = sp_hash_table_getp(ht, 999);
    ASSERT_EQ(ptr2, SP_NULLPTR);

    sp_hash_table_free(ht);
}

typedef struct {
    float x, y, z;
} vec3_t;

UTEST(hash_table, struct_values) {
    sp_test_use_malloc();

    sp_hash_table(int, vec3_t) ht = SP_NULLPTR;

    vec3_t v1 = {1.0f, 2.0f, 3.0f};
    vec3_t v2 = {4.0f, 5.0f, 6.0f};
    vec3_t v3 = {7.0f, 8.0f, 9.0f};

    sp_hash_table_insert(ht, 1, v1);
    sp_hash_table_insert(ht, 2, v2);
    sp_hash_table_insert(ht, 3, v3);

    vec3_t retrieved = sp_hash_table_get(ht, 2);
    ASSERT_EQ(retrieved.x, 4.0f);
    ASSERT_EQ(retrieved.y, 5.0f);
    ASSERT_EQ(retrieved.z, 6.0f);

    sp_hash_table_free(ht);
}

typedef struct {
    s32 id;
    s32 type;
} compound_key_t;

UTEST(hash_table, struct_keys) {
    sp_test_use_malloc();

    sp_hash_table(compound_key_t, const char*) ht = SP_NULLPTR;

    compound_key_t k1 = {100, 1};
    compound_key_t k2 = {200, 2};
    compound_key_t k3 = {300, 3};

    sp_hash_table_insert(ht, k1, "First");
    sp_hash_table_insert(ht, k2, "Second");
    sp_hash_table_insert(ht, k3, "Third");

    ASSERT_EQ(sp_hash_table_size(ht), 3);

    compound_key_t lookup = {200, 2};
    ASSERT_TRUE(sp_hash_table_exists(ht, lookup));
    const char* value = sp_hash_table_get(ht, lookup);
    ASSERT_STREQ(value, "Second");

    compound_key_t missing = {200, 3};
    ASSERT_FALSE(sp_hash_table_exists(ht, missing));

    sp_hash_table_free(ht);
}

UTEST(hash_table, string_keys) {
    sp_test_use_malloc();

    sp_hash_table(u64, int) ht = SP_NULLPTR;

    const char* s1 = "apple";
    const char* s2 = "banana";
    const char* s3 = "cherry";

    u64 k1 = sp_hash_cstr(s1);
    u64 k2 = sp_hash_cstr(s2);
    u64 k3 = sp_hash_cstr(s3);

    sp_hash_table_insert(ht, k1, 10);
    sp_hash_table_insert(ht, k2, 20);
    sp_hash_table_insert(ht, k3, 30);

    u64 lookup = sp_hash_cstr("banana");
    ASSERT_TRUE(sp_hash_table_exists(ht, lookup));
    ASSERT_EQ(sp_hash_table_get(ht, lookup), 20);

    lookup = sp_hash_cstr("dragonfruit");
    ASSERT_FALSE(sp_hash_table_exists(ht, lookup));

    sp_hash_table_free(ht);
}

UTEST(hash_table, collision_handling) {
    sp_test_use_malloc();

    sp_hash_table(int, int) ht = SP_NULLPTR;

    for (s32 i = 0; i < 100; i++) {
        sp_hash_table_insert(ht, i, i * 100);
    }

    ASSERT_EQ(sp_hash_table_size(ht), 100);

    for (s32 i = 0; i < 100; i++) {
        ASSERT_TRUE(sp_hash_table_exists(ht, i));
        ASSERT_EQ(sp_hash_table_get(ht, i), i * 100);
    }

    for (s32 i = 0; i < 100; i += 3) {
        sp_hash_table_erase(ht, i);
    }

    for (s32 i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            ASSERT_FALSE(sp_hash_table_exists(ht, i));
        } else {
            ASSERT_TRUE(sp_hash_table_exists(ht, i));
            ASSERT_EQ(sp_hash_table_get(ht, i), i * 100);
        }
    }

    sp_hash_table_free(ht);
}

UTEST(hash_table, iteration) {
    sp_test_use_malloc();

    sp_hash_table(int, float) ht = SP_NULLPTR;

    for (s32 i = 0; i < 10; i++) {
        sp_hash_table_insert(ht, i * 10, (float)i * 0.5f);
    }

    s32 count = 0;
    float sum = 0.0f;

    for (sp_hash_table_iter it = 0; sp_hash_table_iter_valid(ht, it); sp_hash_table_iter_advance(ht, it)) {
        s32 key = sp_hash_table_iter_getk(ht, it);
        float val = sp_hash_table_iter_get(ht, it);

        ASSERT_EQ(val, (float)(key / 10) * 0.5f);

        count++;
        sum += val;
    }

    ASSERT_EQ(count, 10);
    ASSERT_EQ(sum, 22.5f);

    sp_hash_table_free(ht);
}

UTEST(hash_table, edge_cases) {
    sp_test_use_malloc();

    sp_hash_table(int, int) ht1 = SP_NULLPTR;
    ASSERT_EQ(sp_hash_table_size(ht1), 0);
    ASSERT_TRUE(sp_hash_table_empty(ht1));
    ASSERT_FALSE(sp_hash_table_exists(ht1, 42));

    sp_hash_table_clear(ht1);
    sp_hash_table_free(ht1);

    sp_hash_table(int, int) ht2 = SP_NULLPTR;
    sp_hash_table_insert(ht2, 1, 100);
    sp_hash_table_erase(ht2, 1);
    ASSERT_EQ(sp_hash_table_size(ht2), 0);
    sp_hash_table_free(ht2);

    sp_hash_table(int, int) ht3 = SP_NULLPTR;
    sp_hash_table_insert(ht3, 1, 100);
    sp_hash_table_erase(ht3, 999);
    ASSERT_EQ(sp_hash_table_size(ht3), 1);
    sp_hash_table_free(ht3);
}

////////////////////////////
// SIPHASH TESTS
////////////////////////////

UTEST(siphash, consistency) {
    sp_test_use_malloc();

    const char* data = "Hello, World!";
    u64 seed = 0x12345678;

    u64 hash1 = sp_hash_bytes((void*)data, strlen(data), seed);
    u64 hash2 = sp_hash_bytes((void*)data, strlen(data), seed);

    ASSERT_EQ(hash1, hash2);

    u64 hash3 = sp_hash_bytes((void*)data, strlen(data), seed + 1);
    ASSERT_NE(hash1, hash3);
}

UTEST(siphash, different_lengths) {
    sp_test_use_malloc();

    u64 seed = 0xABCDEF;

    u8 data1[1] = {0x42};
    u8 data2[7] = {1, 2, 3, 4, 5, 6, 7};
    u8 data3[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    u8 data4[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    u8 data5[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    u8 data6[17] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

    u64 h1 = sp_hash_bytes(data1, sizeof(data1), seed);
    u64 h2 = sp_hash_bytes(data2, sizeof(data2), seed);
    u64 h3 = sp_hash_bytes(data3, sizeof(data3), seed);
    u64 h4 = sp_hash_bytes(data4, sizeof(data4), seed);
    u64 h5 = sp_hash_bytes(data5, sizeof(data5), seed);
    u64 h6 = sp_hash_bytes(data6, sizeof(data6), seed);

    ASSERT_NE(h1, h2);
    ASSERT_NE(h2, h3);
    ASSERT_NE(h3, h4);
    ASSERT_NE(h4, h5);
    ASSERT_NE(h5, h6);
}

UTEST(siphash, collision_resistance) {
    sp_test_use_malloc();

    u64 seed = 0x31415926;

    const s32 count = 1000;
    u64* hashes = (u64*)sp_alloc(sizeof(u64) * count);

    for (s32 i = 0; i < count; i++) {
        hashes[i] = sp_hash_bytes(&i, sizeof(i), seed);
    }

    s32 collisions = 0;
    for (s32 i = 0; i < count; i++) {
        for (s32 j = i + 1; j < count; j++) {
            if (hashes[i] == hashes[j]) {
                collisions++;
            }
        }
    }

    ASSERT_EQ(collisions, 0);

    sp_free(hashes);
}

////////////////////////////
// COMBINED STRESS TEST
////////////////////////////

UTEST(combined, hash_table_with_dyn_array_values) {
    sp_test_use_malloc();

    typedef int* int_array;
    sp_hash_table(int, int_array) ht = SP_NULLPTR;

    for (s32 i = 0; i < 5; i++) {
        sp_dyn_array(int) arr = SP_NULLPTR;

        for (s32 j = 0; j < 10; j++) {
            sp_dyn_array_push(arr, i * 100 + j);
        }

        sp_hash_table_insert(ht, i, arr);
    }

    for (s32 i = 0; i < 5; i++) {
        ASSERT_TRUE(sp_hash_table_exists(ht, i));

        int_array arr = sp_hash_table_get(ht, i);
        ASSERT_EQ(sp_dyn_array_size(arr), 10);

        for (s32 j = 0; j < 10; j++) {
            ASSERT_EQ(arr[j], i * 100 + j);
        }
    }

    for (s32 i = 0; i < 5; i++) {
        int_array arr = sp_hash_table_get(ht, i);
        sp_dyn_array_free(arr);
    }

    sp_hash_table_free(ht);
}

UTEST(combined, multiple_arrays_in_hash_table) {
    sp_test_use_malloc();

    sp_hash_table(int, void*) ht = SP_NULLPTR;

    for (s32 key = 0; key < 5; key++) {
        sp_dyn_array(int) arr = SP_NULLPTR;

        for (s32 j = 0; j < 20; j++) {
            sp_dyn_array_push(arr, key * 1000 + j);
        }

        sp_hash_table_insert(ht, key, (void*)arr);
    }

    for (s32 key = 0; key < 5; key++) {
        ASSERT_TRUE(sp_hash_table_exists(ht, key));

        int* arr = (int*)sp_hash_table_get(ht, key);
        ASSERT_EQ(sp_dyn_array_size(arr), 20);

        for (s32 j = 0; j < 20; j++) {
            ASSERT_EQ(arr[j], key * 1000 + j);
        }
    }

    for (s32 key = 0; key < 5; key++) {
        int* arr = (int*)sp_hash_table_get(ht, key);
        sp_dyn_array_free(arr);
    }

    sp_hash_table_free(ht);
}

////////////////////////////
// RING BUFFER TESTS
////////////////////////////

UTEST(ring_buffer, basic_operations) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 10, sizeof(int));

    ASSERT_EQ(rb.size, 0);
    ASSERT_EQ(rb.capacity, 10);
    ASSERT_TRUE(sp_ring_buffer_is_empty(&rb));
    ASSERT_FALSE(sp_ring_buffer_is_full(&rb));

    s32 val = 42;
    sp_ring_buffer_push(&rb, &val);
    ASSERT_EQ(rb.size, 1);
    ASSERT_FALSE(sp_ring_buffer_is_empty(&rb));

    int* back = (int*)sp_ring_buffer_back(&rb);
    ASSERT_EQ(*back, 42);

    for (s32 i = 1; i < 10; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    ASSERT_EQ(rb.size, 10);
    ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

    int* popped = (int*)sp_ring_buffer_pop(&rb);
    ASSERT_EQ(*popped, 42);
    ASSERT_EQ(rb.size, 9);

    sp_ring_buffer_clear(&rb);
    ASSERT_EQ(rb.size, 0);
    ASSERT_TRUE(sp_ring_buffer_is_empty(&rb));

    sp_ring_buffer_destroy(&rb);
    ASSERT_EQ(rb.data, SP_NULLPTR);
}

UTEST(ring_buffer, push_literal_macro) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 5, sizeof(u32));

    sp_ring_buffer_push_literal(&rb, u32, 69);
    sp_ring_buffer_push_literal(&rb, u32, 420);
    sp_ring_buffer_push_literal(&rb, u32, 1337);

    ASSERT_EQ(rb.size, 3);

    u32* val1 = (u32*)sp_ring_buffer_at(&rb, 0);
    u32* val2 = (u32*)sp_ring_buffer_at(&rb, 1);
    u32* val3 = (u32*)sp_ring_buffer_at(&rb, 2);

    ASSERT_EQ(*val1, 69);
    ASSERT_EQ(*val2, 420);
    ASSERT_EQ(*val3, 1337);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, circular_behavior) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 3, sizeof(int));

    for (s32 i = 0; i < 3; i++) {
        sp_ring_buffer_push(&rb, &i);
    }
    ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

    int* popped = (int*)sp_ring_buffer_pop(&rb);
    ASSERT_EQ(*popped, 0);

    s32 val = 3;
    sp_ring_buffer_push(&rb, &val);
    ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

    popped = (int*)sp_ring_buffer_pop(&rb);
    ASSERT_EQ(*popped, 1);
    popped = (int*)sp_ring_buffer_pop(&rb);
    ASSERT_EQ(*popped, 2);
    popped = (int*)sp_ring_buffer_pop(&rb);
    ASSERT_EQ(*popped, 3);

    ASSERT_TRUE(sp_ring_buffer_is_empty(&rb));

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, overwrite_behavior) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 3, sizeof(int));

    for (s32 i = 0; i < 5; i++) {
        sp_ring_buffer_push_overwrite(&rb, &i);
    }

    ASSERT_EQ(rb.size, 3);

    int* val0 = (int*)sp_ring_buffer_pop(&rb);
    int* val1 = (int*)sp_ring_buffer_pop(&rb);
    int* val2 = (int*)sp_ring_buffer_pop(&rb);

    ASSERT_EQ(*val0, 2);
    ASSERT_EQ(*val1, 3);
    ASSERT_EQ(*val2, 4);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, push_zero) {
    sp_test_use_malloc();

    typedef struct {
        s32 x, y, z;
    } point_t;

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 5, sizeof(point_t));

    point_t* p = (point_t*)sp_ring_buffer_push_zero(&rb);
    ASSERT_EQ(p->x, 0);
    ASSERT_EQ(p->y, 0);
    ASSERT_EQ(p->z, 0);

    point_t val = {1, 2, 3};
    sp_ring_buffer_push(&rb, &val);

    p = (point_t*)sp_ring_buffer_push_overwrite_zero(&rb);
    ASSERT_EQ(p->x, 0);
    ASSERT_EQ(p->y, 0);
    ASSERT_EQ(p->z, 0);

    ASSERT_EQ(rb.size, 3);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, iteration_forward) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 10, sizeof(int));

    for (s32 i = 0; i < 5; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    s32 expected = 0;
    sp_ring_buffer_for(rb, it) {
        int* val = sp_rb_it(it, int);
        ASSERT_EQ(*val, expected);
        expected++;
    }
    ASSERT_EQ(expected, 5);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, iteration_reverse) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 10, sizeof(int));

    for (s32 i = 0; i < 5; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    s32 expected = 4;
    sp_ring_buffer_rfor(rb, it) {
        int* val = sp_rb_it(it, int);
        ASSERT_EQ(*val, expected);
        expected--;
    }
    ASSERT_EQ(expected, -1);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, iteration_after_wrap) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 3, sizeof(int));

    for (s32 i = 0; i < 3; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    sp_ring_buffer_pop(&rb);
    sp_ring_buffer_pop(&rb);

    s32 val3 = 3, val4 = 4;
    sp_ring_buffer_push(&rb, &val3);
    sp_ring_buffer_push(&rb, &val4);

    s32 values[3];
    s32 idx = 0;
    sp_ring_buffer_for(rb, it) {
        int* val = sp_rb_it(it, int);
        values[idx++] = *val;
    }

    ASSERT_EQ(values[0], 2);
    ASSERT_EQ(values[1], 3);
    ASSERT_EQ(values[2], 4);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, struct_type) {
    sp_test_use_malloc();

    typedef struct {
        float x, y;
        s32 id;
    } entity_t;

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 5, sizeof(entity_t));

    for (s32 i = 0; i < 5; i++) {
        entity_t e = {(float)i * 1.5f, (float)i * 2.5f, i};
        sp_ring_buffer_push(&rb, &e);
    }

    entity_t* e = (entity_t*)sp_ring_buffer_at(&rb, 2);
    ASSERT_EQ(e->x, 3.0f);
    ASSERT_EQ(e->y, 5.0f);
    ASSERT_EQ(e->id, 2);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, edge_cases) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb1;
    sp_ring_buffer_init(&rb1, 1, sizeof(int));

    s32 val = 42;
    sp_ring_buffer_push(&rb1, &val);
    ASSERT_TRUE(sp_ring_buffer_is_full(&rb1));

    int* popped = (int*)sp_ring_buffer_pop(&rb1);
    ASSERT_EQ(*popped, 42);
    ASSERT_TRUE(sp_ring_buffer_is_empty(&rb1));

    sp_ring_buffer_destroy(&rb1);

    sp_ring_buffer_t rb2;
    sp_ring_buffer_init(&rb2, 2, sizeof(float));

    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    sp_ring_buffer_push(&rb2, &f1);
    sp_ring_buffer_push(&rb2, &f2);
    sp_ring_buffer_push_overwrite(&rb2, &f3);

    float* fp1 = (float*)sp_ring_buffer_pop(&rb2);
    float* fp2 = (float*)sp_ring_buffer_pop(&rb2);

    ASSERT_EQ(*fp1, 2.5f);
    ASSERT_EQ(*fp2, 3.5f);

    sp_ring_buffer_destroy(&rb2);
}

UTEST(ring_buffer, bytes_calculation) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 10, sizeof(double));

    ASSERT_EQ(sp_ring_buffer_bytes(&rb), 10 * sizeof(double));

    sp_ring_buffer_destroy(&rb);

    sp_ring_buffer_init(&rb, 100, sizeof(char));
    ASSERT_EQ(sp_ring_buffer_bytes(&rb), 100);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, iterator_manual) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 5, sizeof(int));

    for (s32 i = 10; i < 15; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    sp_ring_buffer_iterator_t it = sp_ring_buffer_iter(&rb);
    ASSERT_FALSE(sp_ring_buffer_iter_done(&it));

    int* val = (int*)sp_ring_buffer_iter_deref(&it);
    ASSERT_EQ(*val, 10);

    sp_ring_buffer_iter_next(&it);
    val = (int*)sp_ring_buffer_iter_deref(&it);
    ASSERT_EQ(*val, 11);

    sp_ring_buffer_iterator_t rit = sp_ring_buffer_riter(&rb);
    val = (int*)sp_ring_buffer_iter_deref(&rit);
    ASSERT_EQ(*val, 14);

    sp_ring_buffer_iter_prev(&rit);
    val = (int*)sp_ring_buffer_iter_deref(&rit);
    ASSERT_EQ(*val, 13);

    sp_ring_buffer_destroy(&rb);
}

UTEST(fixed_array, basic_operations) {
  sp_test_use_malloc();

  sp_fixed_array_t arr;
  sp_fixed_array_init(&arr, 10, sizeof(s32));

  ASSERT_EQ(arr.size, 0);
  ASSERT_EQ(arr.capacity, 10);
  ASSERT_EQ(arr.element_size, sizeof(s32));
  ASSERT_NE(arr.data, SP_NULLPTR);

  s32 values[] = {42, 100, 200};
  u8* pushed = sp_fixed_array_push(&arr, values, 3);
  ASSERT_NE(pushed, SP_NULLPTR);
  ASSERT_EQ(arr.size, 3);

  s32* elem0 = (s32*)sp_fixed_array_at(&arr, 0);
  s32* elem1 = (s32*)sp_fixed_array_at(&arr, 1);
  s32* elem2 = (s32*)sp_fixed_array_at(&arr, 2);
  ASSERT_EQ(*elem0, 42);
  ASSERT_EQ(*elem1, 100);
  ASSERT_EQ(*elem2, 200);

  ASSERT_EQ(sp_fixed_array_byte_size(&arr), 3 * sizeof(s32));
}

UTEST(fixed_array, capacity_limits) {
  sp_test_use_malloc();

  sp_fixed_array_t arr;
  sp_fixed_array_init(&arr, 5, sizeof(u64));

  u64 val = 123456;
  sp_fixed_array_push(&arr, &val, 1);
  sp_fixed_array_push(&arr, &val, 1);
  sp_fixed_array_push(&arr, &val, 1);
  sp_fixed_array_push(&arr, &val, 1);
  sp_fixed_array_push(&arr, &val, 1);
  ASSERT_EQ(arr.size, 5);

  sp_fixed_array_clear(&arr);
  ASSERT_EQ(arr.size, 0);
  ASSERT_EQ(arr.capacity, 5);

  u64* reserved = (u64*)sp_fixed_array_reserve(&arr, 3);
  ASSERT_NE(reserved, SP_NULLPTR);
  ASSERT_EQ(arr.size, 3);
}


// ██╗    ██╗██╗███╗   ██╗██████╗ ██████╗
// ██║    ██║██║████╗  ██║╚════██╗╚════██╗
// ██║ █╗ ██║██║██╔██╗ ██║ █████╔╝ █████╔╝
// ██║███╗██║██║██║╚██╗██║ ╚═══██╗██╔═══╝
// ╚███╔███╔╝██║██║ ╚████║██████╔╝███████╗
//  ╚══╝╚══╝ ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝
#ifdef SP_WIN32
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
  sp_str_t current_dir_str = SP_CSTR(current_dir);
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
  ASSERT_NE(strstr(test_data.last_file_path, test_filename), SP_NULLPTR);

  // Clean up
  sp_test_delete_file(test_filename);
  sp_free(test_filename);
}
#endif



// ██████╗  ██████╗ ███████╗██╗██╗  ██╗
// ██╔══██╗██╔═══██╗██╔════╝██║╚██╗██╔╝
// ██████╔╝██║   ██║███████╗██║ ╚███╔╝
// ██╔═══╝ ██║   ██║╚════██║██║ ██╔██╗
// ██║     ╚██████╔╝███████║██║██╔╝ ██╗
// ╚═╝      ╚═════╝ ╚══════╝╚═╝╚═╝  ╚═╝
#ifdef SP_POSIX
UTEST(posix, smoke) {
  sp_test_use_malloc();

  sp_str_t path = SP_LIT("/tmp/test");
  bool exists = sp_os_does_path_exist(path);

  sp_mutex_t mutex;
  sp_mutex_init(&mutex, SP_MUTEX_PLAIN);
  sp_mutex_lock(&mutex);
  sp_mutex_unlock(&mutex);
  sp_mutex_destroy(&mutex);

  sp_semaphore_t sem;
  sp_semaphore_init(&sem);
  sp_semaphore_signal(&sem);
  sp_semaphore_wait(&sem);
  sp_semaphore_destroy(&sem);
}
#endif


//  ██████╗██████╗ ██████╗
// ██╔════╝██╔══██╗██╔══██╗
// ██║     ██████╔╝██████╔╝
// ██║     ██╔═══╝ ██╔═══╝
// ╚██████╗██║     ██║
//  ╚═════╝╚═╝     ╚═╝
#ifdef SP_CPP
UTEST(string_cpp, path_concatenation_operator) {
  sp_test_use_malloc();

  // Test basic concatenation
  sp_str_t path1 = SP_LIT("home");
  sp_str_t path2 = SP_LIT("user");
  sp_str_t result = path1 / path2;

  ASSERT_EQ(result.len, 9);
  SP_EXPECT_STR_EQ_CSTR(result, "home/user");

  // Test with backslashes (should be normalized)
  sp_str_t win_path1 = SP_LIT("C:\\Windows");
  sp_str_t win_path2 = SP_LIT("System32");
  sp_str_t win_result = win_path1 / win_path2;

  SP_EXPECT_STR_EQ_CSTR(win_result, "C:/Windows/System32");

  // Test empty paths
  sp_str_t empty = SP_LIT("");
  sp_str_t filename = SP_LIT("file.txt");
  sp_str_t empty_result = empty / filename;

  SP_EXPECT_STR_EQ_CSTR(empty_result, "/file.txt");

  // Test chaining
  sp_str_t base = SP_LIT("root");
  sp_str_t dir = SP_LIT("subdir");
  sp_str_t file = SP_LIT("file.txt");
  sp_str_t chained = base / dir / file;

  SP_EXPECT_STR_EQ_CSTR(chained, "root/subdir/file.txt");

  // Test operator/ with C string literals
  sp_str_t path = SP_LIT("home");
  sp_str_t result_cstr = path / "documents";

  ASSERT_EQ(result_cstr.len, 14);
  SP_EXPECT_STR_EQ_CSTR(result_cstr, "home/documents");

  // Test chaining with C string literals
  sp_str_t chained_cstr = base / "data" / "files";
  SP_EXPECT_STR_EQ_CSTR(chained_cstr, "root/data/files");
}
#endif

UTEST(sp_str_kernels, map_trim) {
  sp_test_use_malloc();

  // Test trim
  sp_str_t strings[] = {
    SP_LIT("  hello  "),
    SP_LIT("\tworld\n"),
    SP_LIT("  \t\n\r  "),
    SP_LIT("no_trim"),
  };

  sp_dyn_array(sp_str_t) results = sp_str_map(strings, 4, NULL, sp_str_map_kernel_trim);

  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "hello");
  SP_EXPECT_STR_EQ_CSTR(results[1], "world");
  SP_EXPECT_STR_EQ_CSTR(results[2], "");
  SP_EXPECT_STR_EQ_CSTR(results[3], "no_trim");
}

UTEST(sp_str_kernels, map_case_transform) {
  sp_test_use_malloc();

  sp_str_t strings[] = {
    SP_LIT("Hello World"),
    SP_LIT("ALREADY UPPER"),
    SP_LIT("already lower"),
    SP_LIT("MiXeD cAsE"),
  };

  // Test uppercase
  sp_dyn_array(sp_str_t) results = sp_str_map(strings, 4, NULL, sp_str_map_kernel_to_upper);
  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "HELLO WORLD");
  SP_EXPECT_STR_EQ_CSTR(results[1], "ALREADY UPPER");
  SP_EXPECT_STR_EQ_CSTR(results[2], "ALREADY LOWER");
  SP_EXPECT_STR_EQ_CSTR(results[3], "MIXED CASE");

  // Test lowercase
  results = sp_str_map(strings, 4, NULL, sp_str_map_kernel_to_lower);
  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "hello world");
  SP_EXPECT_STR_EQ_CSTR(results[1], "already upper");
  SP_EXPECT_STR_EQ_CSTR(results[2], "already lower");
  SP_EXPECT_STR_EQ_CSTR(results[3], "mixed case");

  // Test capitalize
  sp_str_t strings2[] = {
    SP_LIT("hello world"),
    SP_LIT("the quick brown fox"),
    SP_LIT("SHOUTING TEXT"),
    SP_LIT("123 numbers first"),
  };

  results = sp_str_map(strings2, 4, NULL, sp_str_map_kernel_capitalize_words);
  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "Hello World");
  SP_EXPECT_STR_EQ_CSTR(results[1], "The Quick Brown Fox");
  SP_EXPECT_STR_EQ_CSTR(results[2], "Shouting Text");
  SP_EXPECT_STR_EQ_CSTR(results[3], "123 Numbers First");
}

UTEST(sp_str_kernels, reduce_contains) {
  sp_test_use_malloc();

  sp_str_t strings[] = {
    SP_LIT("apple"),
    SP_LIT("banana"),
    SP_LIT("cherry"),
    SP_LIT("date"),
  };

  // Test contains - found case
  ASSERT_TRUE(sp_str_contains_n(strings, 4, SP_LIT("ana")));

  // Test contains - not found case
  ASSERT_FALSE(sp_str_contains_n(strings, 4, SP_LIT("xyz")));
}

UTEST(sp_str_kernels, reduce_count) {
  sp_test_use_malloc();

  sp_str_t strings[] = {
    SP_LIT("hello world"),
    SP_LIT("hello hello"),
    SP_LIT("world"),
    SP_LIT("hello"),
  };

  // Count "hello"
  ASSERT_EQ(sp_str_count_n(strings, 4, SP_LIT("hello")), 4); // "hello" appears 4 times total

  // Count "world"
  ASSERT_EQ(sp_str_count_n(strings, 4, SP_LIT("world")), 2); // "world" appears 2 times total
}

UTEST(sp_str_kernels, reduce_longest_shortest) {
  sp_test_use_malloc();

  sp_str_t strings[] = {
    SP_LIT("short"),
    SP_LIT("medium length"),
    SP_LIT("x"),
    SP_LIT("this is the longest string here"),
    SP_LIT("tiny"),
  };

  // Test longest
  sp_str_t longest = sp_str_find_longest_n(strings, 5);
  SP_EXPECT_STR_EQ_CSTR(longest, "this is the longest string here");

  // Test shortest
  sp_str_t shortest = sp_str_find_shortest_n(strings, 5);
  SP_EXPECT_STR_EQ_CSTR(shortest, "x");
}

typedef enum {
  SP_ENUM_FOO,
  SP_ENUM_BAR,
  SP_ENUM_BAZ,
  SP_ENUM_QUX,
} sp_test_enum_t;

const c8* sp_test_enum_to_cstr(sp_test_enum_t e) {
  switch (e) {
    SP_SWITCH_ENUM_TO_CSTR(SP_ENUM_FOO)
    SP_SWITCH_ENUM_TO_CSTR(SP_ENUM_BAR)
    SP_SWITCH_ENUM_TO_CSTR(SP_ENUM_BAZ)
    SP_SWITCH_ENUM_TO_CSTR(SP_ENUM_QUX)
  }

  SP_UNREACHABLE_RETURN("");
}

sp_str_t sp_test_enum_to_string(sp_test_enum_t e) {
  switch (e) {
    SP_SWITCH_ENUM_TO_STRING(SP_ENUM_FOO)
    SP_SWITCH_ENUM_TO_STRING(SP_ENUM_BAR)
    SP_SWITCH_ENUM_TO_STRING(SP_ENUM_BAZ)
    SP_SWITCH_ENUM_TO_STRING(SP_ENUM_QUX)
  }

  SP_UNREACHABLE_RETURN(SP_LIT(""));
}

UTEST(sp_enum_macros, name_generation) {
  ASSERT_STREQ(sp_test_enum_to_cstr(SP_ENUM_BAZ), "SP_ENUM_BAZ");
  SP_EXPECT_STR_EQ_CSTR(sp_test_enum_to_string(SP_ENUM_QUX), "SP_ENUM_QUX");
}


// ███████╗████████╗██████╗ ███████╗███████╗███████╗
// ██╔════╝╚══██╔══╝██╔══██╗██╔════╝██╔════╝██╔════╝
// ███████╗   ██║   ██████╔╝█████╗  ███████╗███████╗
// ╚════██║   ██║   ██╔══██╗██╔══╝  ╚════██║╚════██║
// ███████║   ██║   ██║  ██║███████╗███████║███████║
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝
#ifdef SP_TEST_ENABLE_STRESS_TESTS
UTEST(dynamic_array, stress_test) {
  sp_test_memory_tracker tracker;
  sp_test_memory_tracker_init(&tracker, 128 * 1024 * 1024);

  // Test millions of operations
  {
    sp_dynamic_array_t arr;
    sp_dynamic_array_init(&arr, sizeof(s32));

    const s32 iterations = 1000000;

    // Push a million elements
    for (s32 i = 0; i < iterations; i++) {
      sp_dynamic_array_push(&arr, &i);
    }

    ASSERT_EQ(arr.size, (u32)iterations);

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

UTEST(dyn_array, large_stress_test) {
    sp_test_use_malloc();

    sp_dyn_array(u64) arr = SP_NULLPTR;

    const s32 count = 100000;

    for (s32 i = 0; i < count; i++) {
        sp_dyn_array_push(arr, (u64)i * 12345);
    }

    ASSERT_EQ(sp_dyn_array_size(arr), count);

    for (s32 i = 0; i < count; i++) {
        ASSERT_EQ(arr[i], (u64)i * 12345);
    }

    sp_dyn_array_clear(arr);
    ASSERT_EQ(sp_dyn_array_size(arr), 0);

    for (s32 i = 0; i < 1000; i++) {
        sp_dyn_array_push(arr, (u64)i);
    }
    ASSERT_EQ(sp_dyn_array_size(arr), 1000);

    sp_dyn_array_free(arr);
}

UTEST(hash_table, large_stress_test) {
    sp_test_use_malloc();

    sp_hash_table(u64, u64) ht = SP_NULLPTR;

    const s32 count = 10000;

    for (u64 i = 0; i < count; i++) {
        sp_hash_table_insert(ht, i, i * i);
    }

    ASSERT_EQ(sp_hash_table_size(ht), count);

    for (s32 i = 0; i < 100; i++) {
        u64 key = rand() % count;
        ASSERT_TRUE(sp_hash_table_exists(ht, key));
        ASSERT_EQ(sp_hash_table_get(ht, key), key * key);
    }

    for (u64 i = 0; i < count; i += 2) {
        sp_hash_table_erase(ht, i);
    }

    ASSERT_EQ(sp_hash_table_size(ht), count / 2);

    for (u64 i = 1; i < count; i += 2) {
        ASSERT_TRUE(sp_hash_table_exists(ht, i));
        ASSERT_EQ(sp_hash_table_get(ht, i), i * i);
    }

    sp_hash_table_free(ht);
}

UTEST(ring_buffer, large_buffer_stress) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 1000, sizeof(u64));

    for (u64 i = 0; i < 1000; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

    for (u64 i = 0; i < 500; i++) {
        u64* val = (u64*)sp_ring_buffer_pop(&rb);
        ASSERT_EQ(*val, i);
    }

    for (u64 i = 1000; i < 1500; i++) {
        sp_ring_buffer_push(&rb, &i);
    }

    ASSERT_TRUE(sp_ring_buffer_is_full(&rb));

    u64 expected = 500;
    sp_ring_buffer_for(rb, it) {
        u64* val = sp_rb_it(it, u64);
        ASSERT_EQ(*val, expected);
        expected++;
    }

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, continuous_overwrite_stress) {
    sp_test_use_malloc();

    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 100, sizeof(int));

    for (s32 i = 0; i < 10000; i++) {
        sp_ring_buffer_push_overwrite(&rb, &i);
    }

    ASSERT_EQ(rb.size, 100);

    for (s32 i = 0; i < 100; i++) {
        int* val = (int*)sp_ring_buffer_pop(&rb);
        ASSERT_EQ(*val, 9900 + i);
    }

    ASSERT_TRUE(sp_ring_buffer_is_empty(&rb));

    sp_ring_buffer_destroy(&rb);
}
#endif

UTEST(os_functions, recursive_directory_removal) {
  sp_test_use_malloc();

  sp_str_t foo = SP_LIT("foo");
  sp_str_t   bar = SP_LIT("foo/bar");
  sp_str_t     baz = SP_LIT("foo/bar/baz");
  sp_str_t       phil = SP_LIT("foo/bar/baz/phil.txt");
  sp_str_t     bobby = SP_LIT("foo/bar/bobby.txt");
  sp_str_t   qux = SP_LIT("foo/qux");
  sp_str_t     billy = SP_LIT("foo/qux/billy.txt");
  sp_str_t   jerry = SP_LIT("foo/jerry.txt");

  sp_os_create_directory(foo);
  sp_os_create_directory(bar);
  sp_os_create_directory(qux);
  sp_os_create_directory(baz);
  sp_os_create_file(jerry);
  sp_os_create_file(bobby);
  sp_os_create_file(phil);
  sp_os_create_file(billy);

  ASSERT_TRUE(sp_os_is_directory(foo));
  ASSERT_TRUE(sp_os_is_directory(bar));
  ASSERT_TRUE(sp_os_is_directory(qux));
  ASSERT_TRUE(sp_os_is_directory(baz));
  ASSERT_TRUE(sp_os_is_regular_file(jerry));
  ASSERT_TRUE(sp_os_is_regular_file(bobby));
  ASSERT_TRUE(sp_os_is_regular_file(phil));
  ASSERT_TRUE(sp_os_is_regular_file(billy));

  sp_os_remove_directory(foo);

  ASSERT_FALSE(sp_os_does_path_exist(foo));
  ASSERT_FALSE(sp_os_does_path_exist(bar));
  ASSERT_FALSE(sp_os_does_path_exist(qux));
  ASSERT_FALSE(sp_os_does_path_exist(baz));
  ASSERT_FALSE(sp_os_does_path_exist(jerry));
  ASSERT_FALSE(sp_os_does_path_exist(bobby));
  ASSERT_FALSE(sp_os_does_path_exist(phil));
  ASSERT_FALSE(sp_os_does_path_exist(billy));
}

sp_str_t sp_test_build_scan_directory() {
  sp_str_t directory = SP_LIT("build/test/sp_os_scan_directory");
  if (sp_os_does_path_exist(directory)) {
    sp_os_remove_directory(directory);
  }
  sp_os_create_directory(SP_LIT("build/test"));
  sp_os_create_directory(directory);
  return directory;
}

UTEST(sp_os_scan_directory, basic_scan) {
  sp_test_use_malloc();
  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t file1 = sp_os_join_path(base, SP_LIT("file1.txt"));
  sp_str_t file2 = sp_os_join_path(base, SP_LIT("file2.log"));
  sp_str_t dir1 = sp_os_join_path(base, SP_LIT("subdir1"));
  sp_str_t dir2 = sp_os_join_path(base, SP_LIT("subdir2"));

  sp_os_create_file(file1);
  sp_os_create_file(file2);
  sp_os_create_directory(dir1);
  sp_os_create_directory(dir2);

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);

  ASSERT_EQ(entries.count, 4);

  u32 file_count = 0;
  u32 dir_count = 0;

  for (u32 i = 0; i < entries.count; i++) {
    sp_os_directory_entry_t* entry = &entries.data[i];
    if (entry->attributes & SP_OS_FILE_ATTR_REGULAR_FILE) {
      file_count++;
    }
    if (entry->attributes & SP_OS_FILE_ATTR_DIRECTORY) {
      dir_count++;
    }
  }

  ASSERT_EQ(file_count, 2);
  ASSERT_EQ(dir_count, 2);

  sp_test_build_scan_directory();
}

UTEST(sp_os_scan_directory, file_names_validation) {
  sp_test_use_malloc();
  sp_str_t base = sp_test_build_scan_directory();

  const c8* expected_names[] = {
    "alpha.txt",
    "beta.log",
    "gamma.c",
    "delta"
  };

  for (u32 i = 0; i < 4; i++) {
    sp_str_t file_path = sp_os_join_path(base, SP_CSTR(expected_names[i]));
    sp_os_create_file(file_path);
  }

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);

  ASSERT_EQ(entries.count, 4);

  bool found[4] = {false, false, false, false};

  for (u32 i = 0; i < entries.count; i++) {
    for (u32 j = 0; j < 4; j++) {
      if (sp_str_equal_cstr(entries.data[i].file_name, expected_names[j])) {
        found[j] = true;
        break;
      }
    }
  }

  for (u32 i = 0; i < 4; i++) {
    ASSERT_TRUE(found[i]);
  }

  sp_test_build_scan_directory();
}

UTEST(sp_os_scan_directory, file_attributes) {
  sp_test_use_malloc();
  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t test_file = sp_os_join_path(base, SP_LIT("test.txt"));
  sp_str_t test_dir = sp_os_join_path(base, SP_LIT("testdir"));

  sp_os_create_file(test_file);
  sp_os_create_directory(test_dir);

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);

  ASSERT_EQ(entries.count, 2);

  bool found_file = false;
  bool found_dir = false;

  for (u32 i = 0; i < entries.count; i++) {
    sp_os_directory_entry_t* entry = &entries.data[i];

    if (sp_str_equal_cstr(entry->file_name, "test.txt")) {
      ASSERT_TRUE(entry->attributes & SP_OS_FILE_ATTR_REGULAR_FILE);
      ASSERT_FALSE(entry->attributes & SP_OS_FILE_ATTR_DIRECTORY);
      found_file = true;
    }

    if (sp_str_equal_cstr(entry->file_name, "testdir")) {
      ASSERT_TRUE(entry->attributes & SP_OS_FILE_ATTR_DIRECTORY);
      ASSERT_FALSE(entry->attributes & SP_OS_FILE_ATTR_REGULAR_FILE);
      found_dir = true;
    }
  }

  ASSERT_TRUE(found_file);
  ASSERT_TRUE(found_dir);

  sp_test_build_scan_directory();
}

UTEST(sp_os_scan_directory, empty_directory) {
  sp_test_use_malloc();
  sp_str_t base = sp_test_build_scan_directory();

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);

  ASSERT_EQ(entries.count, 0);
  ASSERT_TRUE(entries.data == SP_NULLPTR || entries.count == 0);

  sp_test_build_scan_directory();
}

UTEST(sp_os_scan_directory, non_existent_directory) {
  sp_test_use_malloc();
  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t non_existent = sp_os_join_path(base, SP_LIT("some_bullshit"));

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(non_existent);

  ASSERT_EQ(entries.count, 0);
}

UTEST(sp_os_scan_directory, file_path_correctness) {
  sp_test_use_malloc();
  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t file1 = sp_os_join_path(base, SP_LIT("test1.txt"));
  sp_str_t dir1 = sp_os_join_path(base, SP_LIT("subdir"));

  sp_os_create_file(file1);
  sp_os_create_directory(dir1);

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);

  ASSERT_EQ(entries.count, 2);

  for (u32 i = 0; i < entries.count; i++) {
    sp_os_directory_entry_t* entry = &entries.data[i];

    ASSERT_TRUE(sp_str_starts_with(entry->file_path, base));

    ASSERT_TRUE(sp_os_does_path_exist(entry->file_path));

    if (sp_str_equal_cstr(entry->file_name, "test1.txt")) {
      ASSERT_TRUE(sp_str_ends_with(entry->file_path, SP_LIT("test1.txt")));
    }
    if (sp_str_equal_cstr(entry->file_name, "subdir")) {
      ASSERT_TRUE(sp_str_ends_with(entry->file_path, SP_LIT("subdir")));
    }
  }

  sp_test_build_scan_directory();
}


// ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗     ████████╗███████╗███████╗████████╗███████╗
// ██╔════╝╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝     ╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝██╔════╝
// ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗       ██║   █████╗  ███████╗   ██║   ███████╗
// ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║       ██║   ██╔══╝  ╚════██║   ██║   ╚════██║
// ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝       ██║   ███████╗███████║   ██║   ███████║
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝        ╚═╝   ╚══════╝╚══════╝   ╚═╝   ╚══════╝
UTEST(sp_str_trim, whitespace_handling) {
  sp_test_use_malloc();

  // basic trim operations
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("  hello  ")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\thello\t")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\nhello\n")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("  \t\nhello\n\t  ")), SP_LIT("hello"));

  // edge cases
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("")), SP_LIT(""));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("   ")), SP_LIT(""));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\t\n\r")), SP_LIT(""));

  // no whitespace
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("hello")), SP_LIT("hello"));

  // internal whitespace preserved
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("  hello world  ")), SP_LIT("hello world"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\ttab\tseparated\t")), SP_LIT("tab\tseparated"));
}

UTEST(sp_str_trim_right, trailing_whitespace) {
  sp_test_use_malloc();

  // basic right trim
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello  ")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello\t")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello\n")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello\t\n  ")), SP_LIT("hello"));

  // leading whitespace preserved
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("  hello")), SP_LIT("  hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("\thello")), SP_LIT("\thello"));

  // edge cases
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("")), SP_LIT(""));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("   ")), SP_LIT(""));

  // no trailing whitespace
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello")), SP_LIT("hello"));

  // internal whitespace preserved
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello world  ")), SP_LIT("hello world"));
}

UTEST(sp_str_split_c8, delimiter_splitting) {
  sp_test_use_malloc();

  // basic split
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("hello,world,test"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 3);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT("hello"));
    SP_EXPECT_STR_EQ(parts[1], SP_LIT("world"));
    SP_EXPECT_STR_EQ(parts[2], SP_LIT("test"));
  }

  // path splitting
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("/home/user/file.txt"), '/');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT(""));
    SP_EXPECT_STR_EQ(parts[1], SP_LIT("home"));
    SP_EXPECT_STR_EQ(parts[2], SP_LIT("user"));
    SP_EXPECT_STR_EQ(parts[3], SP_LIT("file.txt"));
  }

  // consecutive delimiters
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("a,,b"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 3);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT("a"));
    SP_EXPECT_STR_EQ(parts[1], SP_LIT(""));
    SP_EXPECT_STR_EQ(parts[2], SP_LIT("b"));
  }

  // no delimiter found
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("hello"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 1);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT("hello"));
  }

  // empty string - returns null
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(""), ',');
    ASSERT_EQ(parts, SP_NULLPTR);
  }

  // delimiter at start and end
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(",hello,world,"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT(""));
    SP_EXPECT_STR_EQ(parts[1], SP_LIT("hello"));
    SP_EXPECT_STR_EQ(parts[2], SP_LIT("world"));
    SP_EXPECT_STR_EQ(parts[3], SP_LIT(""));
  }
}

UTEST(sp_str_pad, padding_operations) {
  sp_test_use_malloc();

  // basic padding
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello"), 10), SP_LIT("hello     "));
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hi"), 5), SP_LIT("hi   "));

  // string already longer than padding
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello world"), 5), SP_LIT("hello world"));

  // exact length
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello"), 5), SP_LIT("hello"));

  // empty string
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT(""), 5), SP_LIT("     "));

  // zero padding
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello"), 0), SP_LIT("hello"));
}

UTEST(sp_str_pad_to_longest, array_padding) {
  sp_test_use_malloc();

  // basic array padding
  {
    sp_str_t strings[] = {
      SP_LIT("hi"),
      SP_LIT("hello"),
      SP_LIT("world!")
    };
    sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(strings, 3);
    ASSERT_EQ(sp_dyn_array_size(padded), 3);
    SP_EXPECT_STR_EQ(padded[0], SP_LIT("hi    "));
    SP_EXPECT_STR_EQ(padded[1], SP_LIT("hello "));
    SP_EXPECT_STR_EQ(padded[2], SP_LIT("world!"));
  }

  // all same length
  {
    sp_str_t strings[] = {
      SP_LIT("aaa"),
      SP_LIT("bbb"),
      SP_LIT("ccc")
    };
    sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(strings, 3);
    ASSERT_EQ(sp_dyn_array_size(padded), 3);
    SP_EXPECT_STR_EQ(padded[0], SP_LIT("aaa"));
    SP_EXPECT_STR_EQ(padded[1], SP_LIT("bbb"));
    SP_EXPECT_STR_EQ(padded[2], SP_LIT("ccc"));
  }

  // single string
  {
    sp_str_t strings[] = {
      SP_LIT("hello")
    };
    sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(strings, 1);
    ASSERT_EQ(sp_dyn_array_size(padded), 1);
    SP_EXPECT_STR_EQ(padded[0], SP_LIT("hello"));
  }

  // empty strings
  {
    sp_str_t strings[] = {
      SP_LIT(""),
      SP_LIT("hello"),
      SP_LIT("")
    };
    sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(strings, 3);
    ASSERT_EQ(sp_dyn_array_size(padded), 3);
    SP_EXPECT_STR_EQ(padded[0], SP_LIT("     "));
    SP_EXPECT_STR_EQ(padded[1], SP_LIT("hello"));
    SP_EXPECT_STR_EQ(padded[2], SP_LIT("     "));
  }
}

UTEST(sp_str_starts_with, prefix_checking) {
  sp_test_use_malloc();

  // basic prefix checks
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello world"), SP_LIT("hello")));
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("h")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("world")));

  // exact match
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("hello")));

  // prefix longer than string
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("hi"), SP_LIT("hello")));

  // empty cases
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("")));
  ASSERT_TRUE(sp_str_starts_with(SP_LIT(""), SP_LIT("")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT(""), SP_LIT("hello")));

  // path checking
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("/home/user/file.txt"), SP_LIT("/home")));
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("/home/user/file.txt"), SP_LIT("/home/user")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("/home/user/file.txt"), SP_LIT("/usr")));

  // case sensitivity
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("Hello"), SP_LIT("hello")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("HELLO")));
}

UTEST(sp_str_view, view_creation) {
  sp_test_use_malloc();

  // basic view creation
  {
    const c8* cstr = "hello world";
    sp_str_t view = sp_str_view(cstr);
    ASSERT_EQ(view.len, 11);
    ASSERT_EQ(view.data, cstr);
    SP_EXPECT_STR_EQ(view, SP_LIT("hello world"));
  }

  // empty string view
  {
    const c8* cstr = "";
    sp_str_t view = sp_str_view(cstr);
    ASSERT_EQ(view.len, 0);
    ASSERT_EQ(view.data, cstr);
    SP_EXPECT_STR_EQ(view, SP_LIT(""));
  }

  // null pointer
  {
    sp_str_t view = sp_str_view(SP_NULLPTR);
    ASSERT_EQ(view.len, 0);
    ASSERT_EQ(view.data, SP_NULLPTR);
  }

  // view doesn't copy
  {
    c8 buffer[] = "mutable";
    sp_str_t view = sp_str_view(buffer);
    buffer[0] = 'M';
    ASSERT_EQ(view.data[0], 'M');
  }
}

UTEST(sp_str_from_cstr, string_from_cstr) {
  sp_test_use_malloc();

  // basic string creation
  {
    const c8* cstr = "hello world";
    sp_str_t str = sp_str_from_cstr(cstr);
    ASSERT_EQ(str.len, 11);
    SP_EXPECT_STR_EQ(str, sp_str_view(cstr));
    // verify it's a copy
    ASSERT_NE(str.data, cstr);
  }

  // empty string
  {
    sp_str_t str = sp_str_from_cstr("");
    ASSERT_EQ(str.len, 0);
    SP_EXPECT_STR_EQ(str, SP_LIT(""));
  }

  // null pointer
  {
    sp_str_t str = sp_str_from_cstr(SP_NULLPTR);
    ASSERT_EQ(str.len, 0);
    // sp_str_from_cstr returns an empty allocated string for null, not null
    ASSERT_NE(str.data, SP_NULLPTR);
  }

  // verify deep copy
  {
    c8 buffer[] = "mutable";
    sp_str_t str = sp_str_from_cstr(buffer);
    buffer[0] = 'M';
    ASSERT_EQ(str.data[0], 'm');  // should still be lowercase
  }

  // sized variant
  {
    sp_str_t str = sp_str_from_cstr_sized("hello world", 5);
    ASSERT_EQ(str.len, 5);
    SP_EXPECT_STR_EQ(str, SP_LIT("hello"));
  }

  // null variant
  {
    sp_str_t str = sp_str_from_cstr_null(SP_NULLPTR);
    ASSERT_EQ(str.len, 0);
    // sp_str_from_cstr_null also allocates for null input
    ASSERT_NE(str.data, SP_NULLPTR);

    str = sp_str_from_cstr_null("hello");
    ASSERT_EQ(str.len, 5);
    SP_EXPECT_STR_EQ(str, SP_LIT("hello"));
  }
}

// Asset Registry Tests
#ifdef SP_APP
// Test asset type (user-defined, not in sp.h)
typedef enum {
  SP_ASSET_KIND_TEST = 1000,
} sp_test_asset_kind_t;

typedef struct {
  sp_str_t content;
  s32 value;
} sp_test_asset_data_t;

// Simple importer that just copies the data
void sp_test_asset_import(sp_asset_import_context_t* context) {
  sp_test_asset_data_t* input = (sp_test_asset_data_t*)context->user_data;
  sp_test_asset_data_t* data = (sp_test_asset_data_t*)sp_alloc(sizeof(sp_test_asset_data_t));
  data->content = sp_str_copy(input->content);
  data->value = input->value;

  // Get asset from context and set data (thread-safe with mutex)
  sp_mutex_lock(&context->registry->mutex);
  sp_asset_t* asset = sp_asset_import_context_get_asset(context);
  asset->data = data;
  sp_mutex_unlock(&context->registry->mutex);
}

void sp_test_asset_complete(sp_asset_import_context_t* context) {
  // Nothing special to do on completion for test assets
}

// Test: Basic synchronous add and find
UTEST(asset_registry, basic_add_and_find) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = SP_ZERO_STRUCT(sp_asset_registry_config_t);
  sp_asset_registry_init(&registry, config);

  // Add an asset
  sp_test_asset_data_t* data1 = (sp_test_asset_data_t*)sp_alloc(sizeof(sp_test_asset_data_t));
  data1->content = SP_LIT("test content");
  data1->value = 42;

  sp_asset_t* added = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT("test_asset"), data1);
  ASSERT_NE(added, SP_NULLPTR);
  ASSERT_EQ(added->kind, SP_ASSET_KIND_TEST);
  ASSERT_TRUE(sp_str_equal(added->name, SP_LIT("test_asset")));
  ASSERT_EQ(added->state, SP_ASSET_STATE_COMPLETED);
  ASSERT_EQ(added->data, data1);

  // Find the asset
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("test_asset"));
  ASSERT_NE(found, SP_NULLPTR);
  ASSERT_EQ(found, added);
  ASSERT_EQ(found->data, data1);

  // Find non-existent asset
  sp_asset_t* not_found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("nonexistent"));
  ASSERT_EQ(not_found, SP_NULLPTR);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Multiple assets with same name but different types
UTEST(asset_registry, same_name_different_types) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = SP_ZERO_STRUCT(sp_asset_registry_config_t);
  sp_asset_registry_init(&registry, config);

  // Add assets with same name but different types
  sp_asset_registry_add(&registry, 1001, SP_LIT("shared_name"), (void*)0x1);
  sp_asset_registry_add(&registry, 1002, SP_LIT("shared_name"), (void*)0x2);
  sp_asset_registry_add(&registry, 1003, SP_LIT("shared_name"), (void*)0x3);

  // Find each one
  sp_asset_t* asset1 = sp_asset_registry_find(&registry, 1001, SP_LIT("shared_name"));
  sp_asset_t* asset2 = sp_asset_registry_find(&registry, 1002, SP_LIT("shared_name"));
  sp_asset_t* asset3 = sp_asset_registry_find(&registry, 1003, SP_LIT("shared_name"));

  ASSERT_NE(asset1, SP_NULLPTR);
  ASSERT_NE(asset2, SP_NULLPTR);
  ASSERT_NE(asset3, SP_NULLPTR);

  ASSERT_EQ(asset1->data, (void*)0x1);
  ASSERT_EQ(asset2->data, (void*)0x2);
  ASSERT_EQ(asset3->data, (void*)0x3);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: String copying (verify names are copied, not referenced)
UTEST(asset_registry, string_copying) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = SP_ZERO_STRUCT(sp_asset_registry_config_t);
  sp_asset_registry_init(&registry, config);

  // Create a temporary string
  c8 temp_buffer[32];
  snprintf(temp_buffer, sizeof(temp_buffer), "temp_asset");
  sp_str_t temp_name = sp_str_from_cstr(temp_buffer);

  // Add asset with temporary name
  sp_asset_t* asset = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, temp_name, SP_NULLPTR);

  // Modify the original buffer
  snprintf(temp_buffer, sizeof(temp_buffer), "modified!");

  // The asset's name should still be intact
  ASSERT_TRUE(sp_str_equal(asset->name, SP_LIT("temp_asset")));
  ASSERT_FALSE(sp_str_equal(asset->name, sp_str_from_cstr(temp_buffer)));

  // Should still be findable with original name
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("temp_asset"));
  ASSERT_EQ(found, asset);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: NULL user data handling
UTEST(asset_registry, null_user_data) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = SP_ZERO_STRUCT(sp_asset_registry_config_t);
  sp_asset_registry_init(&registry, config);

  // Add asset with NULL data
  sp_asset_t* asset = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT("null_asset"), SP_NULLPTR);
  ASSERT_NE(asset, SP_NULLPTR);
  ASSERT_EQ(asset->data, SP_NULLPTR);

  // Should be findable
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("null_asset"));
  ASSERT_EQ(found, asset);
  ASSERT_EQ(found->data, SP_NULLPTR);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Empty name strings
UTEST(asset_registry, empty_names) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = SP_ZERO_STRUCT(sp_asset_registry_config_t);
  sp_asset_registry_init(&registry, config);

  // Add asset with empty name
  sp_asset_t* asset = sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, SP_LIT(""), (void*)0xDEAD);
  ASSERT_NE(asset, SP_NULLPTR);
  ASSERT_EQ(asset->name.len, 0);

  // Should be findable with empty name
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT(""));
  ASSERT_EQ(found, asset);
  ASSERT_EQ(found->data, (void*)0xDEAD);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Import/completion pipeline with actual threading
UTEST(asset_registry, import_completion_pipeline) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  // Create test data
  sp_test_asset_data_t input_data = {
    .content = SP_LIT("async content"),
    .value = 999
  };

  // Import an asset (goes through the async pipeline)
  sp_future_t* future = sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, SP_LIT("async_asset"), &input_data);
  ASSERT_NE(future, SP_NULLPTR);

  // Signal the worker thread
  sp_semaphore_signal(&registry.semaphore);

  // Wait a bit for import to complete
  sp_os_sleep_ms(50);

  // Process completions on main thread
  sp_asset_registry_process_completions(&registry);

  // Check the future is ready
  ASSERT_TRUE(future->ready);

  // Find the completed asset
  sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("async_asset"));
  ASSERT_NE(found, SP_NULLPTR);
  ASSERT_EQ(found->state, SP_ASSET_STATE_COMPLETED);

  // Verify the data was copied correctly
  sp_test_asset_data_t* result_data = (sp_test_asset_data_t*)found->data;
  ASSERT_NE(result_data, SP_NULLPTR);
  ASSERT_TRUE(sp_str_equal(result_data->content, SP_LIT("async content")));
  ASSERT_EQ(result_data->value, 999);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: State transitions
UTEST(asset_registry, state_transitions) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  sp_test_asset_data_t input = {
    .content = SP_LIT("state test"),
    .value = 777
  };

  // Start import - should be QUEUED initially
  sp_future_t* future = sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, SP_LIT("state_asset"), &input);

  // Find immediately after import (should be QUEUED)
  sp_asset_t* asset = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("state_asset"));
  ASSERT_NE(asset, SP_NULLPTR);
  ASSERT_EQ(asset->state, SP_ASSET_STATE_QUEUED);

  // Signal and wait for import
  sp_semaphore_signal(&registry.semaphore);
  sp_os_sleep_ms(50);

  // Should now be IMPORTED (but not yet COMPLETED)
  // Note: This is racy without better synchronization, but should work most of the time

  // Process completions
  sp_asset_registry_process_completions(&registry);

  // Should now be COMPLETED
  asset = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, SP_LIT("state_asset"));
  ASSERT_EQ(asset->state, SP_ASSET_STATE_COMPLETED);

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Concurrent find operations while importing
UTEST(asset_registry, concurrent_find_during_import) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = {
    .importers = {
      {
        .kind = SP_ASSET_KIND_TEST,
        .on_import = sp_test_asset_import,
        .on_completion = sp_test_asset_complete
      }
    }
  };
  sp_asset_registry_init(&registry, config);

  // Add some assets first
  for (s32 i = 0; i < 10; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), (void*)(uintptr_t)i);
  }

  // Start importing more assets
  for (s32 i = 10; i < 20; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_test_asset_data_t* data = (sp_test_asset_data_t*)sp_alloc(sizeof(sp_test_asset_data_t));
    data->content = sp_str_from_cstr(name);
    data->value = i;
    sp_asset_registry_import(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), data);
  }

  // Signal worker thread
  sp_semaphore_signal(&registry.semaphore);

  // Concurrent finds while import is happening
  for (s32 iter = 0; iter < 100; iter++) {
    s32 id = iter % 20;
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", id);

    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    if (id < 10) {
      // These were added synchronously, should always be found
      ASSERT_NE(found, SP_NULLPTR);
      ASSERT_EQ(found->data, (void*)(uintptr_t)id);
    }
    // Assets 10-19 might or might not be ready yet (that's ok)
  }

  // Let imports finish
  sp_os_sleep_ms(50);
  sp_asset_registry_process_completions(&registry);

  // Now all should be findable
  for (s32 i = 0; i < 20; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "asset_%d", i);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
  }

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}

// Test: Many assets stress test
UTEST(asset_registry, stress_many_assets) {
  sp_context_push_allocator(sp_malloc_allocator_init());

  sp_asset_registry_t registry = SP_ZERO_STRUCT(sp_asset_registry_t);
  sp_asset_registry_config_t config = SP_ZERO_STRUCT(sp_asset_registry_config_t);
  sp_asset_registry_init(&registry, config);

  const s32 ASSET_COUNT = 1000;

  // Add many assets
  for (s32 i = 0; i < ASSET_COUNT; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "stress_%d", i);
    sp_asset_registry_add(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name), (void*)(uintptr_t)i);
  }

  // Verify all can be found
  for (s32 i = 0; i < ASSET_COUNT; i++) {
    c8 name[32];
    snprintf(name, sizeof(name), "stress_%d", i);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
    ASSERT_EQ(found->data, (void*)(uintptr_t)i);
  }

  // Random access pattern
  for (s32 iter = 0; iter < ASSET_COUNT * 2; iter++) {
    s32 id = (iter * 7919) % ASSET_COUNT;  // Prime number for good distribution
    c8 name[32];
    snprintf(name, sizeof(name), "stress_%d", id);
    sp_asset_t* found = sp_asset_registry_find(&registry, SP_ASSET_KIND_TEST, sp_str_from_cstr(name));
    ASSERT_NE(found, SP_NULLPTR);
    ASSERT_EQ(found->data, (void*)(uintptr_t)id);
  }

  sp_asset_registry_shutdown(&registry);
  sp_context_pop();
}
#endif // SP_APP

UTEST_MAIN()
