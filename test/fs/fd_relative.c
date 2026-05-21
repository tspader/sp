#include "fs.h"

typedef enum {
  FD_REL_STAT,
  FD_REL_OPEN,
  FD_REL_RENAME,
  FD_REL_LINK,
} fd_rel_op_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  const c8* dir;            // directory opened to obtain the relative fd
  fd_rel_op_t op;
  const c8* path;           // STAT/OPEN target, or RENAME/LINK source (relative to dir_fd)
  const c8* dst;            // RENAME/LINK destination (relative to dir_fd)
  bool absolute;            // OPEN: join path against the sandbox so the absolute path ignores the fd
  const c8* expect_content; // OPEN: file contents read back through the fd
  fs_expected_path_t expected[16];
} fd_rel_test_t;

static void run_fd_rel_test(s32* utest_result, sp_test_file_manager_t* fm, fd_rel_test_t t) {
  sp_mem_t a = fm->mem;
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t.label));
  sp_fs_create_dir(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t.setup);

  sp_str_t dir = sp_fs_join_path(a, sandbox, sp_str_view(t.dir));
  sp_sys_fd_t dir_fd = sp_sys_open_s(sp_sys_get_root(0), dir, SP_O_RDONLY | SP_O_DIRECTORY, 0);
  ASSERT_NE(dir_fd, SP_SYS_INVALID_FD);

  switch (t.op) {
    case FD_REL_STAT: {
      sp_sys_file_meta_t st = sp_zero;
      EXPECT_EQ(sp_sys_get_path_metadata_s(dir_fd, sp_str_view(t.path), &st), 0);
      break;
    }
    case FD_REL_OPEN: {
      sp_str_t path = t.absolute
        ? sp_fs_join_path(a, sandbox, sp_str_view(t.path))
        : sp_str_view(t.path);
      sp_sys_fd_t file_fd = sp_sys_open_s(dir_fd, path, SP_O_RDONLY | SP_O_BINARY, 0);
      EXPECT_NE(file_fd, SP_SYS_INVALID_FD);
      if (file_fd != SP_SYS_INVALID_FD) {
        if (t.expect_content) {
          c8 buf[64] = sp_zero;
          s64 n = sp_sys_read(file_fd, buf, sizeof(buf));
          SP_EXPECT_STR_EQ_CSTR(sp_str(buf, n < 0 ? 0 : (u32)n), t.expect_content);
        }
        sp_sys_close(file_fd);
      }
      break;
    }
    case FD_REL_RENAME: {
      EXPECT_EQ(sp_sys_rename_s(dir_fd, sp_str_view(t.path), dir_fd, sp_str_view(t.dst)), 0);
      break;
    }
    case FD_REL_LINK: {
      EXPECT_EQ(sp_sys_link_s(dir_fd, sp_str_view(t.path), dir_fd, sp_str_view(t.dst)), 0);
      break;
    }
  }

  sp_sys_close(dir_fd);
  fs_expect_paths(utest_result, fm, sandbox, t.expected);
}

UTEST_F(fs, fd_relative_stat_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_stat_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .dir = "a",
    .op = FD_REL_STAT,
    .path = "../a/file.txt",
  });
}

UTEST_F(fs, fd_relative_open_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .dir = "a",
    .op = FD_REL_OPEN,
    .path = "../a/file.txt",
    .expect_content = "hello",
  });
}

UTEST_F(fs, fd_relative_rename_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_rename_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .dir = "a",
    .op = FD_REL_RENAME,
    .path = "file.txt",
    .dst = "../a/file2.txt",
    .expected = {
      { .path = "a/file.txt",  .exists = FS_EXPECT_NOT_EXIST, .attr = SP_FS_KIND_NONE },
      { .path = "a/file2.txt", .exists = FS_EXPECT_EXIST,     .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, fd_relative_link_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_link_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .dir = "a",
    .op = FD_REL_LINK,
    .path = "file.txt",
    .dst = "../a/file.hard",
    .expected = {
      { .path = "a/file.hard", .exists = FS_EXPECT_EXIST, .attr = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, fd_relative_open_mixed_separators) {
  SKIP_UNLESS_WIN32()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_mixed_separators",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .dir = "a",
    .op = FD_REL_OPEN,
    .path = "..\\a/file.txt",
  });
}

UTEST_F(fs, fd_relative_open_dot_segments) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_dot_segments",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .dir = "a",
    .op = FD_REL_OPEN,
    .path = "./././file.txt",
  });
}

UTEST_F(fs, fd_relative_open_absolute_ignores_fd) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_absolute_ignores_fd",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
      { .path = "b/file.txt", .kind = FS_SETUP_FILE, .content = "world" },
    },
    .dir = "a",
    .op = FD_REL_OPEN,
    .path = "b/file.txt",
    .absolute = true,
    .expect_content = "world",
  });
}

UTEST_F(fs, fd_relative_open_deep_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_deep_dotdot",
    .setup = {
      { .path = "a/b/inner.txt", .kind = FS_SETUP_FILE, .content = "deep" },
      { .path = "a/sibling.txt", .kind = FS_SETUP_FILE, .content = "side" },
    },
    .dir = "a/b",
    .op = FD_REL_OPEN,
    .path = "../sibling.txt",
    .expect_content = "side",
  });
}
