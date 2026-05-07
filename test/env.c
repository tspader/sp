#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

struct sp_env {
  sp_mem_t mem;
};

UTEST_F_SETUP(sp_env) {
  ut.mem = sp_mem_os_new();
}

UTEST_F_TEARDOWN(sp_env) {
}

UTEST_F(sp_env, init_empty) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  EXPECT_FALSE(sp_env_contains(&env, sp_str_lit("PATH")));
  sp_env_destroy(&env);
}

UTEST_F(sp_env, insert_and_get) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("FOO"), sp_str_lit("bar"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("FOO")), "bar");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, insert_overwrites) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("KEY"), sp_str_lit("first"));
  sp_env_insert(&env, sp_str_lit("KEY"), sp_str_lit("second"));

  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("KEY")), "second");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, get_missing_returns_empty) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_str_t val = sp_env_get(&env, sp_str_lit("DOES_NOT_EXIST"));
  EXPECT_TRUE(sp_str_empty(val));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, contains) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  EXPECT_FALSE(sp_env_contains(&env, sp_str_lit("X")));
  sp_env_insert(&env, sp_str_lit("X"), sp_str_lit("y"));
  EXPECT_TRUE(sp_env_contains(&env, sp_str_lit("X")));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, erase) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("A"), sp_str_lit("1"));
  sp_env_insert(&env, sp_str_lit("B"), sp_str_lit("2"));
  EXPECT_EQ(sp_env_count(&env), (u32)2);

  sp_env_erase(&env, sp_str_lit("A"));
  EXPECT_EQ(sp_env_count(&env), (u32)1);
  EXPECT_FALSE(sp_env_contains(&env, sp_str_lit("A")));
  EXPECT_TRUE(sp_env_contains(&env, sp_str_lit("B")));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, erase_nonexistent) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("A"), sp_str_lit("1"));
  sp_env_erase(&env, sp_str_lit("NOPE"));
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, multiple_entries) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("A"), sp_str_lit("1"));
  sp_env_insert(&env, sp_str_lit("B"), sp_str_lit("2"));
  sp_env_insert(&env, sp_str_lit("C"), sp_str_lit("3"));

  EXPECT_EQ(sp_env_count(&env), (u32)3);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("A")), "1");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("B")), "2");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("C")), "3");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, copy_is_independent) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  sp_env_insert(&env, sp_str_lit("K"), sp_str_lit("V"));

  sp_env_t copy = sp_env_copy(ut.mem, &env);
  EXPECT_EQ(sp_env_count(&copy), (u32)1);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&copy, sp_str_lit("K")), "V");

  sp_env_insert(&copy, sp_str_lit("NEW"), sp_str_lit("val"));
  EXPECT_EQ(sp_env_count(&copy), (u32)2);
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
  sp_env_destroy(&copy);
}

UTEST_F(sp_env, empty_value) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("EMPTY"), sp_str_lit(""));
  EXPECT_TRUE(sp_env_contains(&env, sp_str_lit("EMPTY")));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("EMPTY")), "");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, value_with_equals) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert(&env, sp_str_lit("DSN"), sp_str_lit("host=localhost;port=5432"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("DSN")), "host=localhost;port=5432");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, capture_has_path) {
  SKIP_ON_WASM()
  SKIP_ON_FREESTANDING()
  sp_env_t env = sp_env_capture(ut.mem);

  EXPECT_TRUE(sp_env_count(&env) > 0);

  #if defined(SP_WIN32)
    sp_str_t path_key = sp_str_lit("Path");
  #else
    sp_str_t path_key = sp_str_lit("PATH");
  #endif

  EXPECT_TRUE(sp_env_contains(&env, path_key));
  sp_str_t path = sp_env_get(&env, path_key);
  EXPECT_TRUE(path.len > 0);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, capture_is_snapshot) {
  SKIP_ON_FREESTANDING()
  sp_env_t env = sp_env_capture(ut.mem);
  u32 count = sp_env_count(&env);

  sp_env_insert(&env, sp_str_lit("SP_TEST_ONLY_VAR"), sp_str_lit("hello"));
  EXPECT_EQ(sp_env_count(&env), count + 1);

  sp_env_t env2 = sp_env_capture(ut.mem);
  EXPECT_FALSE(sp_env_contains(&env2, sp_str_lit("SP_TEST_ONLY_VAR")));

  sp_env_destroy(&env);
  sp_env_destroy(&env2);
}

UTEST_F(sp_env, destroy_then_reinit) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  sp_env_insert(&env, sp_str_lit("A"), sp_str_lit("1"));
  sp_env_destroy(&env);

  sp_env_init(ut.mem, &env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  sp_env_insert(&env, sp_str_lit("B"), sp_str_lit("2"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, sp_str_lit("B")), "2");
  sp_env_destroy(&env);
}

#if defined(SP_POSIX)
UTEST_F(sp_env, to_posix_envp) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  sp_env_insert(&env, sp_str_lit("AA"), sp_str_lit("11"));
  sp_env_insert(&env, sp_str_lit("BB"), sp_str_lit("22"));

  c8** envp = sp_env_to_posix_envp_a(ut.mem, &env);

  u32 count = 0;
  while (envp[count] != SP_NULLPTR) count++;
  EXPECT_EQ(count, (u32)2);

  bool found_aa = false;
  bool found_bb = false;
  for (u32 i = 0; i < count; i++) {
    sp_str_t entry = sp_str_view(envp[i]);
    if (sp_str_equal(entry, sp_str_lit("AA=11"))) found_aa = true;
    if (sp_str_equal(entry, sp_str_lit("BB=22"))) found_bb = true;
  }
  EXPECT_TRUE(found_aa);
  EXPECT_TRUE(found_bb);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, to_posix_envp_empty) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  c8** envp = sp_env_to_posix_envp_a(ut.mem, &env);
  EXPECT_EQ(envp[0], (c8*)SP_NULLPTR);

  sp_env_destroy(&env);
}
#endif

UTEST(sp_os_env, get_path) {
  SKIP_ON_WASM()
  SKIP_ON_FREESTANDING()
  sp_str_t path = sp_os_env_get(sp_str_lit("PATH"));
  EXPECT_TRUE(sp_str_valid(path));
  EXPECT_TRUE(path.len > 0);
}

UTEST(sp_os_env, get_missing) {
  SKIP_ON_FREESTANDING()
  sp_str_t val = sp_os_env_get(sp_str_lit("SP_DEFINITELY_NOT_SET_12345"));
  EXPECT_TRUE(sp_str_empty(val));
}

UTEST(sp_os_env, iterate) {
  SKIP_ON_WASM()
  SKIP_ON_FREESTANDING()
  sp_os_env_it_t it = sp_os_env_it_begin();

  #if defined(SP_WIN32)
    sp_str_t path_key = sp_str_lit("Path");
  #else
    sp_str_t path_key = sp_str_lit("PATH");
  #endif

  u32 count = 0;
  bool found_path = false;
  while (sp_os_env_it_valid(&it)) {
    EXPECT_TRUE(it.key.len > 0);
    if (sp_str_equal(it.key, path_key)) {
      found_path = true;
    }
    count++;
    sp_os_env_it_next(&it);
  }

  EXPECT_TRUE(count > 0);
  EXPECT_TRUE(found_path);
}

UTEST(sp_os_env, iterate_matches_capture) {
  SKIP_ON_FREESTANDING()
  sp_env_t captured = sp_env_capture(sp_mem_os_new());

  sp_os_env_it_t it = sp_os_env_it_begin();
  u32 it_count = 0;
  while (sp_os_env_it_valid(&it)) {
    EXPECT_TRUE(sp_env_contains(&captured, it.key));
    it_count++;
    sp_os_env_it_next(&it);
  }

  EXPECT_EQ(it_count, sp_env_count(&captured));
  sp_env_destroy(&captured);
}
