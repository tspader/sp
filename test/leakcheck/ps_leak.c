/*
 * ps_leak.c - Subprocess/Environment API Memory Leak Analysis
 *
 * This program performs real subprocess and environment operations and
 * tracks memory usage to identify functions that leak temporary memory internally.
 *
 * Build: cc -I../.. -o ps_leak ps_leak.c
 * Run: ./ps_leak
 */

#define SP_IMPLEMENTATION
#include "sp.h"
#include <stdio.h>

typedef struct {
  const c8* name;
  u32 bytes_before;
  u32 bytes_after;
  u32 leaked;
} leak_result_t;

#define MAX_RESULTS 64
static leak_result_t g_results[MAX_RESULTS];
static u32 g_result_count = 0;

static void record_result(const c8* name, u32 before, u32 after) {
  if (g_result_count < MAX_RESULTS) {
    g_results[g_result_count++] = (leak_result_t){
      .name = name,
      .bytes_before = before,
      .bytes_after = after,
      .leaked = after - before,
    };
  }
}

// Use scratch arena pattern to track temporary allocations
// Measure bytes used before/after each test (delta approach)
#define LEAK_TEST_BEGIN(test_name) \
  do { \
    const c8* __leak_name = test_name; \
    sp_mem_scratch_t __scratch = sp_mem_begin_scratch(); \
    u32 __leak_before = sp_mem_arena_bytes_used(sp_mem_get_scratch_arena());

#define LEAK_TEST_END() \
    u32 __leak_after = sp_mem_arena_bytes_used(sp_mem_get_scratch_arena()); \
    sp_mem_end_scratch(__scratch); \
    record_result(__leak_name, __leak_before, __leak_after); \
  } while(0)

/*
 * ENVIRONMENT API LEAK TESTS
 */

static void test_os_get_env_var(void) {
  LEAK_TEST_BEGIN("sp_os_get_env_var") {
    // sp_os_get_env_var calls sp_str_to_cstr(key)
    sp_str_t home = sp_os_get_env_var(SP_LIT("HOME"));
    (void)home;
  } LEAK_TEST_END();
}

static void test_os_get_env_var_multiple(void) {
  LEAK_TEST_BEGIN("sp_os_get_env_var (x10)") {
    for (int i = 0; i < 10; i++) {
      sp_str_t home = sp_os_get_env_var(SP_LIT("HOME"));
      (void)home;
    }
  } LEAK_TEST_END();
}

static void test_os_get_env_as_path(void) {
  LEAK_TEST_BEGIN("sp_os_get_env_as_path") {
    // Calls sp_str_to_cstr + sp_fs_normalize_path
    sp_str_t path = sp_os_get_env_as_path(SP_LIT("HOME"));
    (void)path;
  } LEAK_TEST_END();
}

static void test_os_clear_env_var(void) {
  // Set a temp var first
  sp_os_export_env_var(SP_LIT("SP_TEST_CLEAR"), SP_LIT("value"), SP_ENV_EXPORT_OVERWRITE_DUPES);

  LEAK_TEST_BEGIN("sp_os_clear_env_var") {
    sp_os_clear_env_var(SP_LIT("SP_TEST_CLEAR"));
  } LEAK_TEST_END();
}

static void test_os_export_env_var(void) {
  LEAK_TEST_BEGIN("sp_os_export_env_var") {
    // Calls sp_str_to_cstr twice (key and value)
    sp_os_export_env_var(SP_LIT("SP_TEST_EXPORT"), SP_LIT("test_value"), SP_ENV_EXPORT_OVERWRITE_DUPES);
  } LEAK_TEST_END();

  sp_os_clear_env_var(SP_LIT("SP_TEST_EXPORT"));
}

static void test_os_export_env_var_multiple(void) {
  LEAK_TEST_BEGIN("sp_os_export_env_var (x5)") {
    for (int i = 0; i < 5; i++) {
      sp_os_export_env_var(SP_LIT("SP_TEST_MULTI"), SP_LIT("value"), SP_ENV_EXPORT_OVERWRITE_DUPES);
    }
  } LEAK_TEST_END();

  sp_os_clear_env_var(SP_LIT("SP_TEST_MULTI"));
}

static void test_env_init(void) {
  // Note: env_init/destroy are generally leak-free since they're setup/teardown
  // But let's test that init doesn't leak temporary memory
  LEAK_TEST_BEGIN("sp_env_init") {
    sp_env_t env = SP_ZERO_INITIALIZE();
    sp_env_init(&env);
    // Immediately destroy to avoid leak
    sp_env_destroy(&env);
  } LEAK_TEST_END();
}

static void test_env_capture(void) {
  LEAK_TEST_BEGIN("sp_env_capture") {
    // Captures all environment variables
    sp_env_t env = sp_env_capture();
    sp_env_destroy(&env);
  } LEAK_TEST_END();
}

static void test_env_insert(void) {
  // Test that env_insert itself leaks temporary memory
  // We can't easily separate the internal leak from the result storage
  // So we test a full sequence
  LEAK_TEST_BEGIN("sp_env_insert") {
    sp_env_t env = SP_ZERO_INITIALIZE();
    sp_env_init(&env);
    sp_env_insert(&env, SP_LIT("KEY"), SP_LIT("VALUE"));
    sp_env_destroy(&env);
  } LEAK_TEST_END();
}

static void test_env_get(void) {
  LEAK_TEST_BEGIN("sp_env_get") {
    sp_env_t env = SP_ZERO_INITIALIZE();
    sp_env_init(&env);
    sp_env_insert(&env, SP_LIT("KEY"), SP_LIT("VALUE"));
    sp_str_t val = sp_env_get(&env, SP_LIT("KEY"));
    (void)val;
    sp_env_destroy(&env);
  } LEAK_TEST_END();
}

static void test_env_copy(void) {
  LEAK_TEST_BEGIN("sp_env_copy") {
    sp_env_t env = SP_ZERO_INITIALIZE();
    sp_env_init(&env);
    sp_env_insert(&env, SP_LIT("KEY1"), SP_LIT("VALUE1"));
    sp_env_insert(&env, SP_LIT("KEY2"), SP_LIT("VALUE2"));
    sp_env_t copy = sp_env_copy(&env);
    sp_env_destroy(&copy);
    sp_env_destroy(&env);
  } LEAK_TEST_END();
}

static void test_os_export_env(void) {
  LEAK_TEST_BEGIN("sp_os_export_env") {
    sp_env_t env = SP_ZERO_INITIALIZE();
    sp_env_init(&env);
    sp_env_insert(&env, SP_LIT("SP_EXPORT1"), SP_LIT("v1"));
    sp_env_insert(&env, SP_LIT("SP_EXPORT2"), SP_LIT("v2"));
    sp_os_export_env(&env, SP_ENV_EXPORT_OVERWRITE_DUPES);
    sp_env_destroy(&env);
  } LEAK_TEST_END();

  // Cleanup outside the test
  sp_os_clear_env_var(SP_LIT("SP_EXPORT1"));
  sp_os_clear_env_var(SP_LIT("SP_EXPORT2"));
}

/*
 * SUBPROCESS API LEAK TESTS
 */

static void test_ps_config_add_arg(void) {
  LEAK_TEST_BEGIN("sp_ps_config_add_arg") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("echo");
    sp_ps_config_add_arg(&config, SP_LIT("hello"));
    sp_ps_config_add_arg(&config, SP_LIT("world"));
    sp_dyn_array_free(config.dyn_args);
  } LEAK_TEST_END();
}

static void test_ps_config_copy(void) {
  LEAK_TEST_BEGIN("sp_ps_config_copy") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("echo");
    config.args[0] = SP_LIT("hello");
    config.cwd = SP_LIT("/tmp");
    // sp_ps_config_copy does many sp_str_copy calls
    sp_ps_config_t copy = sp_ps_config_copy(&config);
    (void)copy;
    // Note: We're not freeing copy to show the leak (intentional allocation)
  } LEAK_TEST_END();
}

static void test_ps_create_simple(void) {
  LEAK_TEST_BEGIN("sp_ps_create (echo)") {
    // sp_ps_create calls:
    // - sp_ps_build_posix_args (sp_str_to_cstr for each arg)
    // - sp_ps_build_posix_env (sp_str_to_cstr for each env var)
    // - sp_ps_set_cwd if cwd is set (sp_str_to_cstr)
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("true");  // Quick command
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_create_with_args(void) {
  LEAK_TEST_BEGIN("sp_ps_create (with args)") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("echo");
    config.args[0] = SP_LIT("arg1");
    config.args[1] = SP_LIT("arg2");
    config.args[2] = SP_LIT("arg3");
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_create_with_cwd(void) {
  LEAK_TEST_BEGIN("sp_ps_create (with cwd)") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("pwd");
    config.cwd = SP_LIT("/tmp");
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_create_with_env_inherit(void) {
  LEAK_TEST_BEGIN("sp_ps_create (env inherit)") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("true");
    config.env.mode = SP_PS_ENV_INHERIT;
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_create_with_env_clean(void) {
  LEAK_TEST_BEGIN("sp_ps_create (env clean)") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("true");
    config.env.mode = SP_PS_ENV_CLEAN;
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_create_with_extra_env(void) {
  LEAK_TEST_BEGIN("sp_ps_create (extra env vars)") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("true");
    config.env.mode = SP_PS_ENV_CLEAN;
    config.env.extra[0] = (sp_env_var_t){ .key = SP_LIT("VAR1"), .value = SP_LIT("val1") };
    config.env.extra[1] = (sp_env_var_t){ .key = SP_LIT("VAR2"), .value = SP_LIT("val2") };
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_run(void) {
  LEAK_TEST_BEGIN("sp_ps_run") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("echo");
    config.args[0] = SP_LIT("hello");
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_output_t output = sp_ps_run(config);
    (void)output;
  } LEAK_TEST_END();
}

static void test_ps_run_multiple(void) {
  LEAK_TEST_BEGIN("sp_ps_run (x3)") {
    for (int i = 0; i < 3; i++) {
      sp_ps_config_t config = SP_ZERO_INITIALIZE();
      config.command = SP_LIT("true");
      config.io.err.mode = SP_PS_IO_MODE_NULL;

      sp_ps_output_t output = sp_ps_run(config);
      (void)output;
    }
  } LEAK_TEST_END();
}

static void test_ps_output(void) {
  LEAK_TEST_BEGIN("sp_ps_output") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("echo");
    config.args[0] = SP_LIT("test output");
    config.io.out.mode = SP_PS_IO_MODE_CREATE;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);
    sp_ps_output_t output = sp_ps_output(&ps);
    (void)output;
  } LEAK_TEST_END();
}

static void test_ps_io_in(void) {
  LEAK_TEST_BEGIN("sp_ps_io_in") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("head");  // Exits after receiving input
    config.args[0] = SP_LIT("-c1");   // Read 1 char and exit
    config.io.in.mode = SP_PS_IO_MODE_CREATE;
    config.io.out.mode = SP_PS_IO_MODE_NULL;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);

    // sp_ps_io_in allocates a sp_io_writer_t
    sp_io_writer_t* writer = sp_ps_io_in(&ps);
    if (writer) {
      sp_io_write_str(writer, SP_LIT("x"));  // Send input so head exits
      sp_io_writer_close(writer);
    }

    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void test_ps_io_out(void) {
  LEAK_TEST_BEGIN("sp_ps_io_out") {
    sp_ps_config_t config = SP_ZERO_INITIALIZE();
    config.command = SP_LIT("echo");
    config.args[0] = SP_LIT("hello");
    config.io.out.mode = SP_PS_IO_MODE_CREATE;
    config.io.err.mode = SP_PS_IO_MODE_NULL;

    sp_ps_t ps = sp_ps_create(config);

    // sp_ps_io_out allocates a sp_io_reader_t
    sp_io_reader_t* reader = sp_ps_io_out(&ps);
    (void)reader;

    sp_ps_wait(&ps);
  } LEAK_TEST_END();
}

static void print_results(void) {
  printf("\n");
  printf("=== SUBPROCESS/ENV LEAK TEST RESULTS ===\n");
  printf("\n");

  u32 total_leaked = 0;
  u32 leaking_functions = 0;

  for (u32 i = 0; i < g_result_count; i++) {
    leak_result_t* r = &g_results[i];

    if (r->leaked > 0) {
      printf("LEAK %-40s leaked %5u bytes (before=%u, after=%u)\n",
        r->name,
        r->leaked,
        r->bytes_before,
        r->bytes_after
      );
      total_leaked += r->leaked;
      leaking_functions++;
    } else {
      printf("OK   %-40s no leak\n", r->name);
    }
  }

  printf("\n");
  printf("Summary:\n");
  printf("  Total functions tested: %u\n", g_result_count);
  printf("  Functions with leaks:   %u\n", leaking_functions);
  printf("  Total bytes leaked:     %u\n", total_leaked);
  printf("\n");
}

int main(void) {
  printf("Subprocess/Environment Memory Leak Analysis\n");
  printf("Testing subprocess and environment API functions for internal memory leaks...\n");
  printf("\n");

  // Environment API tests
  test_os_get_env_var();
  test_os_get_env_var_multiple();
  test_os_get_env_as_path();
  test_os_clear_env_var();
  test_os_export_env_var();
  test_os_export_env_var_multiple();
  test_env_init();
  test_env_capture();
  test_env_insert();
  test_env_get();
  test_env_copy();
  test_os_export_env();

  // Subprocess API tests
  test_ps_config_add_arg();
  test_ps_config_copy();
  test_ps_create_simple();
  test_ps_create_with_args();
  test_ps_create_with_cwd();
  test_ps_create_with_env_inherit();
  test_ps_create_with_env_clean();
  test_ps_create_with_extra_env();
  test_ps_run();
  test_ps_run_multiple();
  test_ps_output();
  test_ps_io_in();
  test_ps_io_out();

  // Print results
  print_results();

  return 0;
}
