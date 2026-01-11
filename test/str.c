#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct str {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(str) {
  sp_test_file_manager_init(&ut.file_manager);
}

UTEST_F_TEARDOWN(str) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

UTEST(str_builder, basic_operations) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  ASSERT_EQ(builder.writer, SP_NULLPTR);

  sp_str_t test_str = SP_LIT("Hello");
  sp_str_builder_append(&builder, test_str);
  ASSERT_NE(builder.writer, SP_NULLPTR);
  EXPECT_GE(sp_io_writer_size(builder.writer), 5);

  sp_str_builder_append_cstr(&builder, " World");
  EXPECT_GE(sp_io_writer_size(builder.writer), 11);

  sp_str_builder_append_c8(&builder, '!');
  EXPECT_GE(sp_io_writer_size(builder.writer), 12);

  sp_str_t result = sp_str_builder_to_str(&builder);
  ASSERT_EQ(result.len, 12);
  SP_EXPECT_STR_EQ_CSTR(result, "Hello World!");

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_builder_append_cstr(&builder2, "Test");
  const c8* cstr_result = sp_str_to_cstr(sp_str_builder_to_str(&builder2));
  ASSERT_TRUE(sp_cstr_equal(cstr_result, "Test"));
}

UTEST(str_builder, growth_behavior) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_t long_str = SP_LIT("This is a much longer string that will trigger growth");
  sp_str_builder_append(&builder, long_str);
  EXPECT_GE(sp_io_writer_size(builder.writer), long_str.len);
  sp_str_t result = sp_str_builder_to_str(&builder);
  SP_EXPECT_STR_EQ(result, long_str);
}

UTEST(str_builder, edge_cases) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append(&builder, SP_LIT(""));
  EXPECT_GE(sp_io_writer_size(builder.writer), 0);

  sp_str_builder_append_cstr(&builder, "");
  EXPECT_GE(sp_io_writer_size(builder.writer), 0);

  sp_str_t null_str = {.len = 0, .data = SP_NULLPTR};
  sp_str_builder_append(&builder, null_str);
  EXPECT_GE(sp_io_writer_size(builder.writer), 0);

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_for(i, 100) {
    sp_str_builder_append_cstr(&builder2, "test ");
  }
  EXPECT_GE(sp_io_writer_size(builder2.writer), 500);
  sp_str_t result = sp_str_builder_to_str(&builder2);
  ASSERT_EQ(result.len, 500);
}

UTEST(str_builder, indent_operations) {
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

  sp_str_t result = sp_str_builder_to_str(&builder);
  ASSERT_GT(result.len, 10);

  sp_str_builder_t builder2 = SP_ZERO_INITIALIZE();
  sp_str_builder_dedent(&builder2);
  sp_str_builder_dedent(&builder2);
  sp_str_builder_append_cstr(&builder2, "no_crash");
  ASSERT_EQ(builder2.indent.level, 0);
}

UTEST(str_builder, format_append) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_builder_append_fmt(&builder, "Value: {}", SP_FMT_U32(123));
  sp_str_t result = sp_str_builder_to_str(&builder);
  ASSERT_GT(result.len, 0);
  ASSERT_NE(result.data, SP_NULLPTR);
}

UTEST(str_builder, fixed_mem_backend) {
  {
    c8 buffer[64] = SP_ZERO_INITIALIZE();
    sp_io_writer_t writer = sp_io_writer_from_mem(buffer, sizeof(buffer));
    sp_str_builder_t builder = sp_str_builder_from_writer(&writer);

    sp_str_builder_append_cstr(&builder, "Hello");
    sp_str_builder_append_c8(&builder, ' ');
    sp_str_builder_append(&builder, SP_LIT("World"));

    sp_str_t result = sp_str(buffer, writer.mem.pos);
    SP_EXPECT_STR_EQ_CSTR(result, "Hello World");
    ASSERT_TRUE(sp_cstr_equal(buffer, "Hello World"));
  }

  {
    c8 buffer[10] = SP_ZERO_INITIALIZE();
    sp_io_writer_t writer = sp_io_writer_from_mem(buffer, sizeof(buffer));
    sp_str_builder_t builder = sp_str_builder_from_writer(&writer);

    sp_str_builder_append_cstr(&builder, "Short");
    sp_str_t result = sp_str(buffer, writer.mem.pos);
    SP_EXPECT_STR_EQ_CSTR(result, "Short");
  }

  {
    c8 buffer[128] = SP_ZERO_INITIALIZE();
    sp_io_writer_t writer = sp_io_writer_from_mem(buffer, sizeof(buffer));
    sp_str_builder_t builder = sp_str_builder_from_writer(&writer);

    sp_str_builder_append_fmt(&builder, "Count: {}", SP_FMT_U32(42));
    sp_str_t result = sp_str(buffer, writer.mem.pos);
    SP_EXPECT_STR_EQ_CSTR(result, "Count: 42");
  }

  {
    c8 buffer[64] = SP_ZERO_INITIALIZE();
    sp_io_writer_t writer = sp_io_writer_from_mem(buffer, sizeof(buffer));
    sp_str_builder_t builder = sp_str_builder_from_writer(&writer);

    sp_str_builder_append_cstr(&builder, "line1");
    sp_str_builder_indent(&builder);
    sp_str_builder_new_line(&builder);
    sp_str_builder_append_cstr(&builder, "indented");

    sp_str_t result = sp_str(buffer, writer.mem.pos);
    SP_EXPECT_STR_EQ_CSTR(result, "line1\n  indented");
  }
}

UTEST(cstr, all_variations) {
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

UTEST(cstr, buffer_operations) {
  const c8* source = "Hello World";
  c8 buffer[20];
  sp_mem_zero(buffer, 20);
  sp_cstr_copy_to(source, buffer, 20);
  ASSERT_TRUE(sp_cstr_equal(buffer, source));

  c8 exact[12];
  sp_mem_zero(exact, 12);
  sp_cstr_copy_to(source, exact, 12);
  ASSERT_TRUE(sp_cstr_equal(exact, source));

  char small_buffer[6];
  sp_mem_zero(small_buffer, 6);
  sp_cstr_copy_to(source, small_buffer, 6);
  ASSERT_TRUE(sp_cstr_equal(small_buffer, "Hello"));

  c8 partial_buffer[10];
  sp_mem_zero(partial_buffer, 10);
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

UTEST(cstr, comparison_tests) {
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

UTEST(cstr, length_tests) {
  ASSERT_EQ(sp_cstr_len("Hello"), 5);
  ASSERT_EQ(sp_cstr_len("Hello World!"), 12);
  ASSERT_EQ(sp_cstr_len(""), 0);

  ASSERT_EQ(sp_cstr_len(SP_NULLPTR), 0);

  const c8 embedded[] = {'H', 'e', '\0', 'l', 'o', '\0'};
  ASSERT_EQ(sp_cstr_len(embedded), 2);
}

UTEST(wstr, wide_string_conversion) {
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

UTEST(str, conversion_functions) {
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

UTEST(str, string_copy_operations) {
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
  sp_mem_zero(buffer, 20);
  sp_str_copy_to(original, buffer, 20);
  ASSERT_TRUE(sp_mem_is_equal(buffer, original.data, original.len));

  c8 small_buffer[5];
  sp_mem_zero(small_buffer, 5);
  sp_str_copy_to(original, small_buffer, 5);
  ASSERT_TRUE(sp_mem_is_equal(small_buffer, "Hello", 5));
}

UTEST(str, string_creation) {
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

UTEST(str, string_comparison) {
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

UTEST(str, substrings) {
  sp_str_t str = SP_LIT("Jerry Garcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_sub(str, 0, 5), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_sub(str, 6, 6), "Garcia");
}

UTEST(str, sorting_tests) {
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

UTEST(str, map_reduce) {
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

UTEST(str, valid_and_at) {
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

UTEST(str, to_upper_and_replace) {
  {
    sp_str_t lowercase = SP_LIT("hello world!");
    sp_str_t uppercase = sp_str_to_upper(lowercase);
    SP_EXPECT_STR_EQ_CSTR(uppercase, "HELLO WORLD!");
  };

  {
    sp_str_t lower = sp_str_lit("caf\xC3\xA9 123 {[]}");
    SP_EXPECT_STR_EQ_CSTR(sp_str_to_upper(lower), "CAF\xC3\xA9 123 {[]}");
  }

  {
    sp_str_t upper = sp_str_lit("CAF\xC3\xA9 123 {[]}");
    SP_EXPECT_STR_EQ_CSTR(sp_str_to_lower(upper), "caf\xC3\xA9 123 {[]}");
  }

  sp_str_t mixed = SP_LIT("HeLLo WoRLd!");
  sp_str_t upper_mixed = sp_str_to_upper(mixed);
  SP_EXPECT_STR_EQ_CSTR(upper_mixed, "HELLO WORLD!");

  sp_str_t original = SP_LIT("hello world");
  sp_str_t replaced = sp_str_replace_c8(original, 'l', 'X');
  SP_EXPECT_STR_EQ_CSTR(replaced, "heXXo worXd");

  sp_str_t no_match = sp_str_replace_c8(original, 'z', 'X');
  SP_EXPECT_STR_EQ_CSTR(no_match, "hello world");
}

UTEST(str, ends_with) {
  sp_str_t str = SP_LIT("hello world");
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("world")));
  ASSERT_FALSE(sp_str_ends_with(str, SP_LIT("hello")));
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("")));
  ASSERT_TRUE(sp_str_ends_with(str, SP_LIT("d")));
}

UTEST(str, concat) {
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT("Jerry"), SP_LIT("Garcia")), "JerryGarcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT("Jerry"), SP_LIT("")), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat(SP_LIT(""), SP_LIT("Jerry")), "Jerry");
}

UTEST(str, join_operations) {
  SP_EXPECT_STR_EQ_CSTR(sp_str_join(SP_LIT("hello"), SP_LIT("world"), SP_LIT(" - ")), "hello - world");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join(SP_LIT("hello"), SP_LIT("world"), SP_LIT("")), "helloworld");

  const c8* strings[] = {"apple", "banana", "cherry"};
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n(strings, 3, SP_LIT(", ")), "apple, banana, cherry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n(strings, 1, SP_LIT(", ")), "apple");
  ASSERT_EQ(sp_str_join_cstr_n(strings, 0, SP_LIT(", ")).len, 0);
}

UTEST(str_kernel, map_trim) {
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

UTEST(str_kernel, map_case_transform) {
  sp_str_t strings[] = {
    SP_LIT("Hello World"),
    SP_LIT("ALREADY UPPER"),
    SP_LIT("already lower"),
    SP_LIT("MiXeD cAsE"),
  };

  sp_dyn_array(sp_str_t) results = sp_str_map(strings, 4, NULL, sp_str_map_kernel_to_upper);
  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "HELLO WORLD");
  SP_EXPECT_STR_EQ_CSTR(results[1], "ALREADY UPPER");
  SP_EXPECT_STR_EQ_CSTR(results[2], "ALREADY LOWER");
  SP_EXPECT_STR_EQ_CSTR(results[3], "MIXED CASE");

  results = sp_str_map(strings, 4, NULL, sp_str_map_kernel_to_lower);
  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "hello world");
  SP_EXPECT_STR_EQ_CSTR(results[1], "already upper");
  SP_EXPECT_STR_EQ_CSTR(results[2], "already lower");
  SP_EXPECT_STR_EQ_CSTR(results[3], "mixed case");

  sp_str_t strings2[] = {
    SP_LIT("hello world"),
    SP_LIT("the quick brown fox"),
    SP_LIT("SHOUTING TEXT"),
    SP_LIT("123 numbers first"),
  };

  results = sp_str_map(strings2, 4, NULL, sp_str_map_kernel_pascal_case);
  ASSERT_EQ(sp_dyn_array_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "Hello World");
  SP_EXPECT_STR_EQ_CSTR(results[1], "The Quick Brown Fox");
  SP_EXPECT_STR_EQ_CSTR(results[2], "Shouting Text");
  SP_EXPECT_STR_EQ_CSTR(results[3], "123 Numbers First");
}

UTEST(str_kernel, reduce_contains) {
  sp_str_t strings[] = {
    SP_LIT("apple"),
    SP_LIT("banana"),
    SP_LIT("cherry"),
    SP_LIT("date"),
  };

  ASSERT_TRUE(sp_str_contains_n(strings, 4, SP_LIT("a")));
  ASSERT_TRUE(sp_str_contains_n(strings, 4, SP_LIT("ana")));

  ASSERT_FALSE(sp_str_contains_n(strings, 4, SP_LIT("xyz")));

  ASSERT_FALSE(sp_str_contains_n(strings, 0, SP_LIT("apple")));
}

UTEST(str_kernel, reduce_count) {
  sp_str_t strings[] = {
    SP_LIT("hello world"),
    SP_LIT("hello hello"),
    SP_LIT("world"),
    SP_LIT("hello"),
  };

  ASSERT_EQ(sp_str_count_n(strings, 4, SP_LIT("hello")), 4);

  ASSERT_EQ(sp_str_count_n(strings, 4, SP_LIT("world")), 2);
}

UTEST(str_kernel, reduce_longest_shortest) {
  sp_str_t strings[] = {
    SP_LIT("short"),
    SP_LIT("medium length"),
    SP_LIT("x"),
    SP_LIT("this is the longest string here"),
    SP_LIT("tiny"),
  };

  sp_str_t longest = sp_str_find_longest_n(strings, 5);
  SP_EXPECT_STR_EQ_CSTR(longest, "this is the longest string here");

  sp_str_t shortest = sp_str_find_shortest_n(strings, 5);
  SP_EXPECT_STR_EQ_CSTR(shortest, "x");
}

UTEST(str, trim) {
  SP_EXPECT_STR_EQ_CSTR(sp_str_trim(SP_LIT("  hello  ")), "hello");
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\thello\t")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\nhello\n")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("  \t\nhello\n\t  ")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("")), SP_LIT(""));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("   ")), SP_LIT(""));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\t\n\r")), SP_LIT(""));

  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("hello")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("  hello world  ")), SP_LIT("hello world"));
  SP_EXPECT_STR_EQ(sp_str_trim(SP_LIT("\ttab\tseparated\t")), SP_LIT("tab\tseparated"));
}

UTEST(str, trim_right) {
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello  ")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello\t")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello\n")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello\t\n  ")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("  hello")), SP_LIT("  hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("\thello")), SP_LIT("\thello"));

  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("")), SP_LIT(""));
  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("   ")), SP_LIT(""));

  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim_right(SP_LIT("hello world  ")), SP_LIT("hello world"));
}

UTEST(str, strip_left) {
  SP_EXPECT_STR_EQ(sp_str_strip_left(SP_LIT("sp_foo_t"), SP_LIT("sp")), SP_LIT("_foo_t"));
  SP_EXPECT_STR_EQ(sp_str_strip_left(SP_LIT("___hello___"), SP_LIT("_")), SP_LIT("__hello___"));

  SP_EXPECT_STR_EQ(sp_str_strip_left(SP_LIT("hello"), SP_LIT("xyz")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_strip_left(SP_LIT("hello"), SP_LIT("")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_strip_left(SP_LIT(""), SP_LIT("abc")), SP_LIT(""));
}

UTEST(str, strip_right) {
  SP_EXPECT_STR_EQ(sp_str_strip_right(SP_LIT("foo_t_sp"), SP_LIT("sp")), SP_LIT("foo_t_"));
  SP_EXPECT_STR_EQ(sp_str_strip_right(SP_LIT("___hello___"), SP_LIT("_")), SP_LIT("___hello__"));

  SP_EXPECT_STR_EQ(sp_str_strip_right(SP_LIT("hello"), SP_LIT("xyz")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_strip_right(SP_LIT("hello"), SP_LIT("")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_strip_right(SP_LIT(""), SP_LIT("abc")), SP_LIT(""));
}

UTEST(str, strip) {
  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("sp_foo_t_sp"), SP_LIT("sp")), SP_LIT("_foo_t_"));
  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("___hello___"), SP_LIT("_")), SP_LIT("__hello__"));
  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("xyzabcxyz"), SP_LIT("xyz")), SP_LIT("abc"));

  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("sp_foo_t"), SP_LIT("sp")), SP_LIT("_foo_t"));
  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("foo_t_sp"), SP_LIT("sp")), SP_LIT("foo_t_"));

  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("hello"), SP_LIT("xyz")), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT("hello"), SP_LIT("")), SP_LIT("hello"));
  SP_EXPECT_STR_EQ(sp_str_strip(SP_LIT(""), SP_LIT("abc")), SP_LIT(""));
}

UTEST(str, split_c8) {
  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("hello,world,test"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 3);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "hello");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "world");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "test");
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("/home/user/file.txt"), '/');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "home");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "user");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "file.txt");
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("a,,b"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 3);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "a");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "b");
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("a,,,b"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "a");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "b");
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT("hello"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 1);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "hello");
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(""), ',');
    EXPECT_EQ(parts, SP_NULLPTR);
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(",hello,world,"), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "hello");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "world");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "");
  }

  {
    sp_dyn_array(sp_str_t) parts = sp_str_split_c8(SP_LIT(","), ',');
    ASSERT_EQ(sp_dyn_array_size(parts), 2);
    SP_EXPECT_STR_EQ(parts[0], SP_LIT(""));
    SP_EXPECT_STR_EQ(parts[1], SP_LIT(""));
  }
}

UTEST(str, prefix_suffix) {
  sp_str_t str = sp_str_lit("jerry");

  SP_EXPECT_STR_EQ_CSTR(sp_str_suffix(str, 3), "rry");
}

UTEST(str, cleave_c8) {
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

UTEST(str, pad) {
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello"), 10), SP_LIT("hello     "));
  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hi"), 5), SP_LIT("hi   "));

  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello world"), 5), SP_LIT("hello world"));

  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello"), 5), SP_LIT("hello"));

  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT(""), 5), SP_LIT("     "));

  SP_EXPECT_STR_EQ(sp_str_pad(SP_LIT("hello"), 0), SP_LIT("hello"));
}

UTEST(str, pad_to_longest) {
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

  {
    sp_str_t strings[] = {
      SP_LIT("hello")
    };
    sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(strings, 1);
    ASSERT_EQ(sp_dyn_array_size(padded), 1);
    SP_EXPECT_STR_EQ(padded[0], SP_LIT("hello"));
  }

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

UTEST(str, starts_with) {
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello world"), SP_LIT("hello")));
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("h")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("world")));

  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("hello")));

  ASSERT_FALSE(sp_str_starts_with(SP_LIT("hi"), SP_LIT("hello")));

  ASSERT_TRUE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("")));
  ASSERT_TRUE(sp_str_starts_with(SP_LIT(""), SP_LIT("")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT(""), SP_LIT("hello")));

  ASSERT_TRUE(sp_str_starts_with(SP_LIT("/home/user/file.txt"), SP_LIT("/home")));
  ASSERT_TRUE(sp_str_starts_with(SP_LIT("/home/user/file.txt"), SP_LIT("/home/user")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("/home/user/file.txt"), SP_LIT("/usr")));

  ASSERT_FALSE(sp_str_starts_with(SP_LIT("Hello"), SP_LIT("hello")));
  ASSERT_FALSE(sp_str_starts_with(SP_LIT("hello"), SP_LIT("HELLO")));
}

UTEST(str, substring_searching) {
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("world")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("lo w")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("banana"), SP_LIT("ana")));
  ASSERT_FALSE(sp_str_contains(SP_LIT("hello"), SP_LIT("world")));

  ASSERT_TRUE(sp_str_contains(SP_LIT("hello"), SP_LIT("hello")));

  ASSERT_FALSE(sp_str_contains(SP_LIT("hi"), SP_LIT("hello")));

  ASSERT_TRUE(sp_str_contains(SP_LIT("hello"), SP_LIT("")));
  ASSERT_TRUE(sp_str_contains(SP_LIT(""), SP_LIT("")));
  ASSERT_FALSE(sp_str_contains(SP_LIT(""), SP_LIT("hello")));

  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("hello")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("hello world"), SP_LIT("world")));

  ASSERT_FALSE(sp_str_contains(SP_LIT("Hello World"), SP_LIT("hello")));
  ASSERT_FALSE(sp_str_contains(SP_LIT("hello world"), SP_LIT("WORLD")));

  ASSERT_TRUE(sp_str_contains(SP_LIT("aaaa"), SP_LIT("aa")));
  ASSERT_TRUE(sp_str_contains(SP_LIT("abababab"), SP_LIT("abab")));
}

UTEST(str, view_creation) {
  {
    const c8* cstr = "hello world";
    sp_str_t view = sp_str_view(cstr);
    ASSERT_EQ(view.len, 11);
    ASSERT_EQ(view.data, cstr);
    SP_EXPECT_STR_EQ(view, SP_LIT("hello world"));
  }

  {
    const c8* cstr = "";
    sp_str_t view = sp_str_view(cstr);
    ASSERT_EQ(view.len, 0);
    ASSERT_EQ(view.data, cstr);
    SP_EXPECT_STR_EQ(view, SP_LIT(""));
  }

  {
    sp_str_t view = sp_str_view(SP_NULLPTR);
    ASSERT_EQ(view.len, 0);
    ASSERT_EQ(view.data, SP_NULLPTR);
  }

  {
    c8 buffer[] = "mutable";
    sp_str_t view = sp_str_view(buffer);
    buffer[0] = 'M';
    ASSERT_EQ(view.data[0], 'M');
  }
}

UTEST(sp_str_from_cstr, string_from_cstr) {


  {
    const c8* cstr = "hello world";
    sp_str_t str = sp_str_from_cstr(cstr);
    ASSERT_EQ(str.len, 11);
    SP_EXPECT_STR_EQ(str, sp_str_view(cstr));
    ASSERT_NE(str.data, cstr);
  }

  {
    sp_str_t str = sp_str_from_cstr("");
    ASSERT_EQ(str.len, 0);
    SP_EXPECT_STR_EQ(str, SP_LIT(""));
  }

  {
    sp_str_t str = sp_str_from_cstr(SP_NULLPTR);
    ASSERT_EQ(str.len, 0);
    ASSERT_NE(str.data, SP_NULLPTR);
  }

  {
    c8 buffer[] = "mutable";
    sp_str_t str = sp_str_from_cstr(buffer);
    buffer[0] = 'M';
    ASSERT_EQ(str.data[0], 'm');
  }

  {
    sp_str_t str = sp_str_from_cstr_sized("hello world", 5);
    ASSERT_EQ(str.len, 5);
    SP_EXPECT_STR_EQ(str, SP_LIT("hello"));
  }

  {
    sp_str_t str = sp_str_from_cstr_null(SP_NULLPTR);
    ASSERT_EQ(str.len, 0);
    ASSERT_NE(str.data, SP_NULLPTR);

    str = sp_str_from_cstr_null("hello");
    ASSERT_EQ(str.len, 5);
    SP_EXPECT_STR_EQ(str, SP_LIT("hello"));
  }
}

UTEST(str, truncate_longer_than_limit) {
  sp_str_t str = SP_LIT("hello world");
  sp_str_t result = sp_str_truncate(str, 8, SP_LIT("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "hello...");
}

UTEST(str, truncate_shorter_than_limit) {
  sp_str_t str = SP_LIT("hi");
  sp_str_t result = sp_str_truncate(str, 10, SP_LIT("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "hi");
}

UTEST(str, truncate_exact_limit) {
  sp_str_t str = SP_LIT("exactly");
  sp_str_t result = sp_str_truncate(str, 7, SP_LIT("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "exactly");
}

UTEST(str, truncate_zero_limit) {
  sp_str_t str = SP_LIT("test");
  sp_str_t result = sp_str_truncate(str, 0, SP_LIT("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "test");
}

UTEST(utf8, num_bytes_from_byte) {
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0x00), 1);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0x41), 1);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0x7F), 1);

  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xC2), 2);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xDF), 2);

  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xE0), 3);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xEF), 3);

  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xF0), 4);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xF4), 4);

  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0x80), 0);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xBF), 0);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xF8), 0);
  ASSERT_EQ(sp_utf8_num_bytes_from_byte(0xFF), 0);
}

UTEST(utf8, num_bytes_from_cp) {
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x00), 1);
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x41), 1);
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x7F), 1);

  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x80), 2);
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x7FF), 2);

  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x800), 3);
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0xFFFF), 3);

  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x10000), 4);
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x10FFFF), 4);

  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0x110000), 0);
  ASSERT_EQ(sp_utf8_num_bytes_from_codepoint(0xFFFFFFFF), 0);
}

UTEST(utf8, decode_ascii) {
  u8 a[] = {0x41};
  ASSERT_EQ(sp_utf8_decode(a), 0x41);

  u8 zero[] = {0x00};
  ASSERT_EQ(sp_utf8_decode(zero), 0x00);

  u8 del[] = {0x7F};
  ASSERT_EQ(sp_utf8_decode(del), 0x7F);
}

UTEST(utf8, decode_2byte) {
  u8 cent[] = {0xC2, 0xA2};
  ASSERT_EQ(sp_utf8_decode(cent), 0xA2);

  u8 edge[] = {0xDF, 0xBF};
  ASSERT_EQ(sp_utf8_decode(edge), 0x7FF);
}

UTEST(utf8, decode_3byte) {
  u8 euro[] = {0xE2, 0x82, 0xAC};
  ASSERT_EQ(sp_utf8_decode(euro), 0x20AC);

  u8 cjk[] = {0xE4, 0xB8, 0xAD};
  ASSERT_EQ(sp_utf8_decode(cjk), 0x4E2D);
}

UTEST(utf8, decode_4byte) {
  u8 emoji[] = {0xF0, 0x9F, 0x98, 0x80};
  ASSERT_EQ(sp_utf8_decode(emoji), 0x1F600);

  u8 max[] = {0xF4, 0x8F, 0xBF, 0xBF};
  ASSERT_EQ(sp_utf8_decode(max), 0x10FFFF);
}

UTEST(utf8, encode_ascii) {
  u8 out[4];
  ASSERT_EQ(sp_utf8_encode(0x41, out), 1);
  ASSERT_EQ(out[0], 0x41);

  ASSERT_EQ(sp_utf8_encode(0x00, out), 1);
  ASSERT_EQ(out[0], 0x00);

  ASSERT_EQ(sp_utf8_encode(0x7F, out), 1);
  ASSERT_EQ(out[0], 0x7F);
}

UTEST(utf8, encode_2byte) {
  u8 out[4];
  ASSERT_EQ(sp_utf8_encode(0xA2, out), 2);
  ASSERT_EQ(out[0], 0xC2);
  ASSERT_EQ(out[1], 0xA2);

  ASSERT_EQ(sp_utf8_encode(0x7FF, out), 2);
  ASSERT_EQ(out[0], 0xDF);
  ASSERT_EQ(out[1], 0xBF);
}

UTEST(utf8, encode_3byte) {
  u8 out[4];
  ASSERT_EQ(sp_utf8_encode(0x20AC, out), 3);
  ASSERT_EQ(out[0], 0xE2);
  ASSERT_EQ(out[1], 0x82);
  ASSERT_EQ(out[2], 0xAC);

  ASSERT_EQ(sp_utf8_encode(0x4E2D, out), 3);
  ASSERT_EQ(out[0], 0xE4);
  ASSERT_EQ(out[1], 0xB8);
  ASSERT_EQ(out[2], 0xAD);
}

UTEST(utf8, encode_4byte) {
  u8 out[4];
  ASSERT_EQ(sp_utf8_encode(0x1F600, out), 4);
  ASSERT_EQ(out[0], 0xF0);
  ASSERT_EQ(out[1], 0x9F);
  ASSERT_EQ(out[2], 0x98);
  ASSERT_EQ(out[3], 0x80);

  ASSERT_EQ(sp_utf8_encode(0x10FFFF, out), 4);
  ASSERT_EQ(out[0], 0xF4);
  ASSERT_EQ(out[1], 0x8F);
  ASSERT_EQ(out[2], 0xBF);
  ASSERT_EQ(out[3], 0xBF);
}

UTEST(utf8, encode_invalid) {
  u8 out[4];
  ASSERT_EQ(sp_utf8_encode(0xD800, out), 3);
  ASSERT_EQ(sp_utf8_encode(0xDFFF, out), 3);
  ASSERT_EQ(sp_utf8_encode(0x110000, out), 0);
}

UTEST(utf8, roundtrip) {
  u32 codepoints[] = {0x00, 0x41, 0x7F, 0x80, 0x7FF, 0x800, 0xFFFF, 0x10000, 0x10FFFF};
  u8 out[4];
  for (u32 i = 0; i < sizeof(codepoints) / sizeof(codepoints[0]); i++) {
    u32 cp = codepoints[i];
    u8 len = sp_utf8_encode(cp, out);
    ASSERT_GT(len, 0);
    u32 decoded = sp_utf8_decode(out);
    ASSERT_EQ(decoded, cp);
  }
}

UTEST(utf8, iterator_ascii) {
  sp_str_t str = SP_LIT("abc");
  sp_utf8_it_t it = sp_utf8_it(str);

  ASSERT_TRUE(sp_utf8_it_valid(&it));
  ASSERT_EQ(it.codepoint, 'a');
  ASSERT_EQ(it.codepoint_len, 1);
  ASSERT_EQ(it.index, 0);

  sp_utf8_it_next(&it);
  ASSERT_TRUE(sp_utf8_it_valid(&it));
  ASSERT_EQ(it.codepoint, 'b');

  sp_utf8_it_next(&it);
  ASSERT_TRUE(sp_utf8_it_valid(&it));
  ASSERT_EQ(it.codepoint, 'c');

  sp_utf8_it_next(&it);
  ASSERT_FALSE(sp_utf8_it_valid(&it));
}

UTEST(utf8, iterator_multibyte) {
  sp_str_t str = SP_LIT("a\xC2\xA2\xE2\x82\xAC\xF0\x9F\x98\x80z");
  sp_utf8_it_t it = sp_utf8_it(str);

  ASSERT_EQ(it.codepoint, 'a');
  ASSERT_EQ(it.codepoint_len, 1);

  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 0xA2);
  ASSERT_EQ(it.codepoint_len, 2);

  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 0x20AC);
  ASSERT_EQ(it.codepoint_len, 3);

  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 0x1F600);
  ASSERT_EQ(it.codepoint_len, 4);

  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 'z');
  ASSERT_EQ(it.codepoint_len, 1);

  sp_utf8_it_next(&it);
  ASSERT_FALSE(sp_utf8_it_valid(&it));
}

UTEST(utf8, iterator_for_macro) {
  sp_str_t str = SP_LIT("a\xC2\xA2z");
  u32 cps[3];
  u32 i = 0;

  sp_str_for_utf8(str, it) {
    cps[i++] = it.codepoint;
  }

  ASSERT_EQ(i, 3);
  ASSERT_EQ(cps[0], 'a');
  ASSERT_EQ(cps[1], 0xA2);
  ASSERT_EQ(cps[2], 'z');
}

UTEST(utf8, iterator_reverse) {
  sp_str_t str = SP_LIT("a\xC2\xA2z");
  sp_utf8_it_t it = sp_utf8_rit(str);

  ASSERT_TRUE(sp_utf8_it_valid(&it));
  ASSERT_EQ(it.codepoint, 'z');

  sp_utf8_it_prev(&it);
  ASSERT_TRUE(sp_utf8_it_valid(&it));
  ASSERT_EQ(it.codepoint, 0xA2);

  sp_utf8_it_prev(&it);
  ASSERT_TRUE(sp_utf8_it_valid(&it));
  ASSERT_EQ(it.codepoint, 'a');

  sp_utf8_it_prev(&it);
  ASSERT_FALSE(sp_utf8_it_valid(&it));
}

UTEST(utf8, iterator_reverse_for_macro) {
  sp_str_t str = SP_LIT("a\xC2\xA2z");
  u32 cps[3];
  u32 i = 0;

  sp_str_rfor_utf8(str, it) {
    cps[i++] = it.codepoint;
  }

  ASSERT_EQ(i, 3);
  ASSERT_EQ(cps[0], 'z');
  ASSERT_EQ(cps[1], 0xA2);
  ASSERT_EQ(cps[2], 'a');
}

UTEST(utf8, iterator_empty) {
  sp_str_t str = SP_LIT("");
  sp_utf8_it_t it = sp_utf8_it(str);
  ASSERT_FALSE(sp_utf8_it_valid(&it));

  sp_utf8_it_t rit = sp_utf8_rit(str);
  ASSERT_FALSE(sp_utf8_it_valid(&rit));
}

UTEST(utf8, validate_valid) {
  ASSERT_TRUE(sp_utf8_validate(SP_LIT("")));
  ASSERT_TRUE(sp_utf8_validate(SP_LIT("hello")));
  ASSERT_TRUE(sp_utf8_validate(SP_LIT("\xC2\xA2")));
  ASSERT_TRUE(sp_utf8_validate(SP_LIT("\xE2\x82\xAC")));
  ASSERT_TRUE(sp_utf8_validate(SP_LIT("\xF0\x9F\x98\x80")));
  ASSERT_TRUE(sp_utf8_validate(SP_LIT("a\xC2\xA2\xE2\x82\xACz")));
}

UTEST(utf8, validate_invalid) {
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\x80")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xC2")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xE2\x82")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xF0\x9F\x98")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xC0\x80")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xED\xA0\x80")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xF4\x90\x80\x80")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xF0\x80\x80\x80")));
  ASSERT_FALSE(sp_utf8_validate(SP_LIT("\xF0\x8F\xBF\xBF")));
}

UTEST(utf8, num_codepoints) {
  ASSERT_EQ(sp_utf8_num_codepoints(SP_LIT("")), 0);
  ASSERT_EQ(sp_utf8_num_codepoints(SP_LIT("abc")), 3);
  ASSERT_EQ(sp_utf8_num_codepoints(SP_LIT("\xC2\xA2")), 1);
  ASSERT_EQ(sp_utf8_num_codepoints(SP_LIT("a\xC2\xA2z")), 3);
  ASSERT_EQ(sp_utf8_num_codepoints(SP_LIT("\xF0\x9F\x98\x80")), 1);
}

UTEST(utf8, builder_append) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();

  sp_str_builder_append_utf8(&builder, 'a');
  sp_str_builder_append_utf8(&builder, 0xA2);
  sp_str_builder_append_utf8(&builder, 0x20AC);
  sp_str_builder_append_utf8(&builder, 0x1F600);
  sp_str_builder_append_utf8(&builder, 'z');

  sp_str_t result = sp_str_builder_to_str(&builder);
  ASSERT_EQ(result.len, 11);
  ASSERT_EQ(sp_utf8_num_codepoints(result), 5);

  sp_utf8_it_t it = sp_utf8_it(result);
  ASSERT_EQ(it.codepoint, 'a');
  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 0xA2);
  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 0x20AC);
  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 0x1F600);
  sp_utf8_it_next(&it);
  ASSERT_EQ(it.codepoint, 'z');
}

#ifdef SP_CPP
UTEST(string_cpp, path_concatenation_operator) {


  sp_str_t path1 = SP_LIT("home");
  sp_str_t path2 = SP_LIT("user");
  sp_str_t result = path1 / path2;

  ASSERT_EQ(result.len, 9);
  SP_EXPECT_STR_EQ_CSTR(result, "home/user");

  sp_str_t win_path1 = SP_LIT("C:\\Windows");
  sp_str_t win_path2 = SP_LIT("System32");
  sp_str_t win_result = win_path1 / win_path2;

  SP_EXPECT_STR_EQ_CSTR(win_result, "C:/Windows/System32");

  sp_str_t empty = SP_LIT("");
  sp_str_t filename = SP_LIT("file.txt");
  sp_str_t empty_result = empty / filename;

  SP_EXPECT_STR_EQ_CSTR(empty_result, "/file.txt");

  sp_str_t base = SP_LIT("root");
  sp_str_t dir = SP_LIT("subdir");
  sp_str_t file = SP_LIT("file.txt");
  sp_str_t chained = base / dir / file;

  SP_EXPECT_STR_EQ_CSTR(chained, "root/subdir/file.txt");

  sp_str_t path = SP_LIT("home");
  sp_str_t result_cstr = path / "documents";

  ASSERT_EQ(result_cstr.len, 14);
  SP_EXPECT_STR_EQ_CSTR(result_cstr, "home/documents");

  sp_str_t chained_cstr = base / "data" / "files";
  SP_EXPECT_STR_EQ_CSTR(chained_cstr, "root/data/files");
}
#endif
