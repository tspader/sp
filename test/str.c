#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

UTEST(cstr, all_variations) {
  const c8* original = "Hello World";
  c8* copy = sp_cstr_copy_a(sp_mem_get_scratch(), original);
  ASSERT_TRUE(sp_cstr_equal(copy, original));
  ASSERT_NE(copy, original);

  c8* partial = sp_cstr_copy_n_a(sp_mem_get_scratch(), original, 5);
  ASSERT_TRUE(sp_cstr_equal(partial, "Hello"));

  const c8* empty = "";
  c8* empty_copy = sp_cstr_copy_a(sp_mem_get_scratch(), empty);
  ASSERT_TRUE(sp_cstr_equal(empty_copy, ""));

  c8* null_copy = sp_cstr_copy_a(sp_mem_get_scratch(), SP_NULLPTR);
  ASSERT_EQ(null_copy[0], '\0');
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
  sp_cstr_copy_to_n(source, 5, partial_buffer, 10);
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

UTEST(str, conversion_functions) {
  sp_str_t str = sp_str_lit("Hello World");
  c8* cstr = sp_str_to_cstr_a(sp_mem_get_scratch(), str);
  ASSERT_TRUE(sp_cstr_equal(cstr, "Hello World"));

  sp_str_t empty = sp_str_lit("");
  c8* empty_cstr = sp_str_to_cstr_a(sp_mem_get_scratch(), empty);
  ASSERT_TRUE(sp_cstr_equal(empty_cstr, ""));
}

UTEST(str, string_copy_operations) {
  sp_str_t original = sp_str_lit("Hello World");
  sp_str_t copy = sp_str_copy_a(sp_mem_get_scratch(), original);
  ASSERT_EQ(copy.len, original.len);
  ASSERT_TRUE(sp_str_equal(copy, original));
  ASSERT_NE(copy.data, original.data);

  sp_str_t from_cstr = sp_str_from_cstr_a(sp_mem_get_scratch(), "Test String");
  ASSERT_EQ(from_cstr.len, 11);
  SP_EXPECT_STR_EQ_CSTR(from_cstr, "Test String");

  sp_str_t partial = sp_str_from_cstr_n_a(sp_mem_get_scratch(), "Hello World", 5);
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

  sp_str_t str2 = sp_str_lit("World");
  ASSERT_EQ(str2.len, 5);
  SP_EXPECT_STR_EQ_CSTR(str2, "World");

  const c8* cstr = "Dynamic";
  sp_str_t str3 = sp_str_view(cstr);
  ASSERT_EQ(str3.len, 7);
  SP_EXPECT_STR_EQ_CSTR(str3, "Dynamic");

  sp_str_t allocated = sp_str_alloc_a(sp_mem_get_scratch(), 100);
  ASSERT_EQ(allocated.len, 0);
  ASSERT_NE(allocated.data, SP_NULLPTR);
}

UTEST(str, string_comparison) {
  sp_str_t str1 = sp_str_lit("Hello");
  sp_str_t str2 = sp_str_lit("Hello");
  sp_str_t str3 = sp_str_lit("World");
  sp_str_t str4 = sp_str_lit("Hell");

  ASSERT_TRUE(sp_str_equal(str1, str2));
  ASSERT_FALSE(sp_str_equal(str1, str3));
  ASSERT_FALSE(sp_str_equal(str1, str4));

  SP_EXPECT_STR_EQ_CSTR(str1, "Hello");
  ASSERT_FALSE(sp_str_equal_cstr(str1, "World"));
  ASSERT_FALSE(sp_str_equal_cstr(str1, "Hell"));

  sp_str_t empty1 = sp_str_lit("");
  sp_str_t empty2 = sp_str_lit("");
  ASSERT_TRUE(sp_str_equal(empty1, empty2));
  SP_EXPECT_STR_EQ_CSTR(empty1, "");

  sp_str_t long_str = sp_str_lit("Hello World!");
  ASSERT_FALSE(sp_str_equal(str1, long_str));
}

UTEST(str, substrings) {
  sp_str_t str = sp_str_lit("Jerry Garcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_sub(str, 0, 5), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_sub(str, 6, 6), "Garcia");
}

UTEST(str, sorting_tests) {
  sp_str_t strings[] = {
    sp_str_lit("zebra"),
    sp_str_lit("apple"),
    sp_str_lit("banana"),
    sp_str_lit("aardvark"),
    sp_str_lit("zoo")
  };

  sp_os_qsort(strings, 5, sizeof(sp_str_t), sp_str_sort_kernel_alphabetical);

  SP_EXPECT_STR_EQ_CSTR(strings[0], "aardvark");
  SP_EXPECT_STR_EQ_CSTR(strings[1], "apple");
  SP_EXPECT_STR_EQ_CSTR(strings[2], "banana");
  SP_EXPECT_STR_EQ_CSTR(strings[3], "zebra");
  SP_EXPECT_STR_EQ_CSTR(strings[4], "zoo");

  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("a"), sp_str_lit("b")), SP_QSORT_A_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("b"), sp_str_lit("a")), SP_QSORT_B_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("same"), sp_str_lit("same")), SP_QSORT_EQUAL);

  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("ab"), sp_str_lit("abc")), SP_QSORT_A_FIRST);
  ASSERT_EQ(sp_str_compare_alphabetical(sp_str_lit("abc"), sp_str_lit("ab")), SP_QSORT_B_FIRST);
}

sp_str_t sp_test_map_band_member(sp_str_map_context_t* context) {
  return sp_str_concat_a(sp_mem_get_scratch(), context->str, sp_str_lit(" is in the band"));
}

UTEST(str, map_reduce) {
  sp_str_t band [] = {
    sp_str_lit("jerry"), sp_str_lit("bobby"), sp_str_lit("phil")
  };
  sp_da(sp_str_t) result = sp_str_map_a(sp_mem_get_scratch(), &band[0], SP_CARR_LEN(band), SP_NULLPTR, sp_test_map_band_member);
  SP_EXPECT_STR_EQ_CSTR(result[0], "jerry is in the band");
  SP_EXPECT_STR_EQ_CSTR(result[1], "bobby is in the band");
  SP_EXPECT_STR_EQ_CSTR(result[2], "phil is in the band");

  sp_str_t joined = sp_str_join_n_a(sp_mem_get_scratch(), band, SP_CARR_LEN(band), sp_str_lit(" and "));
  SP_EXPECT_STR_EQ_CSTR(joined, "jerry and bobby and phil");

  u32 len = 3;
  sp_da(sp_str_t) clipped = sp_str_map_a(sp_mem_get_scratch(), &band[0], SP_CARR_LEN(band), &len, sp_str_map_kernel_prefix);
  SP_EXPECT_STR_EQ_CSTR(clipped[0], "jer");
  SP_EXPECT_STR_EQ_CSTR(clipped[1], "bob");
  SP_EXPECT_STR_EQ_CSTR(clipped[2], "phi");
}

UTEST(str, valid_and_at) {
  sp_str_t valid = sp_str_lit("Hello");
  sp_str_t invalid = {.data = SP_NULLPTR, .len = 5};
  sp_str_t empty = sp_str_lit("");

  ASSERT_TRUE(sp_str_valid(valid));
  ASSERT_FALSE(sp_str_valid(invalid));
  ASSERT_TRUE(sp_str_valid(empty));

  sp_str_t str = sp_str_lit("Hello");
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
  sp_str_t single = sp_str_lit("X");
  ASSERT_EQ(sp_str_back(single), 'X');
}

UTEST(str, to_upper_and_replace) {
  {
    sp_str_t lowercase = sp_str_lit("hello world!");
    sp_str_t uppercase = sp_str_to_upper_a(sp_mem_get_scratch(), lowercase);
    SP_EXPECT_STR_EQ_CSTR(uppercase, "HELLO WORLD!");
  };

  {
    sp_str_t lower = sp_str_lit("caf\xC3\xA9 123 {[]}");
    SP_EXPECT_STR_EQ_CSTR(sp_str_to_upper_a(sp_mem_get_scratch(), lower), "CAF\xC3\xA9 123 {[]}");
  }

  {
    sp_str_t upper = sp_str_lit("CAF\xC3\xA9 123 {[]}");
    SP_EXPECT_STR_EQ_CSTR(sp_str_to_lower_a(sp_mem_get_scratch(), upper), "caf\xC3\xA9 123 {[]}");
  }

  sp_str_t mixed = sp_str_lit("HeLLo WoRLd!");
  sp_str_t upper_mixed = sp_str_to_upper_a(sp_mem_get_scratch(), mixed);
  SP_EXPECT_STR_EQ_CSTR(upper_mixed, "HELLO WORLD!");

  sp_str_t original = sp_str_lit("hello world");
  sp_str_t replaced = sp_str_replace_c8_a(sp_mem_get_scratch(), original, 'l', 'X');
  SP_EXPECT_STR_EQ_CSTR(replaced, "heXXo worXd");

  sp_str_t no_match = sp_str_replace_c8_a(sp_mem_get_scratch(), original, 'z', 'X');
  SP_EXPECT_STR_EQ_CSTR(no_match, "hello world");

  // all same char
  SP_EXPECT_STR_EQ_CSTR(sp_str_replace_c8_a(sp_mem_get_scratch(), sp_str_lit("aaa"), 'a', 'b'), "bbb");

  // empty string
  SP_EXPECT_STR_EQ_CSTR(sp_str_replace_c8_a(sp_mem_get_scratch(), sp_str_lit(""), 'a', 'b'), "");

  // same from/to (no-op)
  SP_EXPECT_STR_EQ_CSTR(sp_str_replace_c8_a(sp_mem_get_scratch(), sp_str_lit("abc"), 'a', 'a'), "abc");

  // single char string
  SP_EXPECT_STR_EQ_CSTR(sp_str_replace_c8_a(sp_mem_get_scratch(), sp_str_lit("x"), 'x', 'y'), "y");

  // first and last positions
  SP_EXPECT_STR_EQ_CSTR(sp_str_replace_c8_a(sp_mem_get_scratch(), sp_str_lit("/path/to/file/"), '/', '\\'), "\\path\\to\\file\\");
}

UTEST(str, ends_with) {
  sp_str_t str = sp_str_lit("hello world");
  ASSERT_TRUE(sp_str_ends_with(str, sp_str_lit("world")));
  ASSERT_FALSE(sp_str_ends_with(str, sp_str_lit("hello")));
  ASSERT_TRUE(sp_str_ends_with(str, sp_str_lit("")));
  ASSERT_TRUE(sp_str_ends_with(str, sp_str_lit("d")));
}

UTEST(str, concat) {
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat_a(sp_mem_get_scratch(), sp_str_lit("Jerry"), sp_str_lit("Garcia")), "JerryGarcia");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat_a(sp_mem_get_scratch(), sp_str_lit("Jerry"), sp_str_lit("")), "Jerry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_concat_a(sp_mem_get_scratch(), sp_str_lit(""), sp_str_lit("Jerry")), "Jerry");
}

UTEST(str, join_operations) {
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_a(sp_mem_get_scratch(), sp_str_lit("hello"), sp_str_lit("world"), sp_str_lit(" - ")), "hello - world");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_a(sp_mem_get_scratch(), sp_str_lit("hello"), sp_str_lit("world"), sp_str_lit("")), "helloworld");

  const c8* strings[] = {"apple", "banana", "cherry"};
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n_a(sp_mem_get_scratch(), strings, 3, sp_str_lit(", ")), "apple, banana, cherry");
  SP_EXPECT_STR_EQ_CSTR(sp_str_join_cstr_n_a(sp_mem_get_scratch(), strings, 1, sp_str_lit(", ")), "apple");
  ASSERT_EQ(sp_str_join_cstr_n_a(sp_mem_get_scratch(), strings, 0, sp_str_lit(", ")).len, 0);
}

UTEST(str_kernel, map_trim) {
  sp_str_t strings[] = {
    sp_str_lit("  hello  "),
    sp_str_lit("\tworld\n"),
    sp_str_lit("  \t\n\r  "),
    sp_str_lit("no_trim"),
  };

  sp_da(sp_str_t) results = sp_str_map_a(sp_mem_get_scratch(), strings, 4, NULL, sp_str_map_kernel_trim);

  ASSERT_EQ(sp_da_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "hello");
  SP_EXPECT_STR_EQ_CSTR(results[1], "world");
  SP_EXPECT_STR_EQ_CSTR(results[2], "");
  SP_EXPECT_STR_EQ_CSTR(results[3], "no_trim");
}

UTEST(str_kernel, map_case_transform) {
  sp_str_t strings[] = {
    sp_str_lit("Hello World"),
    sp_str_lit("ALREADY UPPER"),
    sp_str_lit("already lower"),
    sp_str_lit("MiXeD cAsE"),
  };

  sp_da(sp_str_t) results = sp_str_map_a(sp_mem_get_scratch(), strings, 4, NULL, sp_str_map_kernel_to_upper);
  ASSERT_EQ(sp_da_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "HELLO WORLD");
  SP_EXPECT_STR_EQ_CSTR(results[1], "ALREADY UPPER");
  SP_EXPECT_STR_EQ_CSTR(results[2], "ALREADY LOWER");
  SP_EXPECT_STR_EQ_CSTR(results[3], "MIXED CASE");

  results = sp_str_map_a(sp_mem_get_scratch(), strings, 4, NULL, sp_str_map_kernel_to_lower);
  ASSERT_EQ(sp_da_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "hello world");
  SP_EXPECT_STR_EQ_CSTR(results[1], "already upper");
  SP_EXPECT_STR_EQ_CSTR(results[2], "already lower");
  SP_EXPECT_STR_EQ_CSTR(results[3], "mixed case");

  sp_str_t strings2[] = {
    sp_str_lit("hello world"),
    sp_str_lit("the quick brown fox"),
    sp_str_lit("SHOUTING TEXT"),
    sp_str_lit("123 numbers first"),
  };

  results = sp_str_map_a(sp_mem_get_scratch(), strings2, 4, NULL, sp_str_map_kernel_pascal_case);
  ASSERT_EQ(sp_da_size(results), 4);
  SP_EXPECT_STR_EQ_CSTR(results[0], "Hello World");
  SP_EXPECT_STR_EQ_CSTR(results[1], "The Quick Brown Fox");
  SP_EXPECT_STR_EQ_CSTR(results[2], "Shouting Text");
  SP_EXPECT_STR_EQ_CSTR(results[3], "123 Numbers First");
}


UTEST(str, trim) {
  SP_EXPECT_STR_EQ_CSTR(sp_str_trim(sp_str_lit("  hello  ")), "hello");
  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("\thello\t")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("\nhello\n")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("  \t\nhello\n\t  ")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("")), sp_str_lit(""));
  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("   ")), sp_str_lit(""));
  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("\t\n\r")), sp_str_lit(""));

  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("hello")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("  hello world  ")), sp_str_lit("hello world"));
  SP_EXPECT_STR_EQ(sp_str_trim(sp_str_lit("\ttab\tseparated\t")), sp_str_lit("tab\tseparated"));
}

UTEST(str, trim_right) {
  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("hello  ")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("hello\t")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("hello\n")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("hello\t\n  ")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("  hello")), sp_str_lit("  hello"));
  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("\thello")), sp_str_lit("\thello"));

  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("")), sp_str_lit(""));
  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("   ")), sp_str_lit(""));

  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("hello")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_trim_right(sp_str_lit("hello world  ")), sp_str_lit("hello world"));
}

UTEST(str, strip_left) {
  SP_EXPECT_STR_EQ(sp_str_strip_left(sp_str_lit("sp_foo_t"), sp_str_lit("sp")), sp_str_lit("_foo_t"));
  SP_EXPECT_STR_EQ(sp_str_strip_left(sp_str_lit("___hello___"), sp_str_lit("_")), sp_str_lit("__hello___"));

  SP_EXPECT_STR_EQ(sp_str_strip_left(sp_str_lit("hello"), sp_str_lit("xyz")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_strip_left(sp_str_lit("hello"), sp_str_lit("")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_strip_left(sp_str_lit(""), sp_str_lit("abc")), sp_str_lit(""));
}

UTEST(str, strip_right) {
  SP_EXPECT_STR_EQ(sp_str_strip_right(sp_str_lit("foo_t_sp"), sp_str_lit("sp")), sp_str_lit("foo_t_"));
  SP_EXPECT_STR_EQ(sp_str_strip_right(sp_str_lit("___hello___"), sp_str_lit("_")), sp_str_lit("___hello__"));

  SP_EXPECT_STR_EQ(sp_str_strip_right(sp_str_lit("hello"), sp_str_lit("xyz")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_strip_right(sp_str_lit("hello"), sp_str_lit("")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_strip_right(sp_str_lit(""), sp_str_lit("abc")), sp_str_lit(""));
}

UTEST(str, strip) {
  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("sp_foo_t_sp"), sp_str_lit("sp")), sp_str_lit("_foo_t_"));
  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("___hello___"), sp_str_lit("_")), sp_str_lit("__hello__"));
  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("xyzabcxyz"), sp_str_lit("xyz")), sp_str_lit("abc"));

  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("sp_foo_t"), sp_str_lit("sp")), sp_str_lit("_foo_t"));
  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("foo_t_sp"), sp_str_lit("sp")), sp_str_lit("foo_t_"));

  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("hello"), sp_str_lit("xyz")), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit("hello"), sp_str_lit("")), sp_str_lit("hello"));
  SP_EXPECT_STR_EQ(sp_str_strip(sp_str_lit(""), sp_str_lit("abc")), sp_str_lit(""));
}

UTEST(str, split_c8) {
  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit("hello,world,test"), ',');
    ASSERT_EQ(sp_da_size(parts), 3);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "hello");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "world");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "test");
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit("/home/user/file.txt"), '/');
    ASSERT_EQ(sp_da_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "home");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "user");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "file.txt");
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit("a,,b"), ',');
    ASSERT_EQ(sp_da_size(parts), 3);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "a");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "b");
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit("a,,,b"), ',');
    ASSERT_EQ(sp_da_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "a");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "b");
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit("hello"), ',');
    ASSERT_EQ(sp_da_size(parts), 1);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "hello");
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit(""), ',');
    EXPECT_EQ(parts, SP_NULLPTR);
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit(",hello,world,"), ',');
    ASSERT_EQ(sp_da_size(parts), 4);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "");
    SP_EXPECT_STR_EQ_CSTR(parts[1], "hello");
    SP_EXPECT_STR_EQ_CSTR(parts[2], "world");
    SP_EXPECT_STR_EQ_CSTR(parts[3], "");
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit(","), ',');
    ASSERT_EQ(sp_da_size(parts), 2);
    SP_EXPECT_STR_EQ(parts[0], sp_str_lit(""));
    SP_EXPECT_STR_EQ(parts[1], sp_str_lit(""));
  }

  {
    sp_da(sp_str_t) parts = sp_str_split_c8_a(sp_mem_get_scratch(), sp_str_lit("x"), ',');
    ASSERT_EQ(sp_da_size(parts), 1);
    SP_EXPECT_STR_EQ_CSTR(parts[0], "x");
  }
}

UTEST(str, prefix_suffix) {
  sp_str_t str = sp_str_lit("jerry");

  SP_EXPECT_STR_EQ_CSTR(sp_str_suffix(str, 3), "rry");
}

UTEST(str, cleave_c8) {
  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit("hello,world"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "hello");
    SP_EXPECT_STR_EQ_CSTR(result.second, "world");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit("key=value"), '=');
    SP_EXPECT_STR_EQ_CSTR(result.first, "key");
    SP_EXPECT_STR_EQ_CSTR(result.second, "value");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit("no_delimiter"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "no_delimiter");
    SP_EXPECT_STR_EQ(result.second, sp_str_lit(""));
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit("a,b,c"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "a");
    SP_EXPECT_STR_EQ_CSTR(result.second, "b,c");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit(",trailing"), ',');
    SP_EXPECT_STR_EQ(result.first, sp_str_lit(""));
    SP_EXPECT_STR_EQ_CSTR(result.second, "trailing");
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit("leading,"), ',');
    SP_EXPECT_STR_EQ_CSTR(result.first, "leading");
    SP_EXPECT_STR_EQ(result.second, sp_str_lit(""));
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit(""), ',');
    SP_EXPECT_STR_EQ(result.first, sp_str_lit(""));
    SP_EXPECT_STR_EQ(result.second, sp_str_lit(""));
  }

  {
    sp_str_pair_t result = sp_str_cleave_c8(sp_str_lit(","), ',');
    SP_EXPECT_STR_EQ(result.first, sp_str_lit(""));
    SP_EXPECT_STR_EQ(result.second, sp_str_lit(""));
  }
}

UTEST(str, pad) {
  SP_EXPECT_STR_EQ(sp_str_pad_a(sp_mem_get_scratch(), sp_str_lit("hello"), 10), sp_str_lit("hello     "));
  SP_EXPECT_STR_EQ(sp_str_pad_a(sp_mem_get_scratch(), sp_str_lit("hi"), 5), sp_str_lit("hi   "));

  SP_EXPECT_STR_EQ(sp_str_pad_a(sp_mem_get_scratch(), sp_str_lit("hello world"), 5), sp_str_lit("hello world"));

  SP_EXPECT_STR_EQ(sp_str_pad_a(sp_mem_get_scratch(), sp_str_lit("hello"), 5), sp_str_lit("hello"));

  SP_EXPECT_STR_EQ(sp_str_pad_a(sp_mem_get_scratch(), sp_str_lit(""), 5), sp_str_lit("     "));

  SP_EXPECT_STR_EQ(sp_str_pad_a(sp_mem_get_scratch(), sp_str_lit("hello"), 0), sp_str_lit("hello"));
}

UTEST(str, pad_to_longest) {
  {
    sp_str_t strings[] = {
      sp_str_lit("hi"),
      sp_str_lit("hello"),
      sp_str_lit("world!")
    };
    sp_da(sp_str_t) padded = sp_str_pad_to_longest_a(sp_mem_get_scratch(), strings, 3);
    ASSERT_EQ(sp_da_size(padded), 3);
    SP_EXPECT_STR_EQ(padded[0], sp_str_lit("hi    "));
    SP_EXPECT_STR_EQ(padded[1], sp_str_lit("hello "));
    SP_EXPECT_STR_EQ(padded[2], sp_str_lit("world!"));
  }

  {
    sp_str_t strings[] = {
      sp_str_lit("aaa"),
      sp_str_lit("bbb"),
      sp_str_lit("ccc")
    };
    sp_da(sp_str_t) padded = sp_str_pad_to_longest_a(sp_mem_get_scratch(), strings, 3);
    ASSERT_EQ(sp_da_size(padded), 3);
    SP_EXPECT_STR_EQ(padded[0], sp_str_lit("aaa"));
    SP_EXPECT_STR_EQ(padded[1], sp_str_lit("bbb"));
    SP_EXPECT_STR_EQ(padded[2], sp_str_lit("ccc"));
  }

  {
    sp_str_t strings[] = {
      sp_str_lit("hello")
    };
    sp_da(sp_str_t) padded = sp_str_pad_to_longest_a(sp_mem_get_scratch(), strings, 1);
    ASSERT_EQ(sp_da_size(padded), 1);
    SP_EXPECT_STR_EQ(padded[0], sp_str_lit("hello"));
  }

  {
    sp_str_t strings[] = {
      sp_str_lit(""),
      sp_str_lit("hello"),
      sp_str_lit("")
    };
    sp_da(sp_str_t) padded = sp_str_pad_to_longest_a(sp_mem_get_scratch(), strings, 3);
    ASSERT_EQ(sp_da_size(padded), 3);
    SP_EXPECT_STR_EQ(padded[0], sp_str_lit("     "));
    SP_EXPECT_STR_EQ(padded[1], sp_str_lit("hello"));
    SP_EXPECT_STR_EQ(padded[2], sp_str_lit("     "));
  }
}

UTEST(str, starts_with) {
  ASSERT_TRUE(sp_str_starts_with(sp_str_lit("hello world"), sp_str_lit("hello")));
  ASSERT_TRUE(sp_str_starts_with(sp_str_lit("hello"), sp_str_lit("h")));
  ASSERT_FALSE(sp_str_starts_with(sp_str_lit("hello"), sp_str_lit("world")));

  ASSERT_TRUE(sp_str_starts_with(sp_str_lit("hello"), sp_str_lit("hello")));

  ASSERT_FALSE(sp_str_starts_with(sp_str_lit("hi"), sp_str_lit("hello")));

  ASSERT_TRUE(sp_str_starts_with(sp_str_lit("hello"), sp_str_lit("")));
  ASSERT_TRUE(sp_str_starts_with(sp_str_lit(""), sp_str_lit("")));
  ASSERT_FALSE(sp_str_starts_with(sp_str_lit(""), sp_str_lit("hello")));

  ASSERT_TRUE(sp_str_starts_with(sp_str_lit("/home/user/file.txt"), sp_str_lit("/home")));
  ASSERT_TRUE(sp_str_starts_with(sp_str_lit("/home/user/file.txt"), sp_str_lit("/home/user")));
  ASSERT_FALSE(sp_str_starts_with(sp_str_lit("/home/user/file.txt"), sp_str_lit("/usr")));

  ASSERT_FALSE(sp_str_starts_with(sp_str_lit("Hello"), sp_str_lit("hello")));
  ASSERT_FALSE(sp_str_starts_with(sp_str_lit("hello"), sp_str_lit("HELLO")));
}

UTEST(str, substring_searching) {
  ASSERT_TRUE(sp_str_contains(sp_str_lit("hello world"), sp_str_lit("world")));
  ASSERT_TRUE(sp_str_contains(sp_str_lit("hello world"), sp_str_lit("lo w")));
  ASSERT_TRUE(sp_str_contains(sp_str_lit("banana"), sp_str_lit("ana")));
  ASSERT_FALSE(sp_str_contains(sp_str_lit("hello"), sp_str_lit("world")));

  ASSERT_TRUE(sp_str_contains(sp_str_lit("hello"), sp_str_lit("hello")));

  ASSERT_FALSE(sp_str_contains(sp_str_lit("hi"), sp_str_lit("hello")));

  ASSERT_TRUE(sp_str_contains(sp_str_lit("hello"), sp_str_lit("")));
  ASSERT_TRUE(sp_str_contains(sp_str_lit(""), sp_str_lit("")));
  ASSERT_FALSE(sp_str_contains(sp_str_lit(""), sp_str_lit("hello")));

  ASSERT_TRUE(sp_str_contains(sp_str_lit("hello world"), sp_str_lit("hello")));
  ASSERT_TRUE(sp_str_contains(sp_str_lit("hello world"), sp_str_lit("world")));

  ASSERT_FALSE(sp_str_contains(sp_str_lit("Hello World"), sp_str_lit("hello")));
  ASSERT_FALSE(sp_str_contains(sp_str_lit("hello world"), sp_str_lit("WORLD")));

  ASSERT_TRUE(sp_str_contains(sp_str_lit("aaaa"), sp_str_lit("aa")));
  ASSERT_TRUE(sp_str_contains(sp_str_lit("abababab"), sp_str_lit("abab")));

  // partial prefix: "la" appears before "lap"
  ASSERT_TRUE(sp_str_contains(sp_str_lit("la_ap_lap"), sp_str_lit("lap")));

  // single char
  ASSERT_TRUE(sp_str_contains(sp_str_lit("abc"), sp_str_lit("b")));
  ASSERT_FALSE(sp_str_contains(sp_str_lit("abc"), sp_str_lit("z")));
}

typedef struct {
  sp_str_t str;
  c8 needle;
  s32 expected;
} sp_str_find_c8_case_t;

UTEST(str, char_find_c8) {
  sp_str_find_c8_case_t cases[] = {
    { sp_str_lit("hello world"), 'h', 0 },
    { sp_str_lit("hello world"), 'o', 4 },
    { sp_str_lit("hello world"), 'd', 10 },
    { sp_str_lit("banana"), 'a', 1 },
    { sp_str_lit("banana"), 'n', 2 },
    { sp_str_lit("/home/user/file.txt"), '/', 0 },
    { sp_str_lit("/home/user/file.txt"), '.', 15 },
    { sp_str_lit(""), 'x', -1 },
    { sp_str_lit("hello"), 'z', -1 },
    { sp_str_lit("x"), 'x', 0 },
  };

  SP_CARR_FOR(cases, i) {
    ASSERT_EQ(sp_str_find_c8(cases[i].str, cases[i].needle), cases[i].expected);
  }
}

UTEST(str, char_find_c8_reverse) {
  sp_str_find_c8_case_t cases[] = {
    { sp_str_lit("hello world"), 'h', 0 },
    { sp_str_lit("hello world"), 'o', 7 },
    { sp_str_lit("hello world"), 'd', 10 },
    { sp_str_lit("banana"), 'a', 5 },
    { sp_str_lit("banana"), 'n', 4 },
    { sp_str_lit("/home/user/file.txt"), '/', 10 },
    { sp_str_lit("/home/user/file.txt"), '.', 15 },
    { sp_str_lit(""), 'x', -1 },
    { sp_str_lit("hello"), 'z', -1 },
    { sp_str_lit("x"), 'x', 0 },
  };

  SP_CARR_FOR(cases, i) {
    ASSERT_EQ(sp_str_find_c8_reverse(cases[i].str, cases[i].needle), cases[i].expected);
  }
}

UTEST(str, view_creation) {
  {
    const c8* cstr = "hello world";
    sp_str_t view = sp_str_view(cstr);
    ASSERT_EQ(view.len, 11);
    ASSERT_EQ(view.data, cstr);
    SP_EXPECT_STR_EQ(view, sp_str_lit("hello world"));
  }

  {
    const c8* cstr = "";
    sp_str_t view = sp_str_view(cstr);
    ASSERT_EQ(view.len, 0);
    ASSERT_EQ(view.data, cstr);
    SP_EXPECT_STR_EQ(view, sp_str_lit(""));
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
    sp_str_t str = sp_str_from_cstr_a(sp_mem_get_scratch(), cstr);
    ASSERT_EQ(str.len, 11);
    SP_EXPECT_STR_EQ(str, sp_str_view(cstr));
    ASSERT_NE(str.data, cstr);
  }

  {
    sp_str_t str = sp_str_from_cstr_a(sp_mem_get_scratch(), "");
    ASSERT_EQ(str.len, 0);
    SP_EXPECT_STR_EQ(str, sp_str_lit(""));
  }

  {
    sp_str_t str = sp_str_from_cstr_a(sp_mem_get_scratch(), SP_NULLPTR);
    ASSERT_EQ(str.len, 0);
    ASSERT_EQ(str.data, SP_NULLPTR);
  }

  {
    c8 buffer[] = "mutable";
    sp_str_t str = sp_str_from_cstr_a(sp_mem_get_scratch(), buffer);
    buffer[0] = 'M';
    ASSERT_EQ(str.data[0], 'm');
  }

  {
    sp_str_t str = sp_str_from_cstr_n_a(sp_mem_get_scratch(), "hello world", 5);
    ASSERT_EQ(str.len, 5);
    SP_EXPECT_STR_EQ(str, sp_str_lit("hello"));
  }
}

UTEST(str, truncate_longer_than_limit) {
  sp_str_t str = sp_str_lit("hello world");
  sp_str_t result = sp_str_truncate_a(sp_mem_get_scratch(), str, 8, sp_str_lit("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "hello...");
}

UTEST(str, truncate_shorter_than_limit) {
  sp_str_t str = sp_str_lit("hi");
  sp_str_t result = sp_str_truncate_a(sp_mem_get_scratch(), str, 10, sp_str_lit("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "hi");
}

UTEST(str, truncate_exact_limit) {
  sp_str_t str = sp_str_lit("exactly");
  sp_str_t result = sp_str_truncate_a(sp_mem_get_scratch(), str, 7, sp_str_lit("..."));
  SP_EXPECT_STR_EQ_CSTR(result, "exactly");
}

UTEST(str, truncate_zero_limit) {
  sp_str_t str = sp_str_lit("test");
  sp_str_t result = sp_str_truncate_a(sp_mem_get_scratch(), str, 0, sp_str_lit("..."));
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
  c8 a[] = {0x41};
  ASSERT_EQ(sp_utf8_decode(a), 0x41);

  c8 zero[] = {0x00};
  ASSERT_EQ(sp_utf8_decode(zero), 0x00);

  c8 del[] = {0x7F};
  ASSERT_EQ(sp_utf8_decode(del), 0x7F);
}

UTEST(utf8, decode_2byte) {
  c8 cent[] = {(c8)0xC2, (c8)0xA2};
  ASSERT_EQ(sp_utf8_decode(cent), 0xA2);

  c8 edge[] = {(c8)0xDF, (c8)0xBF};
  ASSERT_EQ(sp_utf8_decode(edge), 0x7FF);
}

UTEST(utf8, decode_3byte) {
  c8 euro[] = {(c8)0xE2, (c8)0x82, (c8)0xAC};
  ASSERT_EQ(sp_utf8_decode(euro), 0x20AC);

  c8 cjk[] = {(c8)0xE4, (c8)0xB8, (c8)0xAD};
  ASSERT_EQ(sp_utf8_decode(cjk), 0x4E2D);
}

UTEST(utf8, decode_4byte) {
  c8 emoji[] = {(c8)0xF0, (c8)0x9F, (c8)0x98, (c8)0x80};
  ASSERT_EQ(sp_utf8_decode(emoji), 0x1F600);

  c8 max[] = {(c8)0xF4, (c8)0x8F, (c8)0xBF, (c8)0xBF};
  ASSERT_EQ(sp_utf8_decode(max), 0x10FFFF);
}

UTEST(utf8, encode_ascii) {
  c8 out[4];
  ASSERT_EQ(sp_utf8_encode(0x41, out), 1);
  ASSERT_EQ(out[0], 0x41);

  ASSERT_EQ(sp_utf8_encode(0x00, out), 1);
  ASSERT_EQ(out[0], 0x00);

  ASSERT_EQ(sp_utf8_encode(0x7F, out), 1);
  ASSERT_EQ(out[0], 0x7F);
}

UTEST(utf8, encode_2byte) {
  c8 out[4];
  ASSERT_EQ(sp_utf8_encode(0xA2, out), 2);
  ASSERT_EQ((u8)out[0], 0xC2);
  ASSERT_EQ((u8)out[1], 0xA2);

  ASSERT_EQ(sp_utf8_encode(0x7FF, out), 2);
  ASSERT_EQ((u8)out[0], 0xDF);
  ASSERT_EQ((u8)out[1], 0xBF);
}

UTEST(utf8, encode_3byte) {
  c8 out[4];
  ASSERT_EQ(sp_utf8_encode(0x20AC, out), 3);
  ASSERT_EQ((u8)out[0], 0xE2);
  ASSERT_EQ((u8)out[1], 0x82);
  ASSERT_EQ((u8)out[2], 0xAC);

  ASSERT_EQ(sp_utf8_encode(0x4E2D, out), 3);
  ASSERT_EQ((u8)out[0], 0xE4);
  ASSERT_EQ((u8)out[1], 0xB8);
  ASSERT_EQ((u8)out[2], 0xAD);
}

UTEST(utf8, encode_4byte) {
  c8 out[4];
  ASSERT_EQ(sp_utf8_encode(0x1F600, out), 4);
  ASSERT_EQ((u8)out[0], 0xF0);
  ASSERT_EQ((u8)out[1], 0x9F);
  ASSERT_EQ((u8)out[2], 0x98);
  ASSERT_EQ((u8)out[3], 0x80);

  ASSERT_EQ(sp_utf8_encode(0x10FFFF, out), 4);
  ASSERT_EQ((u8)out[0], 0xF4);
  ASSERT_EQ((u8)out[1], 0x8F);
  ASSERT_EQ((u8)out[2], 0xBF);
  ASSERT_EQ((u8)out[3], 0xBF);
}

UTEST(utf8, encode_invalid) {
  c8 out[4];
  ASSERT_EQ(sp_utf8_encode(0xD800, out), 3);
  ASSERT_EQ(sp_utf8_encode(0xDFFF, out), 3);
  ASSERT_EQ(sp_utf8_encode(0x110000, out), 0);
}

UTEST(utf8, roundtrip) {
  u32 codepoints[] = {0x00, 0x41, 0x7F, 0x80, 0x7FF, 0x800, 0xFFFF, 0x10000, 0x10FFFF};
  c8 out[4];
  for (u32 i = 0; i < sizeof(codepoints) / sizeof(codepoints[0]); i++) {
    u32 cp = codepoints[i];
    u8 len = sp_utf8_encode(cp, out);
    ASSERT_GT(len, 0);
    u32 decoded = sp_utf8_decode(out);
    ASSERT_EQ(decoded, cp);
  }
}

UTEST(utf8, iterator_ascii) {
  sp_str_t str = sp_str_lit("abc");
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
  sp_str_t str = sp_str_lit("a\xC2\xA2\xE2\x82\xAC\xF0\x9F\x98\x80z");
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
  sp_str_t str = sp_str_lit("a\xC2\xA2z");
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
  sp_str_t str = sp_str_lit("a\xC2\xA2z");
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
  sp_str_t str = sp_str_lit("a\xC2\xA2z");
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
  sp_str_t str = sp_str_lit("");
  sp_utf8_it_t it = sp_utf8_it(str);
  ASSERT_FALSE(sp_utf8_it_valid(&it));

  sp_utf8_it_t rit = sp_utf8_rit(str);
  ASSERT_FALSE(sp_utf8_it_valid(&rit));
}

UTEST(utf8, validate_valid) {
  ASSERT_TRUE(sp_utf8_validate(sp_str_lit("")));
  ASSERT_TRUE(sp_utf8_validate(sp_str_lit("hello")));
  ASSERT_TRUE(sp_utf8_validate(sp_str_lit("\xC2\xA2")));
  ASSERT_TRUE(sp_utf8_validate(sp_str_lit("\xE2\x82\xAC")));
  ASSERT_TRUE(sp_utf8_validate(sp_str_lit("\xF0\x9F\x98\x80")));
  ASSERT_TRUE(sp_utf8_validate(sp_str_lit("a\xC2\xA2\xE2\x82\xACz")));
}

UTEST(utf8, validate_invalid) {
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\x80")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xC2")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xE2\x82")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xF0\x9F\x98")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xC0\x80")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xED\xA0\x80")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xF4\x90\x80\x80")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xF0\x80\x80\x80")));
  ASSERT_FALSE(sp_utf8_validate(sp_str_lit("\xF0\x8F\xBF\xBF")));
}

UTEST(utf8, num_codepoints) {
  ASSERT_EQ(sp_utf8_num_codepoints(sp_str_lit("")), 0);
  ASSERT_EQ(sp_utf8_num_codepoints(sp_str_lit("abc")), 3);
  ASSERT_EQ(sp_utf8_num_codepoints(sp_str_lit("\xC2\xA2")), 1);
  ASSERT_EQ(sp_utf8_num_codepoints(sp_str_lit("a\xC2\xA2z")), 3);
  ASSERT_EQ(sp_utf8_num_codepoints(sp_str_lit("\xF0\x9F\x98\x80")), 1);
}

UTEST(utf8, builder_append) {
  sp_io_dyn_mem_writer_t builder = sp_zero;
  sp_io_dyn_mem_writer_init_a(sp_mem_get_scratch(), &builder);

  u32 codepoints[] = { 'a', 0xA2, 0x20AC, 0x1F600, 'z' };
  for (u32 i = 0; i < 5; i++) {
    c8 buf[4] = sp_zero;
    u8 len = sp_utf8_encode(codepoints[i], buf);
    sp_io_write_str(&builder.base, sp_str(buf, len), SP_NULLPTR);
  }

  sp_str_t result = sp_io_dyn_mem_writer_as_str(&builder);
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

#ifdef SP_WIN32
SP_PRIVATE sp_str_t sp_win32_utf16_to_utf8(const u16* utf16, s32 len);

UTEST(wstr, wide_string_conversion) {
  wchar_t wide_str[] = L"Hello";
  sp_str_t converted = sp_win32_utf16_to_utf8((const u16*)wide_str, 5);
  ASSERT_TRUE(sp_str_equal_cstr(converted, "Hello"));

  wchar_t empty[] = L"";
  sp_str_t empty_converted = sp_win32_utf16_to_utf8((const u16*)empty, 0);
  ASSERT_TRUE(sp_str_equal_cstr(empty_converted, ""));

  wchar_t special[] = L"Test 123!";
  sp_str_t special_converted = sp_win32_utf16_to_utf8((const u16*)special, 9);
  ASSERT_TRUE(sp_str_equal_cstr(special_converted, "Test 123!"));
}
#endif

