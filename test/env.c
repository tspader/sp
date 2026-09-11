#define SP_APP
#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

// A variable the OS guarantees is in the process env block, with stable casing.
// HOME is set by the loader on POSIX; SystemRoot is populated by csrss before
// any user process exists on Windows. Avoids the PATH/Path casing footgun.
#if defined(SP_WIN32)
  #define SP_TEST_ENV_OS_KEY "SystemRoot"
#else
  #define SP_TEST_ENV_OS_KEY "HOME"
#endif

struct env {
  sp_mem_heap_t* heap;
  sp_mem_t mem;
};

UTEST_F_SETUP(env) {
  ut.heap = sp_mem_heap_new();
  ut.mem = sp_mem_heap_as_allocator(ut.heap);
}

UTEST_F_TEARDOWN(env) {
  sp_mem_heap_destroy(ut.heap);
}

UTEST_F(env, init_empty) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  EXPECT_FALSE(sp_env_contains_c(&env, "PATH"));
  sp_env_destroy(&env);
}

UTEST_F(env, insert_and_get) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "FOO", "bar");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "FOO"), "bar");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(env, insert_overwrites) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "KEY", "first");
  sp_env_insert_c(&env, "KEY", "second");

  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "KEY"), "second");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(env, get_missing_returns_empty) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_str_t val = sp_env_get_c(&env, "DOES_NOT_EXIST");
  EXPECT_TRUE(sp_str_empty(val));

  sp_env_destroy(&env);
}

UTEST_F(env, contains) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  EXPECT_FALSE(sp_env_contains_c(&env, "X"));
  sp_env_insert_c(&env, "X", "y");
  EXPECT_TRUE(sp_env_contains_c(&env, "X"));

  sp_env_destroy(&env);
}

UTEST_F(env, erase) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "A", "1");
  sp_env_insert_c(&env, "B", "2");
  EXPECT_EQ(sp_env_count(&env), (u32)2);

  sp_env_erase_c(&env, "A");
  EXPECT_EQ(sp_env_count(&env), (u32)1);
  EXPECT_FALSE(sp_env_contains_c(&env, "A"));
  EXPECT_TRUE(sp_env_contains_c(&env, "B"));

  sp_env_destroy(&env);
}

UTEST_F(env, erase_nonexistent) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "A", "1");
  sp_env_erase_c(&env, "NOPE");
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
}

UTEST_F(env, multiple_entries) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "A", "1");
  sp_env_insert_c(&env, "B", "2");
  sp_env_insert_c(&env, "C", "3");

  EXPECT_EQ(sp_env_count(&env), (u32)3);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "A"), "1");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "B"), "2");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "C"), "3");

  sp_env_destroy(&env);
}

UTEST_F(env, copy_is_independent) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  sp_env_insert_c(&env, "K", "V");

  sp_env_t copy = sp_env_copy(ut.mem, &env);
  EXPECT_EQ(sp_env_count(&copy), (u32)1);
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&copy, "K"), "V");

  sp_env_insert_c(&copy, "NEW", "val");
  EXPECT_EQ(sp_env_count(&copy), (u32)2);
  EXPECT_EQ(sp_env_count(&env), (u32)1);

  sp_env_destroy(&env);
  sp_env_destroy(&copy);
}

UTEST_F(env, empty_value) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "EMPTY", "");
  EXPECT_TRUE(sp_env_contains_c(&env, "EMPTY"));
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "EMPTY"), "");

  sp_env_destroy(&env);
}

UTEST_F(env, value_with_equals) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  sp_env_insert_c(&env, "spum", "foo=bar");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "spum"), "foo=bar");

  sp_env_destroy(&env);
}

UTEST_F(env, capture_has_known_var) {
  SKIP_ON_WASM()
  SKIP_ON_WIN32() // @spader Just for a moment; it's just a CI thing
  sp_env_t env = sp_env_capture(ut.mem);

  EXPECT_TRUE(sp_env_count(&env) > 0);
  EXPECT_TRUE(sp_env_contains_c(&env, SP_TEST_ENV_OS_KEY));
  EXPECT_TRUE(sp_env_get_c(&env, SP_TEST_ENV_OS_KEY).len > 0);

  sp_env_destroy(&env);
}

UTEST_F(env, capture_is_snapshot) {
  sp_env_t env = sp_env_capture(ut.mem);
  u32 count = sp_env_count(&env);

  sp_env_insert_c(&env, "SP_TEST_ONLY_VAR", "hello");
  EXPECT_EQ(sp_env_count(&env), count + 1);

  sp_env_t env2 = sp_env_capture(ut.mem);
  EXPECT_FALSE(sp_env_contains_c(&env2, "SP_TEST_ONLY_VAR"));

  sp_env_destroy(&env);
  sp_env_destroy(&env2);
}

UTEST_F(env, destroy_then_reinit) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  sp_env_insert_c(&env, "A", "1");
  sp_env_destroy(&env);

  sp_env_init(ut.mem, &env);
  EXPECT_EQ(sp_env_count(&env), (u32)0);
  sp_env_insert_c(&env, "B", "2");
  SP_EXPECT_STR_EQ_CSTR(sp_env_get_c(&env, "B"), "2");
  sp_env_destroy(&env);
}

#if defined(SP_POSIX)
UTEST_F(env, to_posix_envp) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);
  sp_env_insert_c(&env, "AA", "11");
  sp_env_insert_c(&env, "BB", "22");

  c8** envp = sp_env_to_posix_envp(ut.mem, &env);

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

UTEST_F(env, to_posix_envp_empty) {
  sp_env_t env;
  sp_env_init(ut.mem, &env);

  c8** envp = sp_env_to_posix_envp(ut.mem, &env);
  EXPECT_EQ(envp[0], (c8*)SP_NULLPTR);

  sp_env_destroy(&env);
}
#endif

UTEST_F(env, os_get_known_var) {
  SKIP_ON_WASM()
  sp_str_t value = sp_os_env_get(sp_str_lit(SP_TEST_ENV_OS_KEY));
  EXPECT_TRUE(sp_str_valid(value));
  EXPECT_TRUE(value.len > 0);
}

UTEST_F(env, get_missing) {
  sp_str_t val = sp_os_env_get(sp_str_lit("SP_DEFINITELY_NOT_SET_12345"));
  EXPECT_TRUE(sp_str_empty(val));
}

UTEST_F(env, iterate) {
  SKIP_ON_WASM()
  SKIP_ON_WIN32() // @spader Just for a moment; it's just a CI thing
  sp_str_t key = sp_str_lit(SP_TEST_ENV_OS_KEY);
  u32 count = 0;
  bool found = false;
  for (sp_os_env_it_t it = sp_os_env_it_begin(); sp_os_env_it_valid(&it); sp_os_env_it_next(&it)) {
    EXPECT_TRUE(it.key.len > 0);
    if (sp_str_equal(it.key, key)) found = true;
    count++;
  }

  EXPECT_TRUE(count > 0);
  EXPECT_TRUE(found);
}

UTEST_F(env, iterate_matches_capture) {
  sp_env_t captured = sp_env_capture(ut.mem);

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

///////////////
// HOST KEYS //
///////////////
#define ENV_TEST_MAX_OS 2
#define ENV_TEST_MAX_OPS 4
#define ENV_TEST_MAX_VARS 4
#define ENV_TEST_SENTINEL "sp-env-host-keys"
#define ENV_TEST_NEW_KEY "SP_ENV_HOST_KEYS_TEST"
#define ENV_TEST_OS_KEY "SP_ENV_HOST_KEYS_OS"
#define ENV_TEST_OS_KEY_LOWER "sp_env_host_keys_os"

#if defined(SP_WIN32)
static void env_os_set(const c8* key, const c8* value) {
  _putenv_s(key, value);
}

static void env_os_unset(const c8* key) {
  _putenv_s(key, "");
}
#elif defined(SP_POSIX)
static void env_os_set(const c8* key, const c8* value) {
  setenv(key, value, 1);
}

static void env_os_unset(const c8* key) {
  unsetenv(key);
}
#else
static void env_os_set(const c8* key, const c8* value) {
  SP_UNIMPLEMENTED();
  sp_unused(key);
  sp_unused(value);
}

static void env_os_unset(const c8* key) {
  SP_UNIMPLEMENTED();
  sp_unused(key);
}
#endif

typedef struct {
  const c8* key;
  const c8* value;
} env_os_var_t;

typedef enum {
  ENV_OP_NONE,
  ENV_OP_INSERT,
  ENV_OP_ERASE,
  ENV_OP_COPY,
} env_op_kind_t;

typedef struct {
  env_op_kind_t kind;
  const c8* key;
  const c8* value;
} env_op_t;

typedef enum {
  ENV_VALUE_MISSING,
  ENV_VALUE_LITERAL,
  ENV_VALUE_OS,
} env_value_kind_t;

typedef struct {
  const c8* key;
  env_value_kind_t kind;
  const c8* value;
} env_var_expect_t;

typedef struct {
  s32 count_delta;
  bool keys_preserved;
  env_var_expect_t vars [ENV_TEST_MAX_VARS];
} env_host_expect_t;

typedef struct {
  env_os_var_t os [ENV_TEST_MAX_OS];
  env_op_t ops [ENV_TEST_MAX_OPS];
  env_host_expect_t expect;
} env_host_test_t;

void run_env_host_test(struct env* utest_fixture, s32* utest_result, env_host_test_t t) {
  SKIP_ON_WASM()
  EXPECT_FALSE(sp_str_empty(sp_os_env_get(sp_str_lit("PATH"))));

  sp_carr_for(t.os, it) {
    if (!t.os[it].key) break;
    env_os_set(t.os[it].key, t.os[it].value);
  }

  sp_env_t env = sp_env_capture(ut.mem);
  u32 captured = sp_env_count(&env);

  sp_ht(sp_str_t, bool) spellings = sp_zero;
  sp_str_ht_init(ut.mem, spellings);
  sp_str_ht_for(env.vars, it) {
    sp_str_ht_insert(spellings, sp_str_copy(ut.mem, *sp_str_ht_it_getkp(env.vars, it)), true);
  }

  sp_carr_for(t.ops, it) {
    env_op_t op = t.ops[it];
    switch (op.kind) {
      case ENV_OP_NONE: {
        break;
      }
      case ENV_OP_INSERT: {
        sp_env_insert_c(&env, op.key, op.value);
        break;
      }
      case ENV_OP_ERASE: {
        sp_env_erase_c(&env, op.key);
        break;
      }
      case ENV_OP_COPY: {
        sp_env_t copy = sp_env_copy(ut.mem, &env);
        sp_env_destroy(&env);
        env = copy;
        break;
      }
    }
  }

  EXPECT_EQ(sp_env_count(&env), (u32)((s32)captured + t.expect.count_delta));

  sp_carr_for(t.expect.vars, it) {
    env_var_expect_t var = t.expect.vars[it];
    if (!var.key) break;

    sp_str_t key = sp_cstr_as_str(var.key);
    switch (var.kind) {
      case ENV_VALUE_MISSING: {
        EXPECT_FALSE(sp_env_contains(&env, key));
        break;
      }
      case ENV_VALUE_LITERAL: {
        EXPECT_TRUE(sp_env_contains(&env, key));
        SP_EXPECT_STR_EQ_CSTR(sp_env_get(&env, key), var.value);
        break;
      }
      case ENV_VALUE_OS: {
        sp_str_t os = sp_os_env_get(key);
        EXPECT_TRUE(sp_env_contains(&env, key) == !sp_str_empty(os));
        SP_EXPECT_STR_EQ(sp_env_get(&env, key), os);
        break;
      }
    }
  }

  if (t.expect.keys_preserved) {
    EXPECT_EQ(sp_env_count(&env), (u32)sp_str_ht_size(spellings));
    sp_str_ht_for(env.vars, it) {
      EXPECT_TRUE(sp_str_ht_get(spellings, *sp_str_ht_it_getkp(env.vars, it)) != SP_NULLPTR);
    }
  }

  sp_carr_for(t.os, it) {
    if (!t.os[it].key) break;
    env_os_unset(t.os[it].key);
  }

  sp_str_ht_free(spellings);
  sp_env_destroy(&env);
}

UTEST_F(env, host_capture_get_matches_os) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .expect = {
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_OS },
        { .key = SP_TEST_ENV_OS_KEY, .kind = ENV_VALUE_OS },
        { .key = "SP_ENV_HOST_KEYS_UNSET", .kind = ENV_VALUE_OS },
      },
    },
  });
}

UTEST_F(env, host_os_set_spelling_matches_os) {
  SKIP_ON_FREESTANDING()
  SKIP_ON_WASM()
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .os = {
      { .key = ENV_TEST_OS_KEY, .value = ENV_TEST_SENTINEL },
    },
    .expect = {
      .vars = {
        { .key = ENV_TEST_OS_KEY, .kind = ENV_VALUE_OS },
        { .key = ENV_TEST_OS_KEY_LOWER, .kind = ENV_VALUE_OS },
      },
    },
  });
}

UTEST_F(env, host_insert_over_captured) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_INSERT, .key = "PATH", .value = ENV_TEST_SENTINEL },
    },
    .expect = {
      .keys_preserved = true,
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_LITERAL, .value = ENV_TEST_SENTINEL },
        { .key = SP_TEST_ENV_OS_KEY, .kind = ENV_VALUE_OS },
      },
    },
  });
}

UTEST_F(env, host_insert_over_captured_twice) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_INSERT, .key = "PATH", .value = "first" },
      { .kind = ENV_OP_INSERT, .key = "PATH", .value = ENV_TEST_SENTINEL },
    },
    .expect = {
      .keys_preserved = true,
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_LITERAL, .value = ENV_TEST_SENTINEL },
      },
    },
  });
}

UTEST_F(env, host_erase_captured) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_ERASE, .key = "PATH" },
    },
    .expect = {
      .count_delta = -1,
      .vars = {
        { .key = "PATH" },
        { .key = SP_TEST_ENV_OS_KEY, .kind = ENV_VALUE_OS },
      },
    },
  });
}

UTEST_F(env, host_erase_then_insert) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_ERASE, .key = "PATH" },
      { .kind = ENV_OP_INSERT, .key = "PATH", .value = ENV_TEST_SENTINEL },
    },
    .expect = {
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_LITERAL, .value = ENV_TEST_SENTINEL },
      },
    },
  });
}

UTEST_F(env, host_copy_matches_os) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_COPY },
    },
    .expect = {
      .keys_preserved = true,
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_OS },
        { .key = SP_TEST_ENV_OS_KEY, .kind = ENV_VALUE_OS },
      },
    },
  });
}

UTEST_F(env, host_copy_then_insert_over) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_COPY },
      { .kind = ENV_OP_INSERT, .key = "PATH", .value = ENV_TEST_SENTINEL },
    },
    .expect = {
      .keys_preserved = true,
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_LITERAL, .value = ENV_TEST_SENTINEL },
      },
    },
  });
}

UTEST_F(env, host_insert_over_then_copy) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_INSERT, .key = "PATH", .value = ENV_TEST_SENTINEL },
      { .kind = ENV_OP_COPY },
    },
    .expect = {
      .keys_preserved = true,
      .vars = {
        { .key = "PATH", .kind = ENV_VALUE_LITERAL, .value = ENV_TEST_SENTINEL },
      },
    },
  });
}

UTEST_F(env, host_insert_new) {
  run_env_host_test(&ut, &ur, (env_host_test_t) {
    .ops = {
      { .kind = ENV_OP_INSERT, .key = ENV_TEST_NEW_KEY, .value = ENV_TEST_SENTINEL },
    },
    .expect = {
      .count_delta = 1,
      .vars = {
        { .key = ENV_TEST_NEW_KEY, .kind = ENV_VALUE_LITERAL, .value = ENV_TEST_SENTINEL },
        { .key = "PATH", .kind = ENV_VALUE_OS },
      },
    },
  });
}

UTEST_F(env, capture_values_match_os) {
  SKIP_ON_WASM()
  sp_env_t captured = sp_env_capture(ut.mem);

  for (sp_os_env_it_t it = sp_os_env_it_begin(); sp_os_env_it_valid(&it); sp_os_env_it_next(&it)) {
    SP_EXPECT_STR_EQ(sp_env_get(&captured, it.key), sp_os_env_get(it.key));
  }

  sp_env_destroy(&captured);
}
