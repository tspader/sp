Use test helper macros for string comparisons and assertions.

## Good
```c
UTEST(string_operations, trimming) {
    sp_str_t input = SP_LIT("  hello world  ");
    sp_str_t result = sp_str_trim(input);

    SP_EXPECT_STR_EQ_CSTR(result, "hello world");

    sp_str_t empty = SP_LIT("   \t\n  ");
    result = sp_str_trim(empty);
    SP_EXPECT_STR_EQ_CSTR(result, "");

    sp_str_t a = SP_LIT("test");
    sp_str_t b = SP_LIT("test");
    SP_EXPECT_STR_EQ(a, b);
}

UTEST(format_system, basic) {
    sp_str_t msg = sp_format("{} + {} = {}",
        SP_FMT_U32(10), SP_FMT_U32(20), SP_FMT_U32(30));

    SP_EXPECT_STR_EQ_CSTR(msg, "10 + 20 = 30");

    ASSERT_EQ(msg.len, 13);
    ASSERT_NE(msg.data, SP_NULLPTR);
}
```

## Bad
```c
void test_string_trimming() {
    char input[] = "  hello world  ";
    char* result = trim(input);

    if (strcmp(result, "hello world") != 0) {
        printf("Test failed: expected 'hello world', got '%s'\n", result);
        exit(1);
    }

    char empty[] = "   \t\n  ";
    result = trim(empty);
    assert(strlen(result) == 0);
}
```

# Tags
- api.strings.sp_str_t.copy
- api.strings.sp_str_t.comparison
- api.os.formatting
