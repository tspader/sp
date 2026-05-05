#include "sp.h"
#include "test.h"
#include "utest.h"
#include "process.h"

SP_TEST_MAIN()

struct leak {
  sp_test_file_manager_t fs;
  sp_mem_tracking_t tracker;
  sp_mem_t mem;
};

UTEST_F_SETUP(leak) {
  sp_test_file_manager_init(&ut.fs);
  sp_mem_tracking_init(&ut.tracker);
  ut.mem = sp_mem_tracking_as_allocator(&ut.tracker);
}

UTEST_F_TEARDOWN(leak) {
  if (sp_mem_tracking_live_count(&ut.tracker) != 0) {
    sp_mem_tracking_dump(&ut.tracker);
  }
  EXPECT_EQ(sp_mem_tracking_live_count(&ut.tracker), 0u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&ut.tracker), 0u);

  sp_test_file_manager_cleanup(&ut.fs);
  sp_mem_tracking_deinit(&ut.tracker);
}

UTEST_F(leak, hello) {
  EXPECT_NE(69, 420);
}

//////////////////////////
// TRACKING ALLOCATOR //
//////////////////////////

UTEST(tracking, alloc_free_balance) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  void* p = sp_alloc_a(mem, 64);
  EXPECT_NE(p, SP_NULLPTR);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 1u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 64u);

  sp_free_a(mem, p);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_double_frees(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_wild_frees(&t), 0u);

  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, detects_leak) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  sp_alloc_a(mem, 16);
  sp_alloc_a(mem, 32);
  void* freed = sp_alloc_a(mem, 8);
  sp_free_a(mem, freed);

  EXPECT_EQ(sp_mem_tracking_live_count(&t), 2u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 48u);

  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, detects_double_free) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  void* p = sp_alloc_a(mem, 64);
  sp_free_a(mem, p);
  sp_free_a(mem, p);
  sp_free_a(mem, p);

  EXPECT_EQ(sp_mem_tracking_double_frees(&t), 2u);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 0u);

  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, detects_wild_free) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  // A buffer that wasn't allocated through the tracker. Reading the magic
  // field at &buf[200 - sizeof(node)] is well-defined since the buffer is
  // large enough for the back-step, and the bytes there won't match either
  // sentinel (we zero-init the buffer).
  static u8 buf[256] = {0};
  sp_free_a(mem, &buf[200]);

  EXPECT_EQ(sp_mem_tracking_wild_frees(&t), 1u);
  EXPECT_EQ(sp_mem_tracking_double_frees(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 0u);

  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, free_null_is_noop) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  sp_free_a(mem, SP_NULLPTR);

  EXPECT_EQ(sp_mem_tracking_double_frees(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_wild_frees(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 0u);

  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, realloc_grows_and_preserves) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  u8* p = sp_alloc_n_a(mem, u8, 4);
  sp_for(i, 4) p[i] = (u8)(i + 1);

  u8* g = sp_realloc_a(mem, p, 64);
  EXPECT_NE(g, SP_NULLPTR);
  sp_for(i, 4) EXPECT_EQ(g[i], (u8)(i + 1));

  EXPECT_EQ(sp_mem_tracking_live_count(&t), 1u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 64u);

  sp_free_a(mem, g);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 0u);

  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, realloc_null_is_alloc) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  void* p = sp_realloc_a(mem, SP_NULLPTR, 32);
  EXPECT_NE(p, SP_NULLPTR);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 1u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 32u);

  sp_free_a(mem, p);
  sp_mem_tracking_deinit(&t);
}

UTEST(tracking, realloc_zero_is_free) {
  sp_mem_tracking_t t;
  sp_mem_tracking_init(&t);
  sp_mem_t mem = sp_mem_tracking_as_allocator(&t);

  void* p = sp_alloc_a(mem, 32);
  void* r = sp_realloc_a(mem, p, 0);
  EXPECT_EQ(r, SP_NULLPTR);
  EXPECT_EQ(sp_mem_tracking_live_count(&t), 0u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&t), 0u);

  sp_mem_tracking_deinit(&t);
}

//////////////
// SP_PS LEAK //
//////////////
static sp_str_t leak_ps_get_process_path(sp_mem_t mem) {
  sp_str_t exe = sp_fs_parent_path(sp_fs_get_exe_path_a(mem));
  sp_str_t process = sp_fs_join_path_a(mem, exe, sp_str_lit("process"));
  return sp_fs_replace_ext_a(mem, process, sp_os_get_executable_ext());
}

UTEST_F(leak, ps_create_wait_free_balances) {
  SKIP_ON_FREESTANDING();
  SKIP_ON_WASM();
  sp_str_t process = leak_ps_get_process_path(sp_mem_get_scratch());

  sp_ps_config_t config = {
    .command = process,
    .args = {
      sp_str_lit("--fn"),         sp_str_lit("exit_code"),
      sp_str_lit("--exit-code"),  sp_str_lit("0"),
    },
  };

  sp_ps_t ps = sp_ps_create_a(ut.mem, config);
  EXPECT_NE(ps.os, SP_NULLPTR);

  sp_ps_status_t status = sp_ps_wait(&ps);
  EXPECT_EQ(status.exit_code, 0);

  sp_ps_free(&ps);
}

UTEST_F(leak, multiple_allocs_independent) {
  void* a = sp_alloc_a(ut.mem, 8);
  void* b = sp_alloc_a(ut.mem, 16);
  void* c = sp_alloc_a(ut.mem, 32);
  EXPECT_EQ(sp_mem_tracking_live_count(&ut.tracker), 3u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&ut.tracker), 56u);

  sp_free_a(ut.mem, b);
  EXPECT_EQ(sp_mem_tracking_live_count(&ut.tracker), 2u);
  EXPECT_EQ(sp_mem_tracking_live_bytes(&ut.tracker), 40u);

  sp_free_a(ut.mem, a);
  sp_free_a(ut.mem, c);
  EXPECT_EQ(sp_mem_tracking_live_count(&ut.tracker), 0u);
}
