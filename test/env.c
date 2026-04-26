#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"


SP_TEST_MAIN()

typedef struct sp_env {
  u8 placeholder;
} sp_env_fixture;

UTEST_F_SETUP(sp_env) {
  (void)utest_fixture;
}

UTEST_F_TEARDOWN(sp_env) {
  (void)utest_fixture;
}

UTEST_F(sp_env, init_empty) {
  sp_env_t env;
  sp_env_init(&env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  EXPECT_FALSE(sp_env_contains(&env, SP_LIT("PATH")));
  sp_env_destroy(&env);
}

UTEST_F(sp_env, insert_and_get) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("FOO"), SP_LIT("bar"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("FOO")), "bar");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, insert_overwrites) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("KEY"), SP_LIT("first"));
  sp_env_insert(&env, SP_LIT("KEY"), SP_LIT("second"));

  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("KEY")), "second");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, get_missing_returns_empty) {
  sp_env_t env;
  sp_env_init(&env);

  sp_str_t val = sp_env_get(&env, SP_LIT("DOES_NOT_EXIST"));
  EXPECT_TRUE(sp_str_empty(val));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, contains) {
  sp_env_t env;
  sp_env_init(&env);

  EXPECT_FALSE(sp_env_contains(&env, SP_LIT("X")));
  sp_env_insert(&env, SP_LIT("X"), SP_LIT("y"));
  EXPECT_TRUE(sp_env_contains(&env, SP_LIT("X")));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, erase) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_insert(&env, SP_LIT("B"), SP_LIT("2"));
  EXPECT_EQ(sp_env_count(&env), (u32)2);

  sp_env_erase(&env, SP_LIT("A"));
  EXPECT_EQ(sp_env_count(&env), (u32)1);
  EXPECT_FALSE(sp_env_contains(&env, SP_LIT("A")));
  EXPECT_TRUE(sp_env_contains(&env, SP_LIT("B")));

  sp_env_destroy(&env);
}

UTEST_F(sp_env, erase_nonexistent) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_erase(&env, SP_LIT("NOPE"));
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, multiple_entries) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_insert(&env, SP_LIT("B"), SP_LIT("2"));
  sp_env_insert(&env, SP_LIT("C"), SP_LIT("3"));

  EXPECT_EQ(sp_env_count(&env), (u32)3);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("A")), "1");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("B")), "2");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("C")), "3");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, copy_is_independent) {
  sp_env_t env;
  sp_env_init(&env);
  sp_env_insert(&env, SP_LIT("K"), SP_LIT("V"));

  sp_env_t copy = sp_env_copy(&env);
  EXPECT_EQ(sp_env_count(&copy), (u32)1);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&copy, SP_LIT("K")), "V");

  sp_env_insert(&copy, SP_LIT("NEW"), SP_LIT("val"));
  EXPECT_EQ(sp_env_count(&copy), (u32)2);
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
  sp_env_destroy(&copy);
}

UTEST_F(sp_env, empty_value) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("EMPTY"), SP_LIT(""));
  EXPECT_TRUE(sp_env_contains(&env, SP_LIT("EMPTY")));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("EMPTY")), "");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, value_with_equals) {
  sp_env_t env;
  sp_env_init(&env);

  sp_env_insert(&env, SP_LIT("DSN"), SP_LIT("host=localhost;port=5432"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("DSN")), "host=localhost;port=5432");

  sp_env_destroy(&env);
}

UTEST_F(sp_env, capture_has_path) {
  sp_env_t env = sp_env_capture();

  EXPECT_TRUE(sp_env_count(&env) > 0);

  // Windows PEB stores "Path", POSIX uses "PATH"
  #if defined(SP_WIN32)
    sp_str_t path_key = SP_LIT("Path");
  #else
    sp_str_t path_key = SP_LIT("PATH");
  #endif

  EXPECT_TRUE(sp_env_contains(&env, path_key));
  sp_str_t path = sp_env_get(&env, path_key);
  EXPECT_TRUE(path.len > 0);

  sp_env_destroy(&env);
}

UTEST_F(sp_env, capture_is_snapshot) {
  sp_env_t env = sp_env_capture();
  u32 count = sp_env_count(&env);

  sp_env_insert(&env, SP_LIT("SP_TEST_ONLY_VAR"), SP_LIT("hello"));
  EXPECT_EQ(sp_env_count(&env), count + 1);

  sp_env_t env2 = sp_env_capture();
  EXPECT_FALSE(sp_env_contains(&env2, SP_LIT("SP_TEST_ONLY_VAR")));

  sp_env_destroy(&env);
  sp_env_destroy(&env2);
}

UTEST_F(sp_env, destroy_then_reinit) {
  sp_env_t env;
  sp_env_init(&env);
  sp_env_insert(&env, SP_LIT("A"), SP_LIT("1"));
  sp_env_destroy(&env);

  sp_env_init(&env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  sp_env_insert(&env, SP_LIT("B"), SP_LIT("2"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, SP_LIT("B")), "2");
  sp_env_destroy(&env);
}

#if defined(SP_POSIX)
UTEST_F(sp_env, to_posix_envp) {
  sp_env_t env;
  sp_env_init(&env);
  sp_env_insert(&env, SP_LIT("AA"), SP_LIT("11"));
  sp_env_insert(&env, SP_LIT("BB"), SP_LIT("22"));

  c8** envp = sp_env_to_posix_envp(&env);

  u32 count = 0;
  while (envp[count] != SP_NULLPTR) count++;
  EXPECT_EQ(count, (u32)2);

  bool found_aa = false;
  bool found_bb = false;
  for (u32 i = 0; i < count; i++) {
    sp_str_t entry = sp_str_view(envp[i]);
    if (sp_str_equal(entry, SP_LIT("AA=11"))) found_aa = true;
    if (sp_str_equal(entry, SP_LIT("BB=22"))) found_bb = true;
  }
  EXPECT_TRUE(found_aa);
  EXPECT_TRUE(found_bb);

  sp_env_free_posix_envp(envp);
  sp_env_destroy(&env);
}

UTEST_F(sp_env, to_posix_envp_empty) {
  sp_env_t env;
  sp_env_init(&env);

  c8** envp = sp_env_to_posix_envp(&env);
  EXPECT_EQ(envp[0], (c8*)SP_NULLPTR);

  sp_env_free_posix_envp(envp);
  sp_env_destroy(&env);
}
#endif

UTEST(sp_os_env, get_path) {
  sp_str_t path = sp_os_env_get(SP_LIT("PATH"));
  EXPECT_TRUE(sp_str_valid(path));
  EXPECT_TRUE(path.len > 0);
}

UTEST(sp_os_env, get_missing) {
  sp_str_t val = sp_os_env_get(SP_LIT("SP_DEFINITELY_NOT_SET_12345"));
  EXPECT_TRUE(sp_str_empty(val));
}

UTEST(sp_os_env, iterate) {
  sp_os_env_it_t it = sp_os_env_it_begin();

  // Windows PEB stores "Path", POSIX uses "PATH"
  #if defined(SP_WIN32)
    sp_str_t path_key = SP_LIT("Path");
  #else
    sp_str_t path_key = SP_LIT("PATH");
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
  sp_env_t captured = sp_env_capture();

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
