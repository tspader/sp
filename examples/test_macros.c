#define SP_IMPLEMENTATION
#include "../sp.h"

#define UTEST(suite, name) static void test_##suite##_##name(void)
#define RUN_UTEST(suite, name) test_##suite##_##name()
#define ASSERT_EQ(a, b) SP_ASSERT((a) == (b))
#define ASSERT_NE(a, b) SP_ASSERT((a) != (b))
#define SP_EXPECT_STR_EQ_CSTR(str, cstr) SP_ASSERT(sp_str_equal_cstr((str), (cstr)))
#define SP_EXPECT_STR_EQ(a, b) SP_ASSERT(sp_str_equal((a), (b)))

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

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

    if (msg.data) {
      sp_free((void*)msg.data);
    }
}

int main(void) {
  sp_example_init();
  RUN_UTEST(string_operations, trimming);
  RUN_UTEST(format_system, basic);
  sp_example_shutdown();
  return 0;
}
