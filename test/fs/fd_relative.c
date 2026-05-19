#include "fs.h"

static sp_sys_fd_t fd_rel_open_dir(sp_str_t path) {
  return sp_sys_open_s(sp_sys_get_root(0), path, SP_O_RDONLY | SP_O_DIRECTORY, 0);
}

UTEST_F(fs, fd_relative_stat_dotdot) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_stat_dotdot"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  sp_sys_file_meta_t st = sp_zero;
  s32 rc = sp_sys_get_path_metadata_s(dir_fd, sp_str_lit("../a/file.txt"), &st);
  EXPECT_EQ(rc, 0);

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_open_dotdot) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_open_dotdot"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  sp_sys_fd_t file_fd = sp_sys_open_s(dir_fd, sp_str_lit("../a/file.txt"), SP_O_RDONLY | SP_O_BINARY, 0);
  EXPECT_NE(file_fd, SP_SYS_INVALID_FD);

  if (file_fd != SP_SYS_INVALID_FD) {
    c8 buf[16] = sp_zero;
    s64 n = sp_sys_read(file_fd, buf, sizeof(buf));
    EXPECT_EQ((s32)n, 5);
    EXPECT_EQ(buf[0], 'h');
    EXPECT_EQ(buf[4], 'o');
    sp_sys_close(file_fd);
  }

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_rename_dotdot) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_rename_dotdot"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  s32 rc = sp_sys_rename_s(dir_fd, sp_str_lit("file.txt"), dir_fd, sp_str_lit("../a/file2.txt"));
  EXPECT_EQ(rc, 0);

  sp_str_t old_path = sp_fs_join_path(a, sandbox, sp_str_lit("a/file.txt"));
  sp_str_t new_path = sp_fs_join_path(a, sandbox, sp_str_lit("a/file2.txt"));
  EXPECT_FALSE(sp_fs_exists(old_path));
  EXPECT_TRUE(sp_fs_exists(new_path));

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_link_dotdot) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_link_dotdot"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  s32 rc = sp_sys_link_s(dir_fd, sp_str_lit("file.txt"), dir_fd, sp_str_lit("../a/file.hard"));
  EXPECT_EQ(rc, 0);

  sp_str_t link_path = sp_fs_join_path(a, sandbox, sp_str_lit("a/file.hard"));
  EXPECT_TRUE(sp_fs_exists(link_path));

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_open_mixed_separators) {
  SKIP_UNLESS_WIN32()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_open_mixed_separators"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  sp_sys_fd_t file_fd = sp_sys_open_s(dir_fd, sp_str_lit("..\\a/file.txt"), SP_O_RDONLY | SP_O_BINARY, 0);
  EXPECT_NE(file_fd, SP_SYS_INVALID_FD);
  if (file_fd != SP_SYS_INVALID_FD) sp_sys_close(file_fd);

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_open_dot_segments) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_open_dot_segments"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  sp_sys_fd_t file_fd = sp_sys_open_s(dir_fd, sp_str_lit("./././file.txt"), SP_O_RDONLY | SP_O_BINARY, 0);
  EXPECT_NE(file_fd, SP_SYS_INVALID_FD);
  if (file_fd != SP_SYS_INVALID_FD) sp_sys_close(file_fd);

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_open_absolute_ignores_fd) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_open_absolute_ignores_fd"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    { .path = "b/file.txt", .kind = FS_SETUP_FILE, .content = "world" },
    { 0 },
  });

  sp_str_t dir_a = sp_fs_join_path(a, sandbox, sp_str_lit("a"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_a);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  sp_str_t abs_b = sp_fs_join_path(a, sandbox, sp_str_lit("b/file.txt"));
  sp_sys_fd_t file_fd = sp_sys_open_s(dir_fd, abs_b, SP_O_RDONLY | SP_O_BINARY, 0);
  EXPECT_NE(file_fd, SP_SYS_INVALID_FD);

  if (file_fd != SP_SYS_INVALID_FD) {
    c8 buf[16] = sp_zero;
    s64 n = sp_sys_read(file_fd, buf, sizeof(buf));
    EXPECT_EQ((s32)n, 5);
    EXPECT_EQ(buf[0], 'w');
    sp_sys_close(file_fd);
  }

  sp_sys_close(dir_fd);
}

UTEST_F(fs, fd_relative_open_deep_dotdot) {
  SKIP_ON_WASM()
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, sp_str_lit("fd_relative_open_deep_dotdot"));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(&ur, &ut.file_manager, sandbox, (fs_setup_t[]) {
    { .path = "a/b/inner.txt", .kind = FS_SETUP_FILE, .content = "deep" },
    { .path = "a/sibling.txt", .kind = FS_SETUP_FILE, .content = "side" },
    { 0 },
  });

  sp_str_t dir_b = sp_fs_join_path(a, sandbox, sp_str_lit("a/b"));
  sp_sys_fd_t dir_fd = fd_rel_open_dir(dir_b);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  sp_sys_fd_t file_fd = sp_sys_open_s(dir_fd, sp_str_lit("../sibling.txt"), SP_O_RDONLY | SP_O_BINARY, 0);
  EXPECT_NE(file_fd, SP_SYS_INVALID_FD);

  if (file_fd != SP_SYS_INVALID_FD) {
    c8 buf[16] = sp_zero;
    s64 n = sp_sys_read(file_fd, buf, sizeof(buf));
    EXPECT_EQ((s32)n, 4);
    EXPECT_EQ(buf[0], 's');
    sp_sys_close(file_fd);
  }

  sp_sys_close(dir_fd);
}
