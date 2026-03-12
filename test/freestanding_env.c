#define SP_IMPLEMENTATION
#define SP_FREESTANDING
#define SP_BUILTIN
#include "sp.h"

static void print(const char* s) {
  u64 len = 0;
  while (s[len]) len++;
  sp_sys_write(2, s, len);
}

static void print_ok(const char* name) {
  print("  [OK] ");
  print(name);
  print("\n");
}

static void print_fail(const char* name) {
  print("  [FAIL] ");
  print(name);
  print("\n");
}

#define TEST(name, cond) do { \
  if (cond) { print_ok(name); } \
  else { print_fail(name); failures++; } \
} while(0)

s32 env_main(s32 argc, const c8** argv) {
  (void)argc; (void)argv;
  int failures = 0;

  print("=== freestanding env tests ===\n");

  // sp_envp should have been set by SP_ENTRY
  TEST("sp_envp is set", sp_envp != 0);

  // walk envp directly to count
  u32 raw_count = 0;
  if (sp_envp) {
    for (const c8** p = sp_envp; *p; p++) raw_count++;
  }
  TEST("envp has entries", raw_count > 0);

  // sp_os_env_get should find PATH
  sp_str_t path = sp_os_env_get(SP_LIT("PATH"));
  TEST("sp_os_env_get(PATH) non-empty", path.len > 0);

  // sp_os_env_get for missing var should be empty
  sp_str_t missing = sp_os_env_get(SP_LIT("SP_DEFINITELY_NOT_SET_XYZ_12345"));
  TEST("sp_os_env_get(missing) is empty", missing.len == 0);

  // sp_env_capture should produce a map with entries
  sp_env_t env = sp_env_capture();
  TEST("sp_env_capture count > 0", sp_env_count(&env) > 0);
  TEST("sp_env_capture has PATH", sp_env_contains(&env, SP_LIT("PATH")));
  TEST("capture count matches envp count", sp_env_count(&env) == raw_count);
  sp_env_destroy(&env);

  // iterator should match
  sp_os_env_it_t it = sp_os_env_it_begin();
  u32 it_count = 0;
  while (sp_os_env_it_valid(&it)) {
    it_count++;
    sp_os_env_it_next(&it);
  }
  TEST("iterator count matches envp count", it_count == raw_count);

  print("=== done ===\n");
  return failures;
}

SP_ENTRY(env_main)
