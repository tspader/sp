#include "test.h"
#include "utest.h"

UTEST_EMPTY_FIXTURE(sys_posix)

UTEST_F(sys_posix, symlink_honors_dirfd) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t sandbox = sp_test_file_create_dir(&fm, "sys_posix_symlink_dirfd");

  sp_sys_fd_t dir = sp_sys_open_s(sp_sys_get_root(0), sandbox, SP_O_RDONLY | SP_O_DIRECTORY, 0);
  ASSERT_NE(dir, SP_SYS_INVALID_FD);

  if (sp_sys_symlink_s(sp_str_lit("target"), dir, sp_str_lit("lnk"))) {
    sp_sys_close(dir);
    sp_test_file_manager_cleanup(&fm);
    UTEST_SKIP("symlink creation unavailable");
  }

  sp_sys_file_meta_t meta = sp_zero;
  EXPECT_EQ(sp_sys_get_link_metadata_s(dir, sp_str_lit("lnk"), &meta), 0);

  sp_sys_unlink_s(dir, sp_str_lit("lnk"));
  sp_fs_remove_file(sp_str_lit("lnk"));
  sp_sys_close(dir);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_posix, pread_preserves_file_position) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_path_c(&fm, "sys_posix_pread.bin");
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = path,
    .content = sp_str_lit("0123456789ABCDEF"),
  });

  sp_sys_fd_t fd = sp_sys_open_s(sp_sys_get_root(0), path, SP_O_RDONLY | SP_O_BINARY, 0);
  ASSERT_NE(fd, SP_SYS_INVALID_FD);

  c8 head [4] = sp_zero;
  c8 positional [4] = sp_zero;
  c8 next [4] = sp_zero;
  EXPECT_EQ(sp_sys_read(fd, head, 4), 4);
  EXPECT_EQ(sp_sys_pread(fd, positional, 4, 8), 4);
  EXPECT_EQ(sp_sys_read(fd, next, 4), 4);

  const c8* expected = "4567";
  sp_for(it, 4) EXPECT_EQ(next[it], expected[it]);

  sp_sys_close(fd);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_posix, read_clamps_count_at_4gib) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_path_c(&fm, "sys_posix_clamp.bin");
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = path,
    .content = sp_str_lit("0123456789ABCDEF"),
  });

  sp_sys_fd_t fd = sp_sys_open_s(sp_sys_get_root(0), path, SP_O_RDONLY | SP_O_BINARY, 0);
  ASSERT_NE(fd, SP_SYS_INVALID_FD);

  c8 buf [64] = sp_zero;
  s64 n = sp_sys_read(fd, buf, 0x100000000ULL);
  EXPECT_EQ(n, 16);

  sp_sys_close(fd);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_posix, append_writes_at_current_eof) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t path = sp_test_file_path_c(&fm, "sys_posix_append.bin");
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = path,
    .content = sp_str_lit("AAAA"),
  });

  sp_sys_fd_t append_fd = sp_sys_open_s(sp_sys_get_root(0), path, SP_O_WRONLY | SP_O_APPEND | SP_O_BINARY, 0);
  ASSERT_NE(append_fd, SP_SYS_INVALID_FD);

  sp_sys_fd_t plain_fd = sp_sys_open_s(sp_sys_get_root(0), path, SP_O_WRONLY | SP_O_BINARY, 0);
  ASSERT_NE(plain_fd, SP_SYS_INVALID_FD);
  EXPECT_EQ(sp_sys_pwrite(plain_fd, "BBBB", 4, 4), 4);
  sp_sys_close(plain_fd);

  EXPECT_EQ(sp_sys_write(append_fd, "CC", 2), 2);
  sp_sys_close(append_fd);

  sp_sys_fd_t fd = sp_sys_open_s(sp_sys_get_root(0), path, SP_O_RDONLY | SP_O_BINARY, 0);
  ASSERT_NE(fd, SP_SYS_INVALID_FD);
  c8 buf [16] = sp_zero;
  EXPECT_EQ(sp_sys_read(fd, buf, sizeof(buf)), 10);

  const c8* expected = "AAAABBBBCC";
  sp_for(it, 10) EXPECT_EQ(buf[it], expected[it]);

  sp_sys_close(fd);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_posix, open_excl_refuses_symlink) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t sandbox = sp_test_file_create_dir(&fm, "sys_posix_excl");
  sp_str_t lnk = sp_fs_join_path(fm.mem, sandbox, sp_str_lit("lnk"));
  sp_str_t victim = sp_fs_join_path(fm.mem, sandbox, sp_str_lit("victim"));

  if (sp_sys_symlink_s(sp_str_lit("victim"), sp_sys_get_root(0), lnk)) {
    sp_test_file_manager_cleanup(&fm);
    UTEST_SKIP("symlink creation unavailable");
  }

  sp_sys_fd_t fd = sp_sys_open_s(sp_sys_get_root(0), lnk, SP_O_CREAT | SP_O_EXCL | SP_O_WRONLY | SP_O_BINARY, 0644);
  EXPECT_EQ(fd, SP_SYS_INVALID_FD);
  if (fd != SP_SYS_INVALID_FD) sp_sys_close(fd);

  EXPECT_FALSE(sp_fs_exists(victim));

  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_posix, rename_replaces_readonly_target) {
  SKIP_ON_WASM()
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);
  sp_str_t src = sp_test_file_path_c(&fm, "sys_posix_rename_src.bin");
  sp_str_t dst = sp_test_file_path_c(&fm, "sys_posix_rename_dst.bin");
  sp_test_file_create_ex((sp_test_file_config_t) { .path = src, .content = sp_str_lit("new!") });
  sp_test_file_create_ex((sp_test_file_config_t) { .path = dst, .content = sp_str_lit("old!") });

  sp_sys_file_meta_t meta = sp_zero;
  ASSERT_EQ(sp_sys_get_path_metadata_s(sp_sys_get_root(0), dst, &meta), 0);
#if defined(SP_WIN32)
  meta.raw_attrs |= FILE_ATTRIBUTE_READONLY;
#else
  meta.raw_attrs &= ~(u32)0222;
#endif
  ASSERT_EQ(sp_sys_chmod_s(sp_sys_get_root(0), dst, &meta), 0);

  EXPECT_EQ(sp_sys_rename_s(sp_sys_get_root(0), src, sp_sys_get_root(0), dst), 0);

  sp_sys_fd_t fd = sp_sys_open_s(sp_sys_get_root(0), dst, SP_O_RDONLY | SP_O_BINARY, 0);
  EXPECT_NE(fd, SP_SYS_INVALID_FD);
  if (fd != SP_SYS_INVALID_FD) {
    c8 buf [8] = sp_zero;
    EXPECT_EQ(sp_sys_read(fd, buf, sizeof(buf)), 4);
    const c8* expected = "new!";
    sp_for(it, 4) EXPECT_EQ(buf[it], expected[it]);
    sp_sys_close(fd);
  }

  sp_sys_file_meta_t restore = sp_zero;
  if (!sp_sys_get_path_metadata_s(sp_sys_get_root(0), dst, &restore)) {
#if defined(SP_WIN32)
    restore.raw_attrs = 0;
#else
    restore.raw_attrs |= (u32)0200;
#endif
    sp_sys_chmod_s(sp_sys_get_root(0), dst, &restore);
  }
  sp_test_file_manager_cleanup(&fm);
}
