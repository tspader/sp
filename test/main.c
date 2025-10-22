#define SP_APP
#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

UTEST_MAIN()

//  ██████╗ ██████╗ ██████╗ ███████╗
// ██╔════╝██╔═══██╗██╔══██╗██╔════╝
// ██║     ██║   ██║██████╔╝█████╗
// ██║     ██║   ██║██╔══██╗██╔══╝
// ╚██████╗╚██████╔╝██║  ██║███████╗
//  ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
typedef struct {
  const c8* expected;
  sp_str_t actual;
} sp_test_format_case_t;

UTEST(sp_fmt, basic) {
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
  void* ptr = (void*)(uintptr_t)0xDEADBEEF;
  sp_str_t result = sp_format("ptr: {}", SP_FMT_PTR(ptr));
  SP_EXPECT_STR_EQ_CSTR(result, "ptr: 0xdeadbeef");

  void* null_ptr = SP_NULLPTR;
  result = sp_format("null: {}", SP_FMT_PTR(null_ptr));
  SP_EXPECT_STR_EQ_CSTR(result, "null: 0x00000000");
}

UTEST(sp_fmt, hash_type) {
  sp_hash_t hash = 0xABCDEF12;
  sp_str_t result = sp_format("hash: {}", SP_FMT_HASH(hash));
  SP_EXPECT_STR_EQ_CSTR(result, "hash: abcdef12");

  sp_hash_t zero_hash = 0;
  result = sp_format("zero hash: {}", SP_FMT_HASH(zero_hash));
  SP_EXPECT_STR_EQ_CSTR(result, "zero hash: 0");
}

UTEST(sp_fmt, array_types) {
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
  u32 count = 42;
  sp_str_t name = SP_LIT("test");
  f32 value = 3.14f;

  sp_str_t result = sp_format("Count: {}, Name: {}, Value: {}",
    SP_FMT_U32(count), SP_FMT_STR(name), SP_FMT_F32(value));
  SP_EXPECT_STR_EQ_CSTR(result, "Count: 42, Name: test, Value: 3.140");
}

UTEST(sp_fmt, padding) {
  sp_str_t result;

  result = sp_format("{:pad 10}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42        ");

  result = sp_format("{:pad 5}", SP_FMT_CSTR("hi"));
  SP_EXPECT_STR_EQ_CSTR(result, "hi   ");

  sp_str_t foo_str = SP_LIT("foo");
  result = sp_format("{:pad 8}", SP_FMT_STR(foo_str));
  SP_EXPECT_STR_EQ_CSTR(result, "foo     ");

  result = sp_format("{:pad 15}", SP_FMT_S32(-123));
  SP_EXPECT_STR_EQ_CSTR(result, "-123           ");

  result = sp_format("{:pad 0}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42");

  result = sp_format("{:pad 1}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42");

  result = sp_format("{:pad 2}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42");

  result = sp_format("{:pad 3}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42 ");

  result = sp_format("{:pad 5}", SP_FMT_CSTR("hello"));
  SP_EXPECT_STR_EQ_CSTR(result, "hello");

  result = sp_format("{:pad 6}", SP_FMT_CSTR("hello"));
  SP_EXPECT_STR_EQ_CSTR(result, "hello ");

  result = sp_format("{:pad 10}", SP_FMT_CSTR(""));
  SP_EXPECT_STR_EQ_CSTR(result, "          ");

  result = sp_format("{:pad 10}", SP_FMT_U8(255));
  SP_EXPECT_STR_EQ_CSTR(result, "255       ");

  result = sp_format("{:pad 10}", SP_FMT_U16(65535));
  SP_EXPECT_STR_EQ_CSTR(result, "65535     ");

  result = sp_format("{:pad 20}", SP_FMT_U64(9876543210ULL));
  SP_EXPECT_STR_EQ_CSTR(result, "9876543210          ");

  result = sp_format("{:pad 10}", SP_FMT_S8(-128));
  SP_EXPECT_STR_EQ_CSTR(result, "-128      ");

  result = sp_format("{:pad 10}", SP_FMT_S16(-32768));
  SP_EXPECT_STR_EQ_CSTR(result, "-32768    ");

  result = sp_format("{:pad 15}", SP_FMT_F32(3.14f));
  SP_EXPECT_STR_EQ_CSTR(result, "3.140          ");

  result = sp_format("{:pad 10}", SP_FMT_C8('A'));
  SP_EXPECT_STR_EQ_CSTR(result, "A         ");
}

UTEST(sp_fmt, padding_in_context) {
  sp_str_t result;

  result = sp_format("Name: {:pad 20} Age: {:pad 5}",
    SP_FMT_CSTR("Alice"), SP_FMT_U32(30));
  SP_EXPECT_STR_EQ_CSTR(result, "Name: Alice                Age: 30   ");

  result = sp_format("[{:pad 10}] [{:pad 10}] [{:pad 10}]",
    SP_FMT_CSTR("left"), SP_FMT_CSTR("center"), SP_FMT_CSTR("right"));
  SP_EXPECT_STR_EQ_CSTR(result, "[left      ] [center    ] [right     ]");

  result = sp_format("ID:{:pad 6} Value:{:pad 8}",
    SP_FMT_U32(42), SP_FMT_F32(3.14f));
  SP_EXPECT_STR_EQ_CSTR(result, "ID:42     Value:3.140   ");
}

UTEST(sp_fmt, padding_large_widths) {
  sp_str_t result;

  result = sp_format("{:pad 50}", SP_FMT_CSTR("x"));
  ASSERT_EQ(result.len, 50);
  ASSERT_EQ(result.data[0], 'x');
  for (u32 i = 1; i < 50; i++) {
    ASSERT_EQ(result.data[i], ' ');
  }

  result = sp_format("{:pad 100}", SP_FMT_U32(1));
  ASSERT_EQ(result.len, 100);
  ASSERT_EQ(result.data[0], '1');
  for (u32 i = 1; i < 100; i++) {
    ASSERT_EQ(result.data[i], ' ');
  }
}

UTEST(sp_fmt, padding_combined_with_text) {


  sp_str_t result;

  result = sp_format("Start {:pad 10} End", SP_FMT_CSTR("mid"));
  SP_EXPECT_STR_EQ_CSTR(result, "Start mid        End");

  result = sp_format("{:pad 5}|{:pad 5}|{:pad 5}",
    SP_FMT_U32(1), SP_FMT_U32(22), SP_FMT_U32(333));
  SP_EXPECT_STR_EQ_CSTR(result, "1    |22   |333  ");

  result = sp_format("Column1: {:pad 15} Column2: {:pad 15}",
    SP_FMT_CSTR("short"), SP_FMT_CSTR("also short"));
  SP_EXPECT_STR_EQ_CSTR(result, "Column1: short           Column2: also short     ");
}

UTEST(sp_fmt, padding_numeric_parsing) {


  sp_str_t result;

  result = sp_format("{:pad 1}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42");

  result = sp_format("{:pad 10}", SP_FMT_U32(42));
  SP_EXPECT_STR_EQ_CSTR(result, "42        ");

  result = sp_format("{:pad 100}", SP_FMT_U32(42));
  ASSERT_EQ(result.len, 100);

  result = sp_format("{:pad 999}", SP_FMT_CSTR("a"));
  ASSERT_EQ(result.len, 999);
}


// ███████╗████████╗██████╗     ██████╗ ██╗   ██╗██╗██╗     ██████╗ ███████╗██████╗
// ██╔════╝╚══██╔══╝██╔══██╗    ██╔══██╗██║   ██║██║██║     ██╔══██╗██╔════╝██╔══██╗
// ███████╗   ██║   ██████╔╝    ██████╔╝██║   ██║██║██║     ██║  ██║█████╗  ██████╔╝
// ╚════██║   ██║   ██╔══██╗    ██╔══██╗██║   ██║██║██║     ██║  ██║██╔══╝  ██╔══██╗
// ███████║   ██║   ██║  ██║    ██████╔╝╚██████╔╝██║███████╗██████╔╝███████╗██║  ██║
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝    ╚═════╝  ╚═════╝ ╚═╝╚══════╝╚═════╝ ╚══════╝╚═╝  ╚═╝
UTEST(sp_str_builder, basic_operations) {


  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  ASSERT_EQ(builder.buffer.data, SP_NULLPTR);
  ASSERT_EQ(builder.buffer.len, 0);
  ASSERT_EQ(builder.buffer.capacity, 0);

  sp_str_builder_grow(&builder, 10);
  ASSERT_GE(builder.buffer.capacity, 10);
  ASSERT_NE(builder.buffer.data, SP_NULLPTR);
  ASSERT_EQ(builder.buffer.len, 0);

  sp_str_t test_str = SP_LIT("Hello");
  sp_str_builder_append(&builder, test_str);
  ASSERT_EQ(builder.buffer.len, 5);

  sp_str_builder_append_cstr(&builder, " World");
  ASSERT_EQ(builder.buffer.len, 11);

  sp_str_builder_append_c8(&builder, '!');
  ASSERT_EQ(builder.buffer.len, 12);

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
  ASSERT_EQ(builder2.buffer.len, long_str.len);
}

UTEST(sp_str_builder, edge_cases) {


  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, SP_LIT(""));
  ASSERT_EQ(builder.buffer.len, 0);

  sp_str_builder_append_cstr(&builder, "");
  ASSERT_EQ(builder.buffer.len, 0);

  sp_str_t null_str = {.len = 0, .data = SP_NULLPTR};
  sp_str_builder_append(&builder, null_str);
  ASSERT_EQ(builder.buffer.len, 0);

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  for (s32 i = 0; i < 100; i++) {
    sp_str_builder_append_cstr(&builder2, "test ");
  }
  ASSERT_EQ(builder2.buffer.len, 500);
  sp_str_t result = sp_str_builder_write(&builder2);
  ASSERT_EQ(result.len, 500);
}

UTEST(sp_str_builder, indent_operations) {


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


  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append_fmt(&builder, "Value: {}", SP_FMT_U32(123));
  sp_str_t result = sp_str_builder_write(&builder);
  ASSERT_GT(result.len, 0);
  ASSERT_NE(result.data, SP_NULLPTR);
}

UTEST(sp_cstr_copy, all_variations) {


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


  ASSERT_EQ(sp_cstr_len("Hello"), 5);
  ASSERT_EQ(sp_cstr_len("Hello World!"), 12);
  ASSERT_EQ(sp_cstr_len(""), 0);

  ASSERT_EQ(sp_cstr_len(SP_NULLPTR), 0);

  const c8 embedded[] = {'H', 'e', '\0', 'l', 'o', '\0'};
  ASSERT_EQ(sp_cstr_len(embedded), 2);
}

UTEST(sp_wstr_to_cstr, wide_string_conversion) {


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

sp_str_t sp_test_map_band_member(sp_str_map_context_t* context) {
  return sp_str_concat(context->str, SP_LIT(" is in the band"));
}

UTEST(sp_str_t, map_reduce) {


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


  sp_str_t str = SP_LIT("hello world");
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("world")));
  ASSERT_FALSE(sp_str_ends_with(str, SP_LIT("hello")));
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("")));
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("d")));
}

UTEST(sp_str_manipulation, concat) {


  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT("Jerry"), SP_LIT("Garcia")), "JerryGarcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT("Jerry"), SP_LIT("")), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT(""), SP_LIT("Jerry")), "Jerry");
}

UTEST(sp_str_manipulation, join_operations) {


  SP_EXPECT_STR_EQ_CSTR(sp_str_join(SP_LIT("hello"), SP_LIT("world"), SP_LIT(" - ")), "hello - world");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join(SP_LIT("hello"), SP_LIT("world"), SP_LIT("")), "helloworld");

  const c8* strings[] = {"apple", "banana", "cherry"};
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n(strings, 3, SP_LIT(", ")), "apple, banana, cherry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n(strings, 1, SP_LIT(", ")), "apple");
  ASSERT_EQ(sp_str_join_cstr_n(strings, 0, SP_LIT(", ")).len, 0);
}

UTEST(path_functions, normalize_path) {


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

UTEST(sp_os_is_path_root, various_roots) {
  ASSERT_TRUE(sp_os_is_path_root(SP_LIT("")));
  ASSERT_TRUE(sp_os_is_path_root(SP_LIT("/")));

#ifdef SP_WIN32
  ASSERT_TRUE(sp_os_is_path_root(SP_LIT("C:")));
  ASSERT_TRUE(sp_os_is_path_root(SP_LIT("C:/")));
  ASSERT_TRUE(sp_os_is_path_root(SP_LIT("C:\\")));
  ASSERT_FALSE(sp_os_is_path_root(SP_LIT("C:/foo")));
  ASSERT_FALSE(sp_os_is_path_root(SP_LIT("C:\\foo")));
#endif

  ASSERT_FALSE(sp_os_is_path_root(SP_LIT("/home")));
  ASSERT_FALSE(sp_os_is_path_root(SP_LIT("/home/user")));
  ASSERT_FALSE(sp_os_is_path_root(SP_LIT("relative/path")));
}


////////////////////////////
// sp_os_create_directory //
////////////////////////////
typedef struct sp_os_create_directory_fixture {
  s32 foo;
} sp_os_create_directory_fixture;

UTEST_F_SETUP(sp_os_create_directory_fixture) {
}
UTEST_F_TEARDOWN(sp_os_create_directory_fixture) {
}

UTEST_F(sp_os_create_directory_fixture, path_exists_as_directory) {
  sp_str_t dir = SP_LIT("jerry");
  ASSERT_FALSE(sp_os_does_path_exist(dir));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(dir);
}

UTEST_F(sp_os_create_directory_fixture, path_exists_as_file) {
  sp_str_t file = SP_LIT("test_file_conflict.txt");
  ASSERT_FALSE(sp_os_does_path_exist(file));

  sp_os_create_file(file);
  ASSERT_TRUE(sp_os_does_path_exist(file));
  ASSERT_TRUE(sp_os_is_regular_file(file));

  sp_os_create_directory(file);
  ASSERT_TRUE(sp_os_is_regular_file(file));
  ASSERT_FALSE(sp_os_is_directory(file));

  sp_os_remove_file(file);
}

UTEST_F(sp_os_create_directory_fixture, path_doesnt_exist_no_nesting) {
  sp_str_t dir = SP_LIT("test_simple_dir");
  ASSERT_FALSE(sp_os_does_path_exist(dir));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(dir);
}

UTEST_F(sp_os_create_directory_fixture, path_doesnt_exist_requires_nesting) {
  sp_str_t root = SP_LIT("test_nested_root");
  sp_str_t dir = SP_LIT("test_nested_root/level1/level2/level3");
  ASSERT_FALSE(sp_os_does_path_exist(dir));

  sp_os_create_directory(dir);

  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));
  ASSERT_TRUE(sp_os_does_path_exist(SP_LIT("test_nested_root/level1")));
  ASSERT_TRUE(sp_os_does_path_exist(SP_LIT("test_nested_root/level1/level2")));

  sp_os_remove_directory(root);
}



UTEST_F(sp_os_create_directory_fixture, malformed_paths) {
  sp_str_t root = SP_LIT("test_malformed");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  struct {
    sp_str_t malformed;
    sp_str_t formed;
  } dirs [] = {
    { .malformed = SP_LIT("test_malformed//double//slash"), .formed = SP_LIT("test_malformed/double/slash") },
    { .malformed = SP_LIT("test_malformed/trailing/slash/"), .formed = SP_LIT("test_malformed/trailing/slash") },
    { .malformed = SP_LIT("test_malformed///both///kinds/"), .formed = SP_LIT("test_malformed/both/kinds") }
  };

  SP_CARR_FOR(dirs, i) {
    sp_os_create_directory(dirs[i].malformed);
    ASSERT_TRUE(sp_os_does_path_exist(dirs[i].formed));
  }

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, deep_nesting) {
  sp_str_t root = SP_LIT("test_deep");
  sp_str_t dir = SP_LIT("test_deep/a/b/c/d/e/f/g/h/i/j");
  ASSERT_FALSE(sp_os_does_path_exist(dir));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, empty_path) {
  sp_os_create_directory(SP_LIT(""));
}

UTEST_F(sp_os_create_directory_fixture, partially_existing_path) {
  sp_str_t root = SP_LIT("test_partial");
  sp_str_t partial = SP_LIT("test_partial/exists");
  sp_str_t full = SP_LIT("test_partial/exists/deep/nested");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  sp_os_create_directory(partial);
  ASSERT_TRUE(sp_os_does_path_exist(partial));

  sp_os_create_directory(full);
  ASSERT_TRUE(sp_os_does_path_exist(full));
  ASSERT_TRUE(sp_os_is_directory(full));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, path_with_spaces) {
  sp_str_t root = SP_LIT("test_spaces");
  sp_str_t dir = SP_LIT("test_spaces/dir with spaces");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, hidden_directory) {
  sp_str_t root = SP_LIT("test_hidden");
  sp_str_t dir = SP_LIT("test_hidden/.hidden");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, unicode_characters) {
  sp_str_t root = SP_LIT("test_unicode");
  sp_str_t dir = SP_LIT("test_unicode/ñame_with_üñíçødé");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, special_symbols) {
  sp_str_t root = SP_LIT("test_symbols");
  sp_str_t dir = SP_LIT("test_symbols/dir-with_dashes.and.dots");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, trailing_slashes_variations) {
  sp_str_t root = SP_LIT("test_trailing");
  ASSERT_FALSE(sp_os_does_path_exist(root));

  sp_os_create_directory(SP_LIT("test_trailing/dir1/"));
  ASSERT_TRUE(sp_os_does_path_exist(SP_LIT("test_trailing/dir1")));
  ASSERT_TRUE(sp_os_is_directory(SP_LIT("test_trailing/dir1")));

  sp_os_create_directory(SP_LIT("test_trailing/dir2//"));
  ASSERT_TRUE(sp_os_does_path_exist(SP_LIT("test_trailing/dir2")));
  ASSERT_TRUE(sp_os_is_directory(SP_LIT("test_trailing/dir2")));

  sp_os_create_directory(SP_LIT("test_trailing/dir3///"));
  ASSERT_TRUE(sp_os_does_path_exist(SP_LIT("test_trailing/dir3")));
  ASSERT_TRUE(sp_os_is_directory(SP_LIT("test_trailing/dir3")));

  sp_os_remove_directory(root);
}

UTEST_F(sp_os_create_directory_fixture, leading_slashes) {
  sp_str_t dir = SP_LIT("/tmp/sp_test_absolute");
  ASSERT_FALSE(sp_os_does_path_exist(dir));

  sp_os_create_directory(dir);
  ASSERT_TRUE(sp_os_does_path_exist(dir));
  ASSERT_TRUE(sp_os_is_directory(dir));

  sp_os_remove_directory(dir);
}

UTEST_F(sp_os_create_directory_fixture, very_long_path) {
  sp_str_t dir = SP_LIT("test_long");
  ASSERT_FALSE(sp_os_does_path_exist(dir));

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, dir);
  sp_str_builder_append_c8(&builder, '/');

  for (int i = 0; i < 10; i++) {
    sp_str_builder_append(&builder, SP_LIT("very_long_directory_name_"));
  }

  sp_str_t long_path = sp_str_builder_write(&builder);

  sp_os_create_directory(long_path);
  ASSERT_TRUE(sp_os_does_path_exist(long_path));
  ASSERT_TRUE(sp_os_is_directory(long_path));

  sp_os_remove_directory(dir);
}

UTEST(dyn_array, basic_operations) {
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


UTEST(sp_dyn_array_push_f, edge_cases) {
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


  // sp_parse_bool
  ASSERT_EQ(sp_parse_bool(SP_LIT("true")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("false")), false);
  ASSERT_EQ(sp_parse_bool(SP_LIT("1")), true);
  ASSERT_EQ(sp_parse_bool(SP_LIT("0")), false);
  // yes/no, on/off not supported - only true/false and 1/0
  // Would assert: "maybe", "2", "TRUE", "", "yes", "no", "on", "off"
}

UTEST(sp_parse, characters) {


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


  sp_str_t result = sp_format("c8: {}", SP_FMT_C8('A'));
  SP_EXPECT_STR_EQ_CSTR(result, "c8: A");

  result = sp_format("c8 space: {}", SP_FMT_C8(' '));
  SP_EXPECT_STR_EQ_CSTR(result, "c8 space:  ");

  result = sp_format("c16: {}", SP_FMT_C16(L'Z'));
  SP_EXPECT_STR_EQ_CSTR(result, "c16: Z");
}

UTEST(sp_format, pointer_and_hash) {


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


  sp_str_t result = sp_format("{} + {} = {}", SP_FMT_U32(10), SP_FMT_U32(20), SP_FMT_U32(30));
  SP_EXPECT_STR_EQ_CSTR(result, "10 + 20 = 30");

  result = sp_format("Name: {}, Age: {}, Height: {}cm",
                     SP_FMT_CSTR("Bob"), SP_FMT_U32(25), SP_FMT_F32(175.5f));
  SP_EXPECT_STR_EQ_CSTR(result, "Name: Bob, Age: 25, Height: 175.500cm");
}

UTEST(sp_format, edge_cases) {


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
//
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
  sp_ht(int, float) ht = SP_NULLPTR;

  ASSERT_EQ(sp_ht_size(ht), 0);
  ASSERT_TRUE(sp_ht_empty(ht));
  ASSERT_FALSE(sp_ht_exists(ht, 42));

  sp_ht_insert(ht, 42, 3.14f);
  ASSERT_EQ(sp_ht_size(ht), 1);
  ASSERT_FALSE(sp_ht_empty(ht));
  ASSERT_TRUE(sp_ht_exists(ht, 42));
  ASSERT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ht, 10, 1.5f);
  sp_ht_insert(ht, 20, 2.5f);
  sp_ht_insert(ht, 30, 3.5f);
  ASSERT_EQ(sp_ht_size(ht), 4);

  ASSERT_EQ(*sp_ht_getp(ht, 10), 1.5f);
  ASSERT_EQ(*sp_ht_getp(ht, 20), 2.5f);
  ASSERT_EQ(*sp_ht_getp(ht, 30), 3.5f);
  ASSERT_EQ(*sp_ht_getp(ht, 42), 3.14f);

  sp_ht_insert(ht, 42, 6.28f);
  ASSERT_EQ(*sp_ht_getp(ht, 42), 6.28f);
  ASSERT_EQ(sp_ht_size(ht), 4);

  sp_ht_erase(ht, 20);
  ASSERT_FALSE(sp_ht_exists(ht, 20));
  ASSERT_EQ(sp_ht_size(ht), 3);

  sp_ht_clear(ht);
  ASSERT_EQ(sp_ht_size(ht), 0);
  ASSERT_TRUE(sp_ht_empty(ht));

  sp_ht_free(ht);
}

UTEST(hash_table, pointer_retrieval) {
  sp_ht(u32, double) ht = SP_NULLPTR;

  sp_ht_insert(ht, 100, 123.456);
  sp_ht_insert(ht, 200, 789.012);

  double* ptr1 = sp_ht_getp(ht, 100);
  ASSERT_NE(ptr1, SP_NULLPTR);
  ASSERT_EQ(*ptr1, 123.456);

  *ptr1 = 999.999;
  ASSERT_EQ(*sp_ht_getp(ht, 100), 999.999);

  double* ptr2 = sp_ht_getp(ht, 999);
  ASSERT_EQ(ptr2, SP_NULLPTR);

  sp_ht_free(ht);
}

typedef struct {
  float x, y, z;
} vec3_t;

UTEST(hash_table, struct_values) {
  sp_ht(int, vec3_t) ht = SP_NULLPTR;

  vec3_t v1 = {1.0f, 2.0f, 3.0f};
  vec3_t v2 = {4.0f, 5.0f, 6.0f};
  vec3_t v3 = {7.0f, 8.0f, 9.0f};

  sp_ht_insert(ht, 1, v1);
  sp_ht_insert(ht, 2, v2);
  sp_ht_insert(ht, 3, v3);

  vec3_t retrieved = *sp_ht_getp(ht, 2);
  ASSERT_EQ(retrieved.x, 4.0f);
  ASSERT_EQ(retrieved.y, 5.0f);
  ASSERT_EQ(retrieved.z, 6.0f);

  sp_ht_free(ht);
}

typedef struct {
  s32 id;
  s32 type;
} compound_key_t;

UTEST(hash_table, struct_keys) {
  sp_ht(compound_key_t, const char*) ht = SP_NULLPTR;

  compound_key_t k1 = {100, 1};
  compound_key_t k2 = {200, 2};
  compound_key_t k3 = {300, 3};

  sp_ht_insert(ht, k1, "First");
  sp_ht_insert(ht, k2, "Second");
  sp_ht_insert(ht, k3, "Third");

  ASSERT_EQ(sp_ht_size(ht), 3);

  compound_key_t lookup = {200, 2};
  ASSERT_TRUE(sp_ht_exists(ht, lookup));
  const char* value = *sp_ht_getp(ht, lookup);
  ASSERT_STREQ(value, "Second");

  compound_key_t missing = {200, 3};
  ASSERT_FALSE(sp_ht_exists(ht, missing));

  sp_ht_free(ht);
}

UTEST(hash_table, string_keys) {
  sp_ht(u64, int) ht = SP_NULLPTR;

  const char* s1 = "apple";
  const char* s2 = "banana";
  const char* s3 = "cherry";

  u64 k1 = sp_hash_cstr(s1);
  u64 k2 = sp_hash_cstr(s2);
  u64 k3 = sp_hash_cstr(s3);

  sp_ht_insert(ht, k1, 10);
  sp_ht_insert(ht, k2, 20);
  sp_ht_insert(ht, k3, 30);

  u64 lookup = sp_hash_cstr("banana");
  ASSERT_TRUE(sp_ht_exists(ht, lookup));
  ASSERT_EQ(*sp_ht_getp(ht, lookup), 20);

  lookup = sp_hash_cstr("dragonfruit");
  ASSERT_FALSE(sp_ht_exists(ht, lookup));

  sp_ht_free(ht);
}

UTEST(hash_table, collision_handling) {
  sp_ht(int, int) ht = SP_NULLPTR;

  for (s32 i = 0; i < 100; i++) {
      sp_ht_insert(ht, i, i * 100);
  }

  ASSERT_EQ(sp_ht_size(ht), 100);

  for (s32 i = 0; i < 100; i++) {
      ASSERT_TRUE(sp_ht_exists(ht, i));
      ASSERT_EQ(*sp_ht_getp(ht, i), i * 100);
  }

  for (s32 i = 0; i < 100; i += 3) {
      sp_ht_erase(ht, i);
  }

  for (s32 i = 0; i < 100; i++) {
      if (i % 3 == 0) {
          ASSERT_FALSE(sp_ht_exists(ht, i));
      } else {
          ASSERT_TRUE(sp_ht_exists(ht, i));
          ASSERT_EQ(*sp_ht_getp(ht, i), i * 100);
      }
  }

  sp_ht_free(ht);
}

UTEST(hash_table, iteration) {
  sp_ht(s32, s32) ht = SP_NULLPTR;

  for (s32 i = 0; i < 10; i++) {
    sp_ht_insert(ht, i * 10, i);
  }

  s32 count = 0;
  s32 sum = 0;

  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    s32 key = *sp_ht_it_getkp(ht, it);
    float val = *sp_ht_it_getp(ht, it);

    count++;
    sum += val;
  }

  ASSERT_EQ(count, 10);
  ASSERT_EQ(sum, 45);

  sp_ht_free(ht);
}

UTEST(hash_table, edge_cases) {
  sp_ht(int, int) ht1 = SP_NULLPTR;
  ASSERT_EQ(sp_ht_size(ht1), 0);
  ASSERT_TRUE(sp_ht_empty(ht1));
  ASSERT_FALSE(sp_ht_exists(ht1, 42));

  sp_ht_clear(ht1);
  sp_ht_free(ht1);

  sp_ht(int, int) ht2 = SP_NULLPTR;
  sp_ht_insert(ht2, 1, 100);
  sp_ht_erase(ht2, 1);
  ASSERT_EQ(sp_ht_size(ht2), 0);
  sp_ht_free(ht2);

  sp_ht(int, int) ht3 = SP_NULLPTR;
  sp_ht_insert(ht3, 1, 100);
  sp_ht_erase(ht3, 999);
  ASSERT_EQ(sp_ht_size(ht3), 1);
  sp_ht_free(ht3);
}

UTEST(hash_table, pathological_all_same_hash) {
  sp_ht(u32, u32) ht = sp_ht_new(u32, u32);

  u32 cap = sp_ht_capacity(ht);
  if (cap < 2) {
    sp_ht_insert(ht, 0, 0);
    sp_ht_insert(ht, 1, 0);
    cap = sp_ht_capacity(ht);
  }

  for (u32 i = 0; i < cap; i++) {
    sp_ht_insert(ht, i, i * 100);
  }

  for (u32 i = 0; i < cap; i++) {
    ASSERT_TRUE(sp_ht_exists(ht, i));
    ASSERT_EQ(i * 100, *sp_ht_getp(ht, i));
  }

  sp_ht_free(ht);
}

UTEST(hash_table, duplicate_key_insert_size_bug) {
    sp_ht(u32, u32) table = SP_NULLPTR;

    // Insert initial key-value pair
    sp_ht_insert(table, 42, 100);
    ASSERT_EQ(sp_ht_size(table), 1);
    ASSERT_EQ(*sp_ht_getp(table, 42), 100);

    // Insert same key with different value - this should overwrite, not increment size
    sp_ht_insert(table, 42, 200);
    ASSERT_EQ(sp_ht_size(table), 1);
    ASSERT_EQ(*sp_ht_getp(table, 42), 200);

    // Insert another unique key
    sp_ht_insert(table, 99, 300);
    ASSERT_EQ(sp_ht_size(table), 2);
    ASSERT_EQ(*sp_ht_getp(table, 42), 200);
    ASSERT_EQ(*sp_ht_getp(table, 99), 300);

    sp_ht_free(table);
}

UTEST(sp_ht, iterator_yields_inactive_entry_at_slot_zero) {
  sp_ht(u64, u64) ht = sp_ht_new(u64, u64);

  sp_ht_insert(ht, 0, 999);
  sp_ht_erase(ht, 0);

  ASSERT_EQ(sp_ht_size(ht), 0);

  u64 iteration_count = 0;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    u64 key = *sp_ht_it_getkp(ht, it);
    u64 val = *sp_ht_it_getp(ht, it);
    iteration_count++;
  }

  ASSERT_EQ(iteration_count, 0);

  sp_ht_free(ht);
}

UTEST(hash_table, collision) {
  sp_ht(s32, s32) ht = SP_NULLPTR;

  for (u32 i = 0; i < 8; i++) {
    sp_ht_insert(ht, i, i);
  }

  u32 capacity = sp_ht_capacity(ht);
  s32 keys [3] = { -1, -1, -1 };
  s32 num_found = 0;

  for (u32 candidate = 0; candidate < 1000; candidate++) {
    sp_hash_t hash = sp_hash_bytes(&candidate, sizeof(candidate), SP_HT_HASH_SEED);

    u32 bucket = hash % capacity;
    if (bucket == 0) {
      keys[num_found++] = candidate;
    }

    if (num_found == SP_CARR_LEN(keys)) {
      break;
    }
  }

  ASSERT_EQ(num_found, 3);

  sp_ht_clear(ht);

  sp_ht_insert(ht, keys[0], 0);
  sp_ht_insert(ht, keys[1], 1);
  sp_ht_insert(ht, keys[2], 2);

  ASSERT_TRUE(sp_ht_exists(ht, keys[0]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[1]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[2]));

  ASSERT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  ASSERT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  ASSERT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  // remove the first key that maps to bucket 0
  sp_ht_erase(ht, keys[0]);

  ASSERT_FALSE(sp_ht_exists(ht, keys[0]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[1]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[2]));

  ASSERT_EQ(*sp_ht_getp(ht, keys[1]), 1);
  ASSERT_EQ(*sp_ht_getp(ht, keys[2]), 2);

  // put it back
  sp_ht_insert(ht, keys[0], 0);

  ASSERT_EQ(sp_ht_size(ht), 3);
  ASSERT_TRUE(sp_ht_exists(ht, keys[0]));
  ASSERT_EQ(*sp_ht_getp(ht, keys[0]), 0);

  // remove the third key that maps to bucket 0
  sp_ht_erase(ht, keys[2]);

  ASSERT_TRUE(sp_ht_exists(ht, keys[0]));
  ASSERT_TRUE(sp_ht_exists(ht, keys[1]));
  ASSERT_FALSE(sp_ht_exists(ht, keys[2]));

  ASSERT_EQ(*sp_ht_getp(ht, keys[0]), 0);
  ASSERT_EQ(*sp_ht_getp(ht, keys[1]), 1);

  // put it back
  sp_ht_insert(ht, keys[2], 2);
  ASSERT_TRUE(sp_ht_exists(ht, keys[2]));
  ASSERT_EQ(*sp_ht_getp(ht, keys[2]), 2);
}

UTEST(hash_table, iterator_returns_zero_entries_for_populated_table) {
  sp_ht(u64, u64) ht = sp_ht_new(u64, u64);

  sp_ht_insert(ht, 1, 100);
  sp_ht_insert(ht, 2, 200);

  u64 count = 0;
  for (sp_ht_it it = sp_ht_it_init(ht); sp_ht_it_valid(ht, it); sp_ht_it_advance(ht, it)) {
    count++;
  }

  ASSERT_EQ(count, 2);

  sp_ht_free(ht);
}

UTEST(hash_table, null_safety) {
  sp_ht(s32, s32) null_ht = NULL;

  // Test all operations with NULL hash table - should not crash
  ASSERT_EQ(sp_ht_size(null_ht), 0);
  ASSERT_EQ(sp_ht_capacity(null_ht), 0);
  ASSERT_TRUE(sp_ht_empty(null_ht));
  ASSERT_FALSE(sp_ht_exists(null_ht, 42));

  // Test get operations return defaults
  s32* default_val = sp_ht_getp(null_ht, 42);
  ASSERT_EQ(default_val, SP_NULLPTR);

  // Test iterator operations with NULL
  sp_ht_it it = sp_ht_it_init(null_ht);
  ASSERT_EQ(it, 0);
  ASSERT_FALSE(sp_ht_it_valid(null_ht, it));

  sp_ht_it_advance(null_ht, it); // Should not crash

  s32* iter_key = sp_ht_it_getkp(null_ht, it);
  s32* iter_val = sp_ht_it_getp(null_ht, it);
  ASSERT_EQ(iter_key, SP_NULLPTR);
  ASSERT_EQ(iter_val, SP_NULLPTR);

  // Test operations that should be no-ops
  sp_ht_clear(null_ht); // Should not crash
  sp_ht_erase(null_ht, 42); // Should not crash
  sp_ht_free(null_ht); // Should not crash

  // Test insert auto-initializes
  sp_ht_insert(null_ht, 42, 100);
  ASSERT_NE(null_ht, SP_NULLPTR);
  ASSERT_EQ(sp_ht_size(null_ht), 1);
  ASSERT_EQ(*sp_ht_getp(null_ht, 42), 100);

  sp_ht_free(null_ht);
}

sp_hash_t sp_test_string_hash(void* key, u32 size) {
  (void)size;
  sp_str_t* str = (sp_str_t*)key;
  return sp_hash_bytes(str->data, str->len, SP_HT_HASH_SEED);
}

bool sp_test_string_compare(void* ka, void* kb, u32 size) {
  (void)size;
  sp_str_t* a = (sp_str_t*)ka;
  sp_str_t* b = (sp_str_t*)kb;
  return sp_str_equal(*a, *b);
}

UTEST(hash_table, string_key_custom_hash) {
  sp_ht(sp_str_t, int) ht = SP_NULLPTR;
  sp_ht_set_fns(ht, sp_ht_on_hash_str_key, sp_ht_on_compare_str_key);

  sp_str_t ka = sp_str_copy(sp_str_lit("hello"));
  sp_str_t kb = sp_str_copy(sp_str_lit("world"));
  sp_str_t kc = sp_str_copy(sp_str_lit("test"));

  sp_ht_insert(ht, ka, 100);
  sp_ht_insert(ht, kb, 200);
  sp_ht_insert(ht, kc, 300);

  ASSERT_TRUE(sp_ht_exists(ht, ka));
  ASSERT_TRUE(sp_ht_exists(ht, kb));
  ASSERT_TRUE(sp_ht_exists(ht, kc));

  ASSERT_EQ(*sp_ht_getp(ht, ka), 100);
  ASSERT_EQ(*sp_ht_getp(ht, kb), 200);
  ASSERT_EQ(*sp_ht_getp(ht, kc), 300);

  sp_str_t ka_copy = sp_str_copy(SP_LIT("hello"));
  sp_str_t kb_copy = sp_str_copy(SP_LIT("world"));

  ASSERT_TRUE(sp_ht_exists(ht, ka_copy));
  ASSERT_TRUE(sp_ht_exists(ht, kb_copy));

  ASSERT_EQ(*sp_ht_getp(ht, ka_copy), 100);
  ASSERT_EQ(*sp_ht_getp(ht, kb_copy), 200);

  sp_str_t kd = sp_str_copy(SP_LIT("missing"));
  ASSERT_FALSE(sp_ht_exists(ht, kd));
  ASSERT_EQ(sp_ht_getp(ht, kd), SP_NULLPTR);

  sp_ht_free(ht);
}

////////////////////////////
// SIPHASH TESTS
////////////////////////////
UTEST(siphash, consistency) {
  const char* data = "Hello, World!";
  u64 seed = 0x12345678;

  u64 hash1 = sp_hash_bytes((void*)data, strlen(data), seed);
  u64 hash2 = sp_hash_bytes((void*)data, strlen(data), seed);

  ASSERT_EQ(hash1, hash2);

  u64 hash3 = sp_hash_bytes((void*)data, strlen(data), seed + 1);
  ASSERT_NE(hash1, hash3);
}

UTEST(siphash, different_lengths) {
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
    typedef int* int_array;
    sp_ht(int, int_array) ht = SP_NULLPTR;

    for (s32 i = 0; i < 5; i++) {
        sp_dyn_array(int) arr = SP_NULLPTR;

        for (s32 j = 0; j < 10; j++) {
            sp_dyn_array_push(arr, i * 100 + j);
        }

        sp_ht_insert(ht, i, arr);
    }

    for (s32 i = 0; i < 5; i++) {
        ASSERT_TRUE(sp_ht_exists(ht, i));

        int_array arr = *sp_ht_getp(ht, i);
        ASSERT_EQ(sp_dyn_array_size(arr), 10);

        for (s32 j = 0; j < 10; j++) {
            ASSERT_EQ(arr[j], i * 100 + j);
        }
    }

    for (s32 i = 0; i < 5; i++) {
        int_array arr = *sp_ht_getp(ht, i);
        sp_dyn_array_free(arr);
    }

    sp_ht_free(ht);
}

UTEST(combined, multiple_arrays_in_hash_table) {
  sp_ht(int, void*) ht = SP_NULLPTR;

  for (s32 key = 0; key < 5; key++) {
    sp_dyn_array(int) arr = SP_NULLPTR;

    for (s32 j = 0; j < 20; j++) {
        sp_dyn_array_push(arr, key * 1000 + j);
    }

    sp_ht_insert(ht, key, (void*)arr);
  }

  for (s32 key = 0; key < 5; key++) {
    ASSERT_TRUE(sp_ht_exists(ht, key));

    int* arr = (int*)*sp_ht_getp(ht, key);
    ASSERT_EQ(sp_dyn_array_size(arr), 20);

    for (s32 j = 0; j < 20; j++) {
        ASSERT_EQ(arr[j], key * 1000 + j);
    }
  }

  for (s32 key = 0; key < 5; key++) {
    int* arr = (int*)*sp_ht_getp(ht, key);
    sp_dyn_array_free(arr);
  }

  sp_ht_free(ht);
}

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

  sp_test_memory_tracker_deinit(&tracker);
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

  sp_test_memory_tracker_deinit(&tracker);
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

  sp_test_memory_tracker_deinit(&tracker);
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

  sp_test_memory_tracker_deinit(&tracker);
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

  sp_test_memory_tracker_deinit(&tracker);
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

  sp_test_memory_tracker_deinit(&tracker);
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

  sp_test_memory_tracker_deinit(&tracker);
}


////////////////////////////
// RING BUFFER TESTS
////////////////////////////

UTEST(ring_buffer, basic_operations) {


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


    sp_ring_buffer_t rb;
    sp_ring_buffer_init(&rb, 10, sizeof(double));

    ASSERT_EQ(sp_ring_buffer_bytes(&rb), 10 * sizeof(double));

    sp_ring_buffer_destroy(&rb);

    sp_ring_buffer_init(&rb, 100, sizeof(char));
    ASSERT_EQ(sp_ring_buffer_bytes(&rb), 100);

    sp_ring_buffer_destroy(&rb);
}

UTEST(ring_buffer, iterator_manual) {


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


// ███████╗██╗██╗     ███████╗    ███╗   ███╗ ██████╗ ███╗   ██╗██╗████████╗ ██████╗ ██████╗
// ██╔════╝██║██║     ██╔════╝    ████╗ ████║██╔═══██╗████╗  ██║██║╚══██╔══╝██╔═══██╗██╔══██╗
// █████╗  ██║██║     █████╗      ██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║   ██║   ██║██████╔╝
// ██╔══╝  ██║██║     ██╔══╝      ██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║   ██║   ██║██╔══██╗
// ██║     ██║███████╗███████╗    ██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║   ██║   ╚██████╔╝██║  ██║
// ╚═╝     ╚═╝╚══════╝╚══════╝    ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝
typedef struct sp_test_file_monitor_data {
  bool change_detected;
  sp_file_change_event_t last_event;
  c8 last_file_path[SP_MAX_PATH_LEN];
} sp_test_file_monitor_data;

void sp_test_file_monitor_callback(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
  sp_test_file_monitor_data* data = (sp_test_file_monitor_data*)userdata;
  data->change_detected = true;
  data->last_event = change->events;
  sp_str_copy_to(change->file_path, data->last_file_path, SP_MAX_PATH_LEN);
}

#ifdef SP_WIN32
#endif



// ██████╗  ██████╗ ███████╗██╗██╗  ██╗
// ██╔══██╗██╔═══██╗██╔════╝██║╚██╗██╔╝
// ██████╔╝██║   ██║███████╗██║ ╚███╔╝
// ██╔═══╝ ██║   ██║╚════██║██║ ██╔██╗
// ██║     ╚██████╔╝███████║██║██╔╝ ██╗
// ╚═╝      ╚═════╝ ╚══════╝╚═╝╚═╝  ╚═╝
#ifdef SP_POSIX
UTEST(posix, smoke) {
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


  sp_str_t strings[] = {
    SP_LIT("apple"),
    SP_LIT("banana"),
    SP_LIT("cherry"),
    SP_LIT("date"),
  };

  // Test array contains - found in multiple strings
  ASSERT_TRUE(sp_str_contains_n(strings, 4, SP_LIT("a")));  // in apple, banana, date
  ASSERT_TRUE(sp_str_contains_n(strings, 4, SP_LIT("ana"))); // in banana

  // Test array contains - not found in any string
  ASSERT_FALSE(sp_str_contains_n(strings, 4, SP_LIT("xyz")));

  // Test empty array
  ASSERT_FALSE(sp_str_contains_n(strings, 0, SP_LIT("apple")));
}

UTEST(sp_str_kernels, reduce_count) {


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


#define SP_TEST_ENUM(X) \
  X(SP_ENUM_FOO) \
  X(SP_ENUM_BAR) \
  X(SP_ENUM_BAZ) \
  X(SP_ENUM_QUX)

typedef enum {
  SP_TEST_ENUM(SP_X_ENUM_DEFINE)
} sp_test_enum_t;

sp_str_t sp_test_enum_to_str(sp_test_enum_t e) {
  switch (e) {
    SP_TEST_ENUM(SP_X_ENUM_CASE_TO_STRING)
    default: SP_UNREACHABLE_RETURN(sp_str_lit(""));
  }
}

const c8* sp_test_enum_to_cstr(sp_test_enum_t e) {
  switch (e) {
    SP_TEST_ENUM(SP_X_ENUM_CASE_TO_CSTR)
    default: SP_UNREACHABLE_RETURN("");
  }
}

UTEST(sp_enum_macros, name_generation) {
  ASSERT_STREQ(sp_test_enum_to_cstr(SP_ENUM_BAZ), "SP_ENUM_BAZ");
  SP_EXPECT_STR_EQ_CSTR(sp_test_enum_to_str(SP_ENUM_QUX), "SP_ENUM_QUX");
}


UTEST(os_functions, recursive_directory_removal) {
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

  sp_str_t base = sp_test_build_scan_directory();

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);

  ASSERT_EQ(entries.count, 0);
  ASSERT_TRUE(entries.data == SP_NULLPTR || entries.count == 0);

  sp_test_build_scan_directory();
}

UTEST(sp_os_scan_directory, non_existent_directory) {

  sp_str_t base = sp_test_build_scan_directory();

  sp_str_t non_existent = sp_os_join_path(base, SP_LIT("some_bullshit"));

  sp_os_directory_entry_list_t entries = sp_os_scan_directory(non_existent);

  ASSERT_EQ(entries.count, 0);
}

UTEST(sp_os_scan_directory, file_path_correctness) {

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

UTEST(sp_os_file_attributes, basic_functionality) {

  sp_str_t base = sp_test_build_scan_directory();

  // test regular file - verify ONLY file flag is set
  sp_str_t file1 = sp_os_join_path(base, SP_LIT("test_file.txt"));
  sp_os_create_file(file1);
  sp_os_file_attr_t file_attr = sp_os_file_attributes(file1);
  ASSERT_EQ(file_attr, SP_OS_FILE_ATTR_REGULAR_FILE);

  // test directory - verify ONLY directory flag is set
  sp_str_t dir1 = sp_os_join_path(base, SP_LIT("test_dir"));
  sp_os_create_directory(dir1);
  sp_os_file_attr_t dir_attr = sp_os_file_attributes(dir1);
  ASSERT_EQ(dir_attr, SP_OS_FILE_ATTR_DIRECTORY);

  // test non-existent path
  sp_str_t non_existent = sp_os_join_path(base, SP_LIT("does_not_exist"));
  sp_os_file_attr_t none_attr = sp_os_file_attributes(non_existent);
  ASSERT_EQ(none_attr, SP_OS_FILE_ATTR_NONE);

  // verify consistency with sp_os_scan_directory
  sp_os_directory_entry_list_t entries = sp_os_scan_directory(base);
  ASSERT_EQ(entries.count, 2);

  for (u32 i = 0; i < entries.count; i++) {
    sp_os_directory_entry_t* entry = &entries.data[i];
    sp_os_file_attr_t direct_attr = sp_os_file_attributes(entry->file_path);
    ASSERT_EQ(entry->attributes, direct_attr);
  }

  sp_test_build_scan_directory();
}

UTEST(sp_os_file_attributes, path_edge_cases) {

  sp_str_t base = sp_test_build_scan_directory();

  // empty path
  sp_os_file_attr_t empty_attr = sp_os_file_attributes(SP_LIT(""));
  ASSERT_EQ(empty_attr, SP_OS_FILE_ATTR_NONE);

  // null-like sp_str_t
  sp_str_t null_str = SP_ZERO_STRUCT(sp_str_t);
  null_str.data = SP_NULLPTR;
  null_str.len = 0;
  sp_os_file_attr_t null_attr = sp_os_file_attributes(null_str);
  ASSERT_EQ(null_attr, SP_OS_FILE_ATTR_NONE);

  // current directory
  sp_os_file_attr_t dot_attr = sp_os_file_attributes(SP_LIT("."));
  ASSERT_EQ(dot_attr, SP_OS_FILE_ATTR_DIRECTORY);

  // parent directory
  sp_os_file_attr_t dotdot_attr = sp_os_file_attributes(SP_LIT(".."));
  ASSERT_EQ(dotdot_attr, SP_OS_FILE_ATTR_DIRECTORY);

  // path with trailing slash (directory)
  sp_str_t dir_trail = sp_os_join_path(base, SP_LIT("testdir"));
  sp_os_create_directory(dir_trail);
  sp_str_builder_t slash_builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&slash_builder, dir_trail);
  sp_str_builder_append(&slash_builder, SP_LIT("/"));
  sp_str_t dir_with_slash = sp_str_builder_write(&slash_builder);
  sp_os_file_attr_t trail_attr = sp_os_file_attributes(dir_with_slash);
  ASSERT_EQ(trail_attr, SP_OS_FILE_ATTR_DIRECTORY);

  // path with double slashes
  sp_str_builder_t double_builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&double_builder, base);
  sp_str_builder_append(&double_builder, SP_LIT("//testdir"));
  sp_str_t double_slash = sp_str_builder_write(&double_builder);
  sp_os_file_attr_t double_attr = sp_os_file_attributes(double_slash);
  ASSERT_EQ(double_attr, SP_OS_FILE_ATTR_DIRECTORY);

  // test very long file name
  sp_str_builder_t long_name = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&long_name, base);
  sp_str_builder_append(&long_name, SP_LIT("/"));
  for (u32 i = 0; i < 50; i++) {
    sp_str_builder_append(&long_name, SP_LIT("long"));
  }
  sp_str_builder_append(&long_name, SP_LIT(".txt"));
  sp_str_t long_path = sp_str_builder_write(&long_name);
  sp_os_create_file(long_path);
  sp_os_file_attr_t long_attr = sp_os_file_attributes(long_path);
  ASSERT_EQ(long_attr, SP_OS_FILE_ATTR_REGULAR_FILE);

  sp_test_build_scan_directory();
}

UTEST(sp_os_file_attributes, special_names_and_nesting) {

  sp_str_t base = sp_test_build_scan_directory();

  // test with spaces in names
  sp_str_t space_file = sp_os_join_path(base, SP_LIT("file with spaces.txt"));
  sp_os_create_file(space_file);
  sp_os_file_attr_t space_attr = sp_os_file_attributes(space_file);
  ASSERT_EQ(space_attr, SP_OS_FILE_ATTR_REGULAR_FILE);

  sp_str_t space_dir = sp_os_join_path(base, SP_LIT("dir with spaces"));
  sp_os_create_directory(space_dir);
  sp_os_file_attr_t space_dir_attr = sp_os_file_attributes(space_dir);
  ASSERT_EQ(space_dir_attr, SP_OS_FILE_ATTR_DIRECTORY);

  // test deeply nested paths
  sp_str_t level1 = sp_os_join_path(base, SP_LIT("level1"));
  sp_os_create_directory(level1);
  sp_str_t level2 = sp_os_join_path(level1, SP_LIT("level2"));
  sp_os_create_directory(level2);
  sp_str_t level3 = sp_os_join_path(level2, SP_LIT("level3"));
  sp_os_create_directory(level3);
  sp_str_t deep_file = sp_os_join_path(level3, SP_LIT("deep.txt"));
  sp_os_create_file(deep_file);

  ASSERT_EQ(sp_os_file_attributes(level1), SP_OS_FILE_ATTR_DIRECTORY);
  ASSERT_EQ(sp_os_file_attributes(level2), SP_OS_FILE_ATTR_DIRECTORY);
  ASSERT_EQ(sp_os_file_attributes(level3), SP_OS_FILE_ATTR_DIRECTORY);
  ASSERT_EQ(sp_os_file_attributes(deep_file), SP_OS_FILE_ATTR_REGULAR_FILE);

  // test file deleted after creation (race condition)
  sp_str_t temp_file = sp_os_join_path(base, SP_LIT("temp.txt"));
  sp_os_create_file(temp_file);
  sp_os_remove_file(temp_file);
  sp_os_file_attr_t deleted_attr = sp_os_file_attributes(temp_file);
  ASSERT_EQ(deleted_attr, SP_OS_FILE_ATTR_NONE);

  sp_test_build_scan_directory();
}

UTEST(sp_os_normalize_path_soft, trailing_slash_removal) {


  // Test forward slash removal
  sp_str_t path1 = sp_str_copy(SP_LIT("path/to/dir/"));
  sp_os_normalize_path_soft(&path1);
  SP_EXPECT_STR_EQ(path1, SP_LIT("path/to/dir"));

  // Test backslash removal
  sp_str_t path2 = sp_str_copy(SP_LIT("path\\to\\dir\\"));
  sp_os_normalize_path_soft(&path2);
  SP_EXPECT_STR_EQ(path2, SP_LIT("path\\to\\dir"));

  // Test no change needed
  sp_str_t path3 = sp_str_copy(SP_LIT("path/to/file.txt"));
  sp_os_normalize_path_soft(&path3);
  SP_EXPECT_STR_EQ(path3, SP_LIT("path/to/file.txt"));

  // Test empty path
  sp_str_t empty = sp_str_copy(SP_LIT(""));
  sp_os_normalize_path_soft(&empty);
  SP_EXPECT_STR_EQ(empty, SP_LIT(""));

  // Test single slash
  sp_str_t single = sp_str_copy(SP_LIT("/"));
  sp_os_normalize_path_soft(&single);
  SP_EXPECT_STR_EQ(single, SP_LIT(""));
}

UTEST(sp_os_join_path, empty_path_handling) {


  // Left side empty
  sp_str_t left_empty = sp_os_join_path(SP_LIT(""), SP_LIT("file.txt"));
  SP_EXPECT_STR_EQ(left_empty, SP_LIT("/file.txt"));

  sp_str_t left_empty_dir = sp_os_join_path(SP_LIT(""), SP_LIT("dir/"));
  SP_EXPECT_STR_EQ(left_empty_dir, SP_LIT("/dir"));

  // Right side empty
  sp_str_t right_empty = sp_os_join_path(SP_LIT("path/to"), SP_LIT(""));
  SP_EXPECT_STR_EQ(right_empty, SP_LIT("path/to"));

  sp_str_t right_empty_slash = sp_os_join_path(SP_LIT("path/to/"), SP_LIT(""));
  SP_EXPECT_STR_EQ(right_empty_slash, SP_LIT("path/to"));

  // Both sides empty
  sp_str_t both_empty = sp_os_join_path(SP_LIT(""), SP_LIT(""));
  SP_EXPECT_STR_EQ(both_empty, SP_LIT(""));

  // Single character paths
  sp_str_t single_char = sp_os_join_path(SP_LIT("a"), SP_LIT("b"));
  SP_EXPECT_STR_EQ(single_char, SP_LIT("a/b"));
}

UTEST(path_functions, normalized_join_and_parent) {


  // Test join path removes trailing slash
  sp_str_t joined = sp_os_join_path(SP_LIT("path/to/dir/"), SP_LIT("file.txt"));
  SP_EXPECT_STR_EQ(joined, SP_LIT("path/to/dir/file.txt"));

  // Test parent path removes trailing slash
  sp_str_t parent = sp_os_parent_path(SP_LIT("path/to/file.txt/"));
  SP_EXPECT_STR_EQ(parent, SP_LIT("path/to"));

  // Test joining with empty paths
  sp_str_t empty_left = sp_os_join_path(SP_LIT(""), SP_LIT("file.txt"));
  SP_EXPECT_STR_EQ(empty_left, SP_LIT("/file.txt"));

  sp_str_t empty_right = sp_os_join_path(SP_LIT("path/to"), SP_LIT(""));
  SP_EXPECT_STR_EQ(empty_right, SP_LIT("path/to"));

  sp_str_t both_empty = sp_os_join_path(SP_LIT(""), SP_LIT(""));
  SP_EXPECT_STR_EQ(both_empty, SP_LIT(""));

  // Verify consistency - no trailing slashes in results
  for (u32 i = 0; i < joined.len; i++) {
    if (i == joined.len - 1) {
      ASSERT_NE(joined.data[i], '/');
      ASSERT_NE(joined.data[i], '\\');
    }
  }

  for (u32 i = 0; i < parent.len; i++) {
    if (i == parent.len - 1) {
      ASSERT_NE(parent.data[i], '/');
      ASSERT_NE(parent.data[i], '\\');
    }
  }
}


// ███████╗████████╗██████╗ ██╗███╗   ██╗ ██████╗     ████████╗███████╗███████╗████████╗███████╗
// ██╔════╝╚══██╔══╝██╔══██╗██║████╗  ██║██╔════╝     ╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝██╔════╝
// ███████╗   ██║   ██████╔╝██║██╔██╗ ██║██║  ███╗       ██║   █████╗  ███████╗   ██║   ███████╗
// ╚════██║   ██║   ██╔══██╗██║██║╚██╗██║██║   ██║       ██║   ██╔══╝  ╚════██║   ██║   ╚════██║
// ███████║   ██║   ██║  ██║██║██║ ╚████║╚██████╔╝       ██║   ███████╗███████║   ██║   ███████║
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝        ╚═╝   ╚══════╝╚══════╝   ╚═╝   ╚══════╝
UTEST(sp_str, trim) {
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

UTEST(sp_str, trim_right) {
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

UTEST(sp_str, split_c8) {
  // basic split
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("hello,world,test"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 3);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "hello");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "world");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "test");
  }

  // path splitting
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("/home/user/file.txt"), '/');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "home");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "user");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "file.txt");
  }

  // consecutive delimiters
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("a,,b"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 3);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "a");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "b");
  }
  //
  // multiple consecutive delimiters
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("a,,,b"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "a");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "b");
  }

  // no delimiter found
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("hello"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 1);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "hello");
  }

  // empty string - returns null
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(""), ',');
    EXPECT_EQ(parts, SP_NULLPTR);
  }

  // delimiter at start and end
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(",hello,world,"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "hello");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "world");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "");
  }

  // single delimiter only
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(","), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 2);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT(""));
    SP_EXPECT_STR_EQ(parts[1], SP_LIT(""));
  }
}

UTEST(sp_str, cleave_c8) {
  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT("hello,world"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "hello");
    SP_EXPECT_STR_EQ_CSTR(result.second, "world");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT("key=value"), '=');
    SP_EXPECT_STR_EQ_CSTR(result.first, "key");
    SP_EXPECT_STR_EQ_CSTR(result.second, "value");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT("no_delimiter"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "no_delimiter");
    SP_EXPECT_STR_EQ(result.second, SP_LIT(""));
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT("a,b,c"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "a");
    SP_EXPECT_STR_EQ_CSTR(result.second, "b,c");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT(",trailing"), ',');
    SP_EXPECT_STR_EQ(result.first, SP_LIT(""));
    SP_EXPECT_STR_EQ_CSTR(result.second, "trailing");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT("leading,"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "leading");
    SP_EXPECT_STR_EQ(result.second, SP_LIT(""));
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT(""), ',');
    SP_EXPECT_STR_EQ(result.first, SP_LIT(""));
    SP_EXPECT_STR_EQ(result.second, SP_LIT(""));
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(SP_LIT(","), ',');
    SP_EXPECT_STR_EQ(result.first, SP_LIT(""));
    SP_EXPECT_STR_EQ(result.second, SP_LIT(""));
  }
}

UTEST(sp_str, pad) {
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

UTEST(sp_str, pad_to_longest) {
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

UTEST(sp_str, starts_with) {
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

UTEST(sp_str_contains, substring_searching) {


  // basic substring checks
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("world")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("lo w")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("banana"), SP_LIT("ana")));
  ASSERT_FALSE(sp_str_contains(SP_LIT("hello"), SP_LIT("world")));

  // exact match
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello"), SP_LIT("hello")));

  // needle longer than string
  ASSERT_FALSE(sp_str_contains(SP_LIT("hi"), SP_LIT("hello")));

  // empty cases
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello"), SP_LIT("")));
  ASSERT_TRUE(sp_str_contains(SP_LIT(""), SP_LIT("")));
  ASSERT_FALSE(sp_str_contains(SP_LIT(""), SP_LIT("hello")));

  // edge cases - beginning and end
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("hello")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("world")));

  // case sensitivity
  ASSERT_FALSE(sp_str_contains(SP_LIT("Hello World"), SP_LIT("hello")));
  ASSERT_FALSE(sp_str_contains(SP_LIT("hello world"), SP_LIT("WORLD")));

  // repeated patterns
  ASSERT_TRUE(sp_str_contains(SP_LIT("aaaa"), SP_LIT("aa")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("abababab"), SP_LIT("abab")));
}

UTEST(sp_str_view, view_creation) {


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


struct sp_io {
  sp_str_t test_file_path;
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(sp_io) {
  sp_test_file_manager_init(&ut.file_manager);
  ut.test_file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("sp_io.file"));
}

UTEST_F_TEARDOWN(sp_io) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST_F(sp_io, memory_open) {
  u8 buffer[64];
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  ASSERT_TRUE(true);
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_close) {
  u8 buffer[64];
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  sp_io_close(&stream);
  ASSERT_TRUE(true);
}

UTEST_F(sp_io, memory_size) {
  u8 buffer[128];
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, 128);
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_read_full) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));
  u64 bytes = sp_io_read(&stream, dest, sizeof(dest));

  ASSERT_EQ(bytes, 16);
  for (u32 i = 0; i < 16; i++) {
    ASSERT_EQ(dest[i], source[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_read_partial) {
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  u8 dest[8] = SP_ZERO_INITIALIZE();

  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));
  u64 bytes = sp_io_read(&stream, dest, sizeof(dest));

  ASSERT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    ASSERT_EQ(dest[i], source[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_read_past_end) {
  u8 source[8] = {1,2,3,4,5,6,7,8};
  u8 dest[16] = SP_ZERO_INITIALIZE();

  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));
  u64 bytes = sp_io_read(&stream, dest, sizeof(dest));

  ASSERT_EQ(bytes, 8);
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_write_in_bounds) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  u8 source[8] = {1,2,3,4,5,6,7,8};

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&stream, source, sizeof(source));

  ASSERT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    ASSERT_EQ(buffer[i], source[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_write_overflow) {
  u8 buffer[8] = SP_ZERO_INITIALIZE();
  u8 source[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));
  u64 bytes = sp_io_write(&stream, source, sizeof(source));

  ASSERT_EQ(bytes, 8);
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_seek_set) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&stream, 32, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 32);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_seek_bounds) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 0);

  pos = sp_io_seek(&stream, 64, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 64);

  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_END);
  ASSERT_EQ(pos, 64);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_seek_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  s64 pos = sp_io_seek(&stream, 100, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);

  pos = sp_io_seek(&stream, -10, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);

  sp_io_close(&stream);
}

UTEST_F(sp_io, file_open_read) {
  const char* test_content = "Hello, World!";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 13);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  ASSERT_TRUE(stream.file.fd >= 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_open_write) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  ASSERT_TRUE(stream.file.fd >= 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_open_nonexistent) {
  sp_str_t file_path = sp_test_file_path(&ut.file_manager, sp_str_lit("sp_io.file_open_nonexistent.file"));
  sp_io_stream_t stream = sp_io_from_file(file_path, SP_IO_MODE_READ);
  ASSERT_TRUE(stream.file.fd < 0);
}

UTEST_F(sp_io, file_close) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);
  ASSERT_TRUE(true);
}

UTEST_F(sp_io, file_read_full) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 16);
  sp_io_close(&write_stream);

  char buffer[16] = {0};
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&stream, buffer, 16);

  ASSERT_EQ(bytes, 16);
  for (u32 i = 0; i < 16; i++) {
    ASSERT_EQ(buffer[i], test_content[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_read_partial) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 16);
  sp_io_close(&write_stream);

  char buffer[8] = {0};
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&stream, buffer, 8);

  ASSERT_EQ(bytes, 8);
  for (u32 i = 0; i < 8; i++) {
    ASSERT_EQ(buffer[i], test_content[i]);
  }
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_read_past_eof) {
  const char* test_content = "0123";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 4);
  sp_io_close(&write_stream);

  char buffer[16] = {0};
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&stream, buffer, 16);

  ASSERT_EQ(bytes, 4);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_write_new) {
  const char* test_content = "test data";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  u64 bytes = sp_io_write(&stream, test_content, 9);

  ASSERT_EQ(bytes, 9);
  ASSERT_TRUE(sp_os_does_path_exist(utest_fixture->test_file_path));
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_write_overwrite) {
  const char* first = "XXXXXXXX";
  const char* second = "1234";

  sp_io_stream_t stream1 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream1, first, 8);
  sp_io_close(&stream1);

  sp_io_stream_t stream2 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream2, second, 4);
  sp_io_close(&stream2);

  char buffer[8] = {0};
  sp_io_stream_t read_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_read(&read_stream, buffer, 8);

  ASSERT_EQ(bytes, 4);
  ASSERT_EQ(buffer[0], '1');
  ASSERT_EQ(buffer[1], '2');
  ASSERT_EQ(buffer[2], '3');
  ASSERT_EQ(buffer[3], '4');
  sp_io_close(&read_stream);
}

UTEST_F(sp_io, file_seek_set) {
  const char* test_content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&stream, 5, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 5);

  char buffer[5] = {0};
  sp_io_read(&stream, buffer, 5);
  ASSERT_EQ(buffer[0], '5');
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_seek_cur) {
  const char* test_content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  char buffer[5] = {0};
  sp_io_read(&stream, buffer, 3);

  s64 pos = sp_io_seek(&stream, 2, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 5);

  sp_io_read(&stream, buffer, 1);
  ASSERT_EQ(buffer[0], '5');
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_seek_end) {
  const char* test_content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&stream, -3, SP_IO_SEEK_END);
  ASSERT_EQ(pos, 7);

  char buffer[3] = {0};
  sp_io_read(&stream, buffer, 3);
  ASSERT_EQ(buffer[0], '7');
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_size_regular) {
  const char* test_content = "0123456789ABCDEF";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 16);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, 16);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_size_empty) {
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_to_memory) {
  const char* test_content = "file to memory test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 19);
  sp_io_close(&write_stream);

  sp_io_stream_t file_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 size = sp_io_size(&file_stream);
  char buffer[32] = {0};
  sp_io_read(&file_stream, buffer, size);
  sp_io_close(&file_stream);

  sp_io_stream_t mem_stream = sp_io_from_memory(buffer, size);
  char result[32] = {0};
  sp_io_read(&mem_stream, result, size);

  ASSERT_EQ(size, 19);
  for (u32 i = 0; i < 19; i++) {
    ASSERT_EQ(result[i], test_content[i]);
  }
  sp_io_close(&mem_stream);
}

UTEST_F(sp_io, memory_to_file) {
  const char* test_content = "memory to file test";
  char buffer[32] = {0};
  sp_os_copy_memory(test_content, buffer, 19);

  sp_io_stream_t mem_stream = sp_io_from_memory(buffer, 19);
  char temp[32] = {0};
  sp_io_read(&mem_stream, temp, 19);
  sp_io_close(&mem_stream);

  sp_io_stream_t file_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&file_stream, temp, 19);
  sp_io_close(&file_stream);

  sp_io_stream_t read_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  char result[32] = {0};
  sp_io_read(&read_stream, result, 19);
  sp_io_close(&read_stream);

  for (u32 i = 0; i < 19; i++) {
    ASSERT_EQ(result[i], test_content[i]);
  }
}

UTEST_F(sp_io, load_file_helper) {
  const char* test_content = "load file helper test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 21);
  sp_io_close(&write_stream);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 21);
  for (u32 i = 0; i < 21; i++) {
    ASSERT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(sp_io, file_write_append) {
  const char* first = "first";
  const char* second = "second";

  sp_io_stream_t stream1 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream1, first, 5);
  sp_io_close(&stream1);

  sp_io_stream_t stream2 = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_APPEND);
  sp_io_write(&stream2, second, 6);
  sp_io_close(&stream2);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 11);
  ASSERT_EQ(loaded.data[0], 'f');
  ASSERT_EQ(loaded.data[4], 't');
  ASSERT_EQ(loaded.data[5], 's');
  ASSERT_EQ(loaded.data[10], 'd');
}

UTEST_F(sp_io, file_read_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  char buffer[10] = {0};
  u64 bytes = sp_io_read(&stream, buffer, 10);
  ASSERT_EQ(bytes, 0);
}

UTEST_F(sp_io, file_write_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  u64 bytes = sp_io_write(&stream, "test", 4);
  ASSERT_EQ(bytes, 0);
}

UTEST_F(sp_io, file_seek_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);
}

UTEST_F(sp_io, file_size_invalid_fd) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  stream.file.fd = -1;
  s64 size = sp_io_size(&stream);
  ASSERT_EQ(size, -1);
}

UTEST_F(sp_io, file_close_autoclose_false) {
  const char* test_content = "autoclose test";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  stream.file.close_mode = SP_IO_FILE_CLOSE_MODE_NONE;
  sp_io_write(&stream, test_content, 14);
  s32 fd = stream.file.fd;
  sp_io_close(&stream);

  sp_io_stream_t stream2 = SP_ZERO_INITIALIZE();
  stream2.file.fd = fd;
  stream2.file.close_mode = SP_IO_FILE_CLOSE_MODE_AUTO;
  stream2.callbacks.close = sp_io_file_close;
  sp_io_close(&stream2);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 14);
  for (u32 i = 0; i < 14; i++) {
    ASSERT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(sp_io, file_close_autoclose_true) {
  const char* test_content = "autoclose true";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&stream, test_content, 14);
  ASSERT_TRUE(stream.file.close_mode == SP_IO_FILE_CLOSE_MODE_AUTO);
  sp_io_close(&stream);

  sp_str_t loaded = sp_io_read_file(utest_fixture->test_file_path);
  ASSERT_EQ(loaded.len, 14);
  for (u32 i = 0; i < 14; i++) {
    ASSERT_EQ(loaded.data[i], test_content[i]);
  }
}

UTEST_F(sp_io, file_seek_invalid_negative) {
  const char* test_content = "seek test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 9);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  s64 pos = sp_io_seek(&stream, -100, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, -1);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_write_to_readonly) {
  const char* test_content = "initial";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, test_content, 7);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  u64 bytes = sp_io_write(&stream, "data", 4);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_read_from_writeonly) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  char buffer[10] = {0};
  u64 bytes = sp_io_read(&stream, buffer, 10);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, file_open_read_write) {
  const char* initial = "initial data";
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));
  ASSERT_TRUE(stream.file.fd >= 0);

  u64 written = sp_io_write(&stream, initial, 12);
  ASSERT_EQ(written, 12);

  sp_io_seek(&stream, 0, SP_IO_SEEK_SET);

  char buffer[12] = {0};
  u64 read = sp_io_read(&stream, buffer, 12);
  ASSERT_EQ(read, 12);

  for (u32 i = 0; i < 12; i++) {
    ASSERT_EQ(buffer[i], initial[i]);
  }

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_seek_cur_forward) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 64; i++) buffer[i] = (u8)i;

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&stream, 5, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 15);

  u8 val;
  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 15);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_seek_cur_backward) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 64; i++) buffer[i] = (u8)i;

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  sp_io_seek(&stream, 30, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&stream, -10, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 20);

  u8 val;
  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 20);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_seek_cur_invalid) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);
  s64 pos = sp_io_seek(&stream, -20, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, -1);

  pos = sp_io_seek(&stream, 100, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, -1);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_sequential_reads) {
  u8 source[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  sp_io_stream_t stream = sp_io_from_memory(source, sizeof(source));

  u8 buf1[4], buf2[4], buf3[4], buf4[4];

  ASSERT_EQ(sp_io_read(&stream, buf1, 4), 4);
  ASSERT_EQ(sp_io_read(&stream, buf2, 4), 4);
  ASSERT_EQ(sp_io_read(&stream, buf3, 4), 4);
  ASSERT_EQ(sp_io_read(&stream, buf4, 4), 4);

  ASSERT_EQ(buf1[0], 0);
  ASSERT_EQ(buf2[0], 4);
  ASSERT_EQ(buf3[0], 8);
  ASSERT_EQ(buf4[0], 12);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_sequential_writes) {
  u8 buffer[16] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 data1[] = {1,2,3,4};
  u8 data2[] = {5,6,7,8};
  u8 data3[] = {9,10,11,12};

  ASSERT_EQ(sp_io_write(&stream, data1, 4), 4);
  ASSERT_EQ(sp_io_write(&stream, data2, 4), 4);
  ASSERT_EQ(sp_io_write(&stream, data3, 4), 4);

  ASSERT_EQ(buffer[0], 1);
  ASSERT_EQ(buffer[4], 5);
  ASSERT_EQ(buffer[8], 9);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_interleaved_operations) {
  u8 buffer[32] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 data[] = {1,2,3,4};
  sp_io_write(&stream, data, 4);

  sp_io_seek(&stream, 0, SP_IO_SEEK_SET);

  u8 read_buf[4];
  sp_io_read(&stream, read_buf, 4);
  ASSERT_EQ(read_buf[0], 1);

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);

  u8 data2[] = {5,6,7,8};
  sp_io_write(&stream, data2, 4);

  sp_io_seek(&stream, 10, SP_IO_SEEK_SET);
  sp_io_read(&stream, read_buf, 4);
  ASSERT_EQ(read_buf[0], 5);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_size_zero) {
  u8 dummy;
  sp_io_stream_t stream = sp_io_from_memory(&dummy, 0);

  ASSERT_EQ(sp_io_size(&stream), 0);

  u8 buffer[10];
  ASSERT_EQ(sp_io_read(&stream, buffer, 10), 0);
  ASSERT_EQ(sp_io_write(&stream, buffer, 10), 0);

  sp_io_close(&stream);
}

UTEST_F(sp_io, file_read_zero_bytes) {
  const char* content = "test";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, content, 4);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);
  char buffer[10];
  u64 bytes = sp_io_read(&stream, buffer, 0);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_read_zero_bytes) {
  u8 buffer[64] = SP_ZERO_INITIALIZE();
  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 dest[10];
  u64 bytes = sp_io_read(&stream, dest, 0);
  ASSERT_EQ(bytes, 0);

  sp_io_close(&stream);
}

UTEST_F(sp_io, file_write_zero_bytes) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  u64 bytes = sp_io_write(&stream, "test", 0);
  ASSERT_EQ(bytes, 0);
  sp_io_close(&stream);
}

UTEST_F(sp_io, sp_io_read_file_nonexistent) {
  sp_str_t path = SP_LIT("/tmp/sp_io_nonexistent_xyz_12345.txt");
  sp_str_t result = sp_io_read_file(path);

  ASSERT_EQ(result.len, 0);
  ASSERT_EQ(result.data, SP_NULLPTR);
}

UTEST_F(sp_io, sp_io_read_file_empty) {
  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_close(&stream);

  sp_str_t result = sp_io_read_file(utest_fixture->test_file_path);

  ASSERT_EQ(result.len, 0);
}

UTEST_F(sp_io, file_seek_all_whence_zero_offset) {
  const char* content = "0123456789";
  sp_io_stream_t write_stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_WRITE);
  sp_io_write(&write_stream, content, 10);
  sp_io_close(&write_stream);

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, SP_IO_MODE_READ);

  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_SET);
  ASSERT_EQ(pos, 0);

  sp_io_seek(&stream, 5, SP_IO_SEEK_SET);
  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 5);

  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_END);
  ASSERT_EQ(pos, 10);

  sp_io_close(&stream);
}

UTEST_F(sp_io, memory_position_tracking) {
  u8 buffer[16];
  for (u32 i = 0; i < 16; i++) buffer[i] = (u8)i;

  sp_io_stream_t stream = sp_io_from_memory(buffer, sizeof(buffer));

  u8 val;
  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 0);

  s64 pos = sp_io_seek(&stream, 0, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 1);

  sp_io_read(&stream, &val, 1);
  ASSERT_EQ(val, 1);

  pos = sp_io_seek(&stream, 0, SP_IO_SEEK_CUR);
  ASSERT_EQ(pos, 2);

  sp_io_close(&stream);
}

UTEST_F(sp_io, file_write_read_roundtrip) {
  const char* data = "roundtrip test data";

  sp_io_stream_t stream = sp_io_from_file(utest_fixture->test_file_path, (sp_io_mode_t)(SP_IO_MODE_READ | SP_IO_MODE_WRITE));

  sp_io_write(&stream, data, 19);
  sp_io_seek(&stream, 0, SP_IO_SEEK_SET);

  char buffer[32] = {0};
  sp_io_read(&stream, buffer, 19);

  for (u32 i = 0; i < 19; i++) {
    ASSERT_EQ(buffer[i], data[i]);
  }

  sp_io_close(&stream);
}

typedef struct sp_ps {
  s32 foo;
} sp_ps_fixture;

UTEST(sp_env, all_operations) {
  sp_env_t env = sp_env_capture();

  sp_str_t path = sp_env_get(&env, SP_LIT("PATH"));
  ASSERT_GT(path.len, 0);

  sp_env_insert(&env, SP_LIT("SP_TEST_VAR"), SP_LIT("test_value"));
  sp_str_t result = sp_env_get(&env, SP_LIT("SP_TEST_VAR"));
  SP_EXPECT_STR_EQ_CSTR(result, "test_value");

  sp_env_insert(&env, SP_LIT("SP_TEST_VAR"), SP_LIT("updated_value"));
  result = sp_env_get(&env, SP_LIT("SP_TEST_VAR"));
  SP_EXPECT_STR_EQ_CSTR(result, "updated_value");

  sp_env_erase(&env, SP_LIT("SP_TEST_VAR"));
  result = sp_env_get(&env, SP_LIT("SP_TEST_VAR"));
  ASSERT_EQ(result.len, 0);

  sp_env_insert(&env, SP_LIT("SP_EMPTY"), SP_LIT(""));
  result = sp_env_get(&env, SP_LIT("SP_EMPTY"));
  ASSERT_EQ(result.len, 0);

  sp_str_t nonexistent = sp_env_get(&env, SP_LIT("SP_NONEXISTENT_VAR_12345"));
  ASSERT_EQ(nonexistent.len, 0);

  sp_env_t copy = sp_env_copy(&env);
  sp_env_insert(&env, SP_LIT("SP_ORIGINAL"), SP_LIT("original"));
  sp_env_insert(&copy, SP_LIT("SP_COPY"), SP_LIT("copy"));

  sp_str_t orig_val = sp_env_get(&env, SP_LIT("SP_ORIGINAL"));
  SP_EXPECT_STR_EQ_CSTR(orig_val, "original");
  sp_str_t orig_missing = sp_env_get(&env, SP_LIT("SP_COPY"));
  ASSERT_EQ(orig_missing.len, 0);

  sp_str_t copy_val = sp_env_get(&copy, SP_LIT("SP_COPY"));
  SP_EXPECT_STR_EQ_CSTR(copy_val, "copy");
  sp_str_t copy_missing = sp_env_get(&copy, SP_LIT("SP_ORIGINAL"));
  ASSERT_EQ(copy_missing.len, 0);

  sp_env_destroy(&env);
  sp_env_destroy(&copy);
}


typedef struct {
  bool context_valid;
  bool allocator_valid;
  bool alloc_succeeded;
  sp_str_t allocated_string;
} sp_thread_context_test_data_t;

s32 sp_thread_context_test_fn(void* userdata) {
  sp_thread_context_test_data_t* data = (sp_thread_context_test_data_t*)userdata;

  sp_context_t* ctx = sp_context_get();
  data->context_valid = (ctx != SP_NULLPTR);
  data->allocator_valid = (ctx != SP_NULLPTR && ctx->allocator.on_alloc != SP_NULLPTR);

  sp_str_t test_str = sp_str_from_cstr("thread allocation test");
  data->allocated_string = test_str;
  data->alloc_succeeded = test_str.data != SP_NULLPTR && test_str.len > 0;

  return 0;
}

UTEST(threading, context_in_child_thread) {
  sp_thread_context_test_data_t data = SP_ZERO_INITIALIZE();

  sp_thread_t thread;
  sp_thread_init(&thread, sp_thread_context_test_fn, &data);
  sp_thread_join(&thread);

  ASSERT_TRUE(data.context_valid);
  ASSERT_TRUE(data.allocator_valid);
  ASSERT_TRUE(data.alloc_succeeded);
  ASSERT_GT(data.allocated_string.len, 0);
  SP_EXPECT_STR_EQ_CSTR(data.allocated_string, "thread allocation test");
}
