// sp_str_alloc
// sp_str_capitalize_words
// sp_str_cleave_c8
// sp_str_copy
// sp_str_copy_to
// sp_str_find_longest_n
// sp_str_find_shortest_n
// sp_str_from_cstr
// sp_str_from_cstr_null
// sp_str_from_cstr_sized
// sp_str_join
// sp_str_join_cstr_n
// sp_str_join_n
// sp_str_to_lower
// sp_str_map
// sp_str_map_kernel_append
// sp_str_map_kernel_capitalize_words
// sp_str_map_kernel_pad
// sp_str_map_kernel_prepend
// sp_str_map_kernel_to_lower
// sp_str_map_kernel_to_upper
// sp_str_map_kernel_trim
// sp_str_null_terminate
// sp_str_pad
// sp_str_pad_to_longest
// sp_str_reduce
// sp_str_reduce_kernel_join
// sp_str_replace_c8
// sp_str_split_c8
// sp_str_strip
// sp_str_strip_left
// sp_str_strip_right
// sp_str_sub
// sp_str_sub_reverse
// sp_str_to_cstr
// sp_str_to_cstr_double_nt
// sp_str_to_lower
// sp_str_to_upper
// sp_str_trim
// sp_str_trim_left
// sp_str_trim_right
// sp_str_truncate

#define SP_APP
#include "sp.h"

#include "utest.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

//#define SP_LEAK_LOG_ONLY

#if defined(SP_LEAK_LOG_ONLY)
  #define SP_LEAK_VERIFY()
#else
  #define SP_LEAK_VERIFY() EXPECT_EQ(sp_mem_get_scratch_arena()->bytes_used, 0)
#endif

struct leak {
  sp_test_memory_tracker tracker;
  sp_mem_scratch_t marker;
  sp_str_t str;
};

UTEST_F_SETUP(leak) {
  ut.str = sp_str_lit("hello, world!");

  sp_mem_arena_clear(sp_mem_get_scratch_arena());

  #if defined(SP_LEAK_LOG_ONLY)
  ut.marker = sp_mem_begin_scratch();
  #endif
}

UTEST_F_TEARDOWN(leak) {
  #if defined(SP_LEAK_LOG_ONLY)
    sp_mem_arena_t* arena = sp_mem_get_scratch_arena();
    SP_LOG("{} bytes", SP_FMT_U32(arena->bytes_used));
    sp_mem_end_scratch(ut.marker);
  #endif
}

UTEST_F(leak, to_upper) {
  sp_str_to_upper(ut.str);
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_alloc) {
  sp_str_alloc(32);
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_copy) {
  sp_str_copy(ut.str);
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_from_cstr) {
  sp_str_from_cstr("test");
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_from_cstr_null) {
  sp_str_from_cstr_null(SP_NULLPTR);
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_from_cstr_sized) {
  sp_str_from_cstr_sized("test", 4);
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_concat) {
  sp_str_concat(ut.str, SP_LIT(" extra"));
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_join) {
  sp_str_join(ut.str, SP_LIT("test"), SP_LIT(" "));
  SP_LEAK_VERIFY();
}

UTEST_F(leak, sp_str_join_cstr_n) {
  const c8* strings[] = {"a", "b", "c"};
  sp_str_join_cstr_n(strings, 3, SP_LIT(","));
  SP_LEAK_VERIFY();
}

#if 0
UTEST_F(sp_test_leak, sp_str_join_n) {
  sp_str_t strs[] = {SP_LIT("a"), SP_LIT("b"), SP_LIT("c")};
  sp_str_t result = sp_str_join_n(strs, 3, SP_LIT(","));
}

UTEST_F(sp_test_leak, sp_str_null_terminate) {
  sp_str_t result = sp_str_null_terminate(ut.str);
}

UTEST_F(sp_test_leak, sp_str_to_cstr) {
  c8* result = sp_str_to_cstr(ut.str);
}

UTEST_F(sp_test_leak, sp_str_to_cstr_double_nt) {
  c8* result = sp_str_to_cstr_double_nt(ut.str);
}

UTEST_F(sp_test_leak, sp_str_replace_c8) {
  sp_str_t result = sp_str_replace_c8(ut.str, 'o', 'x');
}

UTEST_F(sp_test_leak, sp_str_pad) {
  sp_str_t result = sp_str_pad(ut.str, 5);
}

UTEST_F(sp_test_leak, sp_str_to_lower) {
  sp_str_t result = sp_str_to_lower(ut.str);
}

UTEST_F(sp_test_leak, sp_str_capitalize_words) {
  sp_str_t result = sp_str_capitalize_words(ut.str);
}

UTEST_F(sp_test_leak, sp_str_truncate) {
  sp_str_t result = sp_str_truncate(ut.str, 5, SP_LIT("..."));
}

UTEST_F(sp_test_leak, sp_str_map) {
  sp_str_t strs[] = {ut.str};
  sp_da(sp_str_t) result = sp_str_map(strs, 1, SP_NULLPTR, sp_str_map_kernel_to_upper);
}

UTEST_F(sp_test_leak, sp_str_map_kernel_append) {
  sp_str_t suffix = SP_LIT(" suffix");
  sp_str_map_context_t ctx = {.str = ut.str, .user_data = &suffix};
  sp_str_t result = sp_str_map_kernel_append(&ctx);
}

UTEST_F(sp_test_leak, sp_str_map_kernel_prepend) {
  sp_str_t prefix = SP_LIT("prefix ");
  sp_str_map_context_t ctx = {.str = ut.str, .user_data = &prefix};
  sp_str_t result = sp_str_map_kernel_prepend(&ctx);
}

UTEST_F(sp_test_leak, sp_str_map_kernel_pad) {
  u32 pad = 5;
  sp_str_map_context_t ctx = {.str = ut.str, .user_data = &pad};
  sp_str_t result = sp_str_map_kernel_pad(&ctx);
}

UTEST_F(sp_test_leak, sp_str_map_kernel_to_upper) {
  sp_str_map_context_t ctx = {.str = ut.str};
  sp_str_t result = sp_str_map_kernel_to_upper(&ctx);
}

UTEST_F(sp_test_leak, sp_str_map_kernel_to_lower) {
  sp_str_map_context_t ctx = {.str = ut.str};
  sp_str_t result = sp_str_map_kernel_to_lower(&ctx);
}

UTEST_F(sp_test_leak, sp_str_map_kernel_capitalize_words) {
  sp_str_map_context_t ctx = {.str = ut.str};
  sp_str_t result = sp_str_map_kernel_capitalize_words(&ctx);
}

UTEST_F(sp_test_leak, sp_str_reduce) {
  sp_str_t strs[] = {ut.str, SP_LIT(" more")};
  sp_str_builder_t joiner = SP_ZERO_INITIALIZE();
  sp_str_t result = sp_str_reduce(strs, 2, &joiner, sp_str_reduce_kernel_join);
}

UTEST_F(sp_test_leak, sp_str_reduce_kernel_join) {
  sp_str_t joiner = SP_LIT(", ");
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_reduce_context_t ctx = {.str = ut.str, .builder = builder, .user_data = &joiner};
  sp_str_reduce_kernel_join(&ctx);
}

UTEST_F(sp_test_leak, sp_str_split_c8) {
  sp_da(sp_str_t) result = sp_str_split_c8(ut.str, ',');
}
#endif
SP_TEST_MAIN()
