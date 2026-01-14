/*
 * fs_leak.c - Filesystem API Memory Leak Analysis
 *
 * This program performs real filesystem operations and tracks memory usage
 * to identify functions that leak temporary memory internally.
 *
 * Build: cc -I../.. -o fs_leak fs_leak.c
 * Run: ./fs_leak
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
// Measure bytes used inside the scratch scope
#define LEAK_TEST_BEGIN(test_name) \
  do { \
    const c8* __leak_name = test_name; \
    sp_mem_arena_t* __arena = sp_mem_get_scratch_arena(); \
    sp_mem_arena_clear(__arena); \
    sp_mem_scratch_t __scratch = sp_mem_begin_scratch(); \
    (void)0;

#define LEAK_TEST_END() \
    u32 __leak_after = sp_mem_arena_bytes_used(sp_mem_get_scratch_arena()); \
    sp_mem_end_scratch(__scratch); \
    record_result(__leak_name, 0, __leak_after); \
  } while(0)

// Test directory for our operations
static sp_str_t g_test_dir;

static void setup_test_dir(void) {
  g_test_dir = sp_fs_join_path(SP_LIT("/tmp"), SP_LIT("sp_leak_test"));
  sp_fs_remove_dir(g_test_dir);
  sp_fs_create_dir(g_test_dir);
}

static void cleanup_test_dir(void) {
  sp_fs_remove_dir(g_test_dir);
}

/*
 * FILESYSTEM LEAK TESTS
 * Each test performs a real operation and measures memory before/after
 */

static void test_fs_exists(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("test_file.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_exists") {
    // sp_fs_exists calls sp_str_to_cstr internally
    bool exists = sp_fs_exists(path);
    (void)exists;
  } LEAK_TEST_END();
}

static void test_fs_is_regular_file(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("regular.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_is_regular_file") {
    bool is_file = sp_fs_is_regular_file(path);
    (void)is_file;
  } LEAK_TEST_END();
}

static void test_fs_is_dir(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("subdir"));
  sp_fs_create_dir(path);

  LEAK_TEST_BEGIN("sp_fs_is_dir") {
    bool is_dir = sp_fs_is_dir(path);
    (void)is_dir;
  } LEAK_TEST_END();
}

static void test_fs_is_symlink(void) {
  sp_str_t target = sp_fs_join_path(g_test_dir, SP_LIT("target.txt"));
  sp_str_t link = sp_fs_join_path(g_test_dir, SP_LIT("link.txt"));
  sp_fs_create_file(target);
  sp_fs_create_sym_link(target, link);

  LEAK_TEST_BEGIN("sp_fs_is_symlink") {
    bool is_link = sp_fs_is_symlink(link);
    (void)is_link;
  } LEAK_TEST_END();
}

static void test_fs_create_dir(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("new_dir"));
  sp_fs_remove_dir(path);

  LEAK_TEST_BEGIN("sp_fs_create_dir") {
    // sp_fs_create_dir calls sp_str_to_cstr internally
    sp_fs_create_dir(path);
  } LEAK_TEST_END();
}

static void test_fs_remove_dir(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("dir_to_remove"));
  sp_fs_create_dir(path);

  LEAK_TEST_BEGIN("sp_fs_remove_dir") {
    // sp_fs_remove_dir calls sp_fs_collect + sp_str_to_cstr
    sp_fs_remove_dir(path);
  } LEAK_TEST_END();
}

static void test_fs_create_file(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("new_file.txt"));
  sp_fs_remove_file(path);

  LEAK_TEST_BEGIN("sp_fs_create_file") {
    sp_fs_create_file(path);
  } LEAK_TEST_END();
}

static void test_fs_remove_file(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("file_to_remove.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_remove_file") {
    sp_fs_remove_file(path);
  } LEAK_TEST_END();
}

static void test_fs_collect(void) {
  // Create some files to collect
  sp_str_t subdir = sp_fs_join_path(g_test_dir, SP_LIT("collect_dir"));
  sp_fs_create_dir(subdir);
  sp_fs_create_file(sp_fs_join_path(subdir, SP_LIT("a.txt")));
  sp_fs_create_file(sp_fs_join_path(subdir, SP_LIT("b.txt")));
  sp_fs_create_file(sp_fs_join_path(subdir, SP_LIT("c.txt")));

  LEAK_TEST_BEGIN("sp_fs_collect") {
    // sp_fs_collect allocates:
    // - cstr for path
    // - string builder for each entry
    // - sp_str_from_cstr for file_name
    sp_da(sp_os_dir_ent_t) entries = sp_fs_collect(subdir);
    (void)entries;
    // Note: entries array itself is intentionally returned (not a leak)
  } LEAK_TEST_END();
}

static void test_fs_collect_recursive(void) {
  // Create nested structure
  sp_str_t base = sp_fs_join_path(g_test_dir, SP_LIT("recursive_dir"));
  sp_str_t nested = sp_fs_join_path(base, SP_LIT("nested"));
  sp_fs_create_dir(base);
  sp_fs_create_dir(nested);
  sp_fs_create_file(sp_fs_join_path(base, SP_LIT("a.txt")));
  sp_fs_create_file(sp_fs_join_path(nested, SP_LIT("b.txt")));

  LEAK_TEST_BEGIN("sp_fs_collect_recursive") {
    sp_da(sp_os_dir_ent_t) entries = sp_fs_collect_recursive(base);
    (void)entries;
  } LEAK_TEST_END();
}

static void test_fs_canonicalize_path(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("canon_test.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_canonicalize_path") {
    // Allocates via sp_str_to_cstr and sp_str_copy
    sp_str_t canon = sp_fs_canonicalize_path(path);
    (void)canon;
  } LEAK_TEST_END();
}

static void test_fs_normalize_path(void) {
  sp_str_t path = SP_LIT("path\\with\\backslashes");

  LEAK_TEST_BEGIN("sp_fs_normalize_path") {
    sp_str_t norm = sp_fs_normalize_path(path);
    (void)norm;
  } LEAK_TEST_END();
}

static void test_fs_join_path(void) {
  LEAK_TEST_BEGIN("sp_fs_join_path") {
    sp_str_t joined = sp_fs_join_path(SP_LIT("/base"), SP_LIT("child"));
    (void)joined;
  } LEAK_TEST_END();
}

static void test_fs_parent_path(void) {
  LEAK_TEST_BEGIN("sp_fs_parent_path") {
    sp_str_t parent = sp_fs_parent_path(SP_LIT("/base/child/file.txt"));
    (void)parent;
  } LEAK_TEST_END();
}

static void test_fs_get_name(void) {
  LEAK_TEST_BEGIN("sp_fs_get_name") {
    sp_str_t name = sp_fs_get_name(SP_LIT("/path/to/file.txt"));
    (void)name;
  } LEAK_TEST_END();
}

static void test_fs_get_ext(void) {
  LEAK_TEST_BEGIN("sp_fs_get_ext") {
    sp_str_t ext = sp_fs_get_ext(SP_LIT("/path/to/file.txt"));
    (void)ext;
  } LEAK_TEST_END();
}

static void test_fs_get_stem(void) {
  LEAK_TEST_BEGIN("sp_fs_get_stem") {
    sp_str_t stem = sp_fs_get_stem(SP_LIT("/path/to/file.txt"));
    (void)stem;
  } LEAK_TEST_END();
}

static void test_fs_get_cwd(void) {
  LEAK_TEST_BEGIN("sp_fs_get_cwd") {
    sp_str_t cwd = sp_fs_get_cwd();
    (void)cwd;
  } LEAK_TEST_END();
}

static void test_fs_get_exe_path(void) {
  LEAK_TEST_BEGIN("sp_fs_get_exe_path") {
    sp_str_t exe = sp_fs_get_exe_path();
    (void)exe;
  } LEAK_TEST_END();
}

static void test_fs_get_mod_time(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("modtime.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_get_mod_time") {
    sp_tm_epoch_t mod = sp_fs_get_mod_time(path);
    (void)mod;
  } LEAK_TEST_END();
}

static void test_fs_get_file_attrs(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("attrs.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_get_file_attrs") {
    sp_os_file_attr_t attrs = sp_fs_get_file_attrs(path);
    (void)attrs;
  } LEAK_TEST_END();
}

static void test_fs_create_hard_link(void) {
  sp_str_t target = sp_fs_join_path(g_test_dir, SP_LIT("link_target.txt"));
  sp_str_t link = sp_fs_join_path(g_test_dir, SP_LIT("hard_link.txt"));
  sp_fs_create_file(target);
  sp_fs_remove_file(link);

  LEAK_TEST_BEGIN("sp_fs_create_hard_link") {
    sp_fs_create_hard_link(target, link);
  } LEAK_TEST_END();
}

static void test_fs_create_sym_link(void) {
  sp_str_t target = sp_fs_join_path(g_test_dir, SP_LIT("sym_target.txt"));
  sp_str_t link = sp_fs_join_path(g_test_dir, SP_LIT("sym_link.txt"));
  sp_fs_create_file(target);
  sp_fs_remove_file(link);

  LEAK_TEST_BEGIN("sp_fs_create_sym_link") {
    sp_fs_create_sym_link(target, link);
  } LEAK_TEST_END();
}

static void test_fs_copy_file(void) {
  sp_str_t src = sp_fs_join_path(g_test_dir, SP_LIT("copy_src.txt"));
  sp_str_t dst = sp_fs_join_path(g_test_dir, SP_LIT("copy_dst.txt"));
  sp_fs_create_file(src);
  sp_fs_remove_file(dst);

  // Write some content to source
  sp_io_writer_t writer = sp_io_writer_from_file(src, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&writer, SP_LIT("test content for copy"));
  sp_io_writer_close(&writer);

  LEAK_TEST_BEGIN("sp_fs_copy_file") {
    sp_fs_copy_file(src, dst);
  } LEAK_TEST_END();
}

static void test_fs_is_target_regular_file(void) {
  sp_str_t target = sp_fs_join_path(g_test_dir, SP_LIT("is_target_file.txt"));
  sp_str_t link = sp_fs_join_path(g_test_dir, SP_LIT("is_target_link.txt"));
  sp_fs_create_file(target);
  sp_fs_remove_file(link);
  sp_fs_create_sym_link(target, link);

  LEAK_TEST_BEGIN("sp_fs_is_target_regular_file") {
    bool result = sp_fs_is_target_regular_file(link);
    (void)result;
  } LEAK_TEST_END();
}

static void test_fs_is_target_dir(void) {
  sp_str_t target = sp_fs_join_path(g_test_dir, SP_LIT("is_target_dir"));
  sp_str_t link = sp_fs_join_path(g_test_dir, SP_LIT("is_target_dir_link"));
  sp_fs_create_dir(target);
  sp_fs_remove_file(link);
  sp_fs_create_sym_link(target, link);

  LEAK_TEST_BEGIN("sp_fs_is_target_dir") {
    bool result = sp_fs_is_target_dir(link);
    (void)result;
  } LEAK_TEST_END();
}

static void test_fs_get_storage_path(void) {
  LEAK_TEST_BEGIN("sp_fs_get_storage_path") {
    sp_str_t path = sp_fs_get_storage_path();
    (void)path;
  } LEAK_TEST_END();
}

static void test_fs_get_config_path(void) {
  LEAK_TEST_BEGIN("sp_fs_get_config_path") {
    sp_str_t path = sp_fs_get_config_path();
    (void)path;
  } LEAK_TEST_END();
}

// Additional test: multiple calls to same function
static void test_fs_exists_multiple(void) {
  sp_str_t path = sp_fs_join_path(g_test_dir, SP_LIT("multi_test.txt"));
  sp_fs_create_file(path);

  LEAK_TEST_BEGIN("sp_fs_exists (x10)") {
    for (int i = 0; i < 10; i++) {
      sp_fs_exists(path);
    }
  } LEAK_TEST_END();
}

static void test_fs_collect_multiple(void) {
  sp_str_t subdir = sp_fs_join_path(g_test_dir, SP_LIT("multi_collect"));
  sp_fs_create_dir(subdir);
  sp_fs_create_file(sp_fs_join_path(subdir, SP_LIT("a.txt")));
  sp_fs_create_file(sp_fs_join_path(subdir, SP_LIT("b.txt")));

  LEAK_TEST_BEGIN("sp_fs_collect (x5)") {
    for (int i = 0; i < 5; i++) {
      sp_da(sp_os_dir_ent_t) entries = sp_fs_collect(subdir);
      (void)entries;
    }
  } LEAK_TEST_END();
}

static void print_results(void) {
  printf("\n");
  printf("=== FILESYSTEM LEAK TEST RESULTS ===\n");
  printf("\n");

  u32 total_leaked = 0;
  u32 leaking_functions = 0;

  for (u32 i = 0; i < g_result_count; i++) {
    leak_result_t* r = &g_results[i];

    if (r->leaked > 0) {
      printf("LEAK %-35s leaked %5u bytes (before=%u, after=%u)\n",
        r->name,
        r->leaked,
        r->bytes_before,
        r->bytes_after
      );
      total_leaked += r->leaked;
      leaking_functions++;
    } else {
      printf("OK   %-35s no leak\n", r->name);
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
  printf("Filesystem Memory Leak Analysis\n");
  printf("Testing filesystem API functions for internal memory leaks...\n");
  printf("\n");

  // Setup test directory (uses default allocator, not scratch)
  setup_test_dir();

  // Run all tests
  test_fs_exists();
  test_fs_is_regular_file();
  test_fs_is_dir();
  test_fs_is_symlink();
  test_fs_create_dir();
  test_fs_remove_dir();
  test_fs_create_file();
  test_fs_remove_file();
  test_fs_collect();
  test_fs_collect_recursive();
  test_fs_canonicalize_path();
  test_fs_normalize_path();
  test_fs_join_path();
  test_fs_parent_path();
  test_fs_get_name();
  test_fs_get_ext();
  test_fs_get_stem();
  test_fs_get_cwd();
  test_fs_get_exe_path();
  test_fs_get_mod_time();
  test_fs_get_file_attrs();
  test_fs_create_hard_link();
  test_fs_create_sym_link();
  test_fs_copy_file();
  test_fs_is_target_regular_file();
  test_fs_is_target_dir();
  test_fs_get_storage_path();
  test_fs_get_config_path();

  // Multiple-call tests to show accumulation
  test_fs_exists_multiple();
  test_fs_collect_multiple();

  // Print results
  print_results();

  // Cleanup
  cleanup_test_dir();

  return 0;
}
