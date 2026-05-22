#include "fs.h"
#include "utest.h"

typedef enum {
  FD_REL_GET_METADATA,
  FD_REL_OPEN_FROM_RELATIVE_PATH,
  FD_REL_OPEN_FROM_ABSOLUTE_PATH,
  FD_REL_OP_RENAME,
  FD_REL_OP_LINK,
} fd_rel_op_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  struct {
    const c8* cwd;
    fd_rel_op_t kind;
    union {
      struct { const c8* path; } metadata;
      struct { const c8* path; const c8* content; } open;
      struct { const c8* source; const c8* dest; } rename;
      struct { const c8* source; const c8* dest; } link;
    };
  } op;
  fs_expected_path_t expected[16];
} fd_rel_test_t;

static void run_fd_rel_test(s32* utest_result, sp_test_file_manager_t* fs, fd_rel_test_t t) {
  sp_mem_t mem = fs->mem;
  sp_str_t sandbox = sp_test_file_create_dir(fs, t.label);
  fs_apply_setup(utest_result, fs, sandbox, t.setup);

  struct {
    sp_str_t path;
    sp_sys_fd_t fd;
  } cwd = sp_zero;
  cwd.path = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(t.op.cwd));
  cwd.fd = sp_sys_open_s(sp_sys_get_root(0), cwd.path, SP_O_RDONLY | SP_O_DIRECTORY, 0);
  ASSERT_NE(cwd.fd, SP_SYS_INVALID_FD);

  switch (t.op.kind) {
    case FD_REL_GET_METADATA: {
      sp_str_t path = sp_cstr_as_str(t.op.metadata.path);
      sp_sys_file_meta_t metadata = sp_zero;
      EXPECT_OK(sp_sys_get_path_metadata_s(cwd.fd, path, &metadata));
      break;
    }
    case FD_REL_OPEN_FROM_ABSOLUTE_PATH:
    case FD_REL_OPEN_FROM_RELATIVE_PATH: {
      sp_str_t path = t.op.kind == FD_REL_OPEN_FROM_ABSOLUTE_PATH
        ? sp_fs_join_path(mem, sandbox, sp_cstr_as_str(t.op.open.path))
        : sp_cstr_as_str(t.op.open.path);
      sp_sys_fd_t fd = sp_sys_open_s(cwd.fd, path, SP_O_RDONLY | SP_O_BINARY, 0);
      EXPECT_NE(fd, SP_SYS_INVALID_FD);

      if (fd != SP_SYS_INVALID_FD) {
        if (t.op.open.content) {
          c8 buf[64] = sp_zero;
          s64 n = sp_sys_read(fd, buf, sizeof(buf));
          EXPECT_GE(n, 0);
          EXPECT_TRUE(sp_mem_is_equal(buf, t.op.open.content, sp_max(n, 0)));
        }
        sp_sys_close(fd);
      }
      break;
    }
    case FD_REL_OP_RENAME: {
      EXPECT_EQ(sp_sys_rename_s(cwd.fd, sp_cstr_as_str(t.op.rename.source), cwd.fd, sp_cstr_as_str(t.op.rename.dest)), 0);
      break;
    }
    case FD_REL_OP_LINK: {
      EXPECT_EQ(sp_sys_link_s(cwd.fd, sp_cstr_as_str(t.op.link.source), cwd.fd, sp_cstr_as_str(t.op.link.dest)), 0);
      break;
    }
  }

  sp_sys_close(cwd.fd);
  fs_expect_paths(utest_result, fs, sandbox, t.expected);
}

UTEST_F(fs, fd_relative_stat_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_stat_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .op = {
      .cwd = "a",
      .kind = FD_REL_GET_METADATA,
      .metadata = {
        .path = "../a/file.txt",
      }
    }
  });
}

UTEST_F(fs, fd_relative_open_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .op = {
      .cwd = "a",
      .kind = FD_REL_OPEN_FROM_RELATIVE_PATH,
      .open = {
        .path = "../a/file.txt",
        .content = "hello"
      }
    }
  });
}

UTEST_F(fs, fd_relative_rename_dotdot) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_rename_dotdot",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .op = {
      .cwd = "a",
      .kind = FD_REL_OP_RENAME,
      .rename = {
        .source = "file.txt",
        .dest = "../a/file2.txt",
      }
    },
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
    .op = {
      .cwd = "a",
      .kind = FD_REL_OP_LINK,
      .link = {
        .source = "file.txt",
        .dest = "../a/file.hard",
      }
    },
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
    .op = {
      .cwd = "a",
      .kind = FD_REL_OPEN_FROM_RELATIVE_PATH,
      .open = {
        .path = "..\\a/file.txt",
      }
    }
  });
}

UTEST_F(fs, fd_relative_open_dot_segments) {
  SKIP_ON_WASM()
  run_fd_rel_test(&ur, &ut.file_manager, (fd_rel_test_t) {
    .label = "fd_relative_open_dot_segments",
    .setup = {
      { .path = "a/file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .op = {
      .cwd = "a",
      .kind = FD_REL_OPEN_FROM_RELATIVE_PATH,
      .open = {
        .path = "./././file.txt",
      }
    }
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
    .op = {
      .cwd = "a",
      .kind = FD_REL_OPEN_FROM_ABSOLUTE_PATH,
      .open = {
        .path = "b/file.txt",
        .content = "world",
      }
    }
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
    .op = {
      .cwd = "a/b",
      .kind = FD_REL_OPEN_FROM_RELATIVE_PATH,
      .open = {
        .path = "../sibling.txt",
        .content = "side",
      }
    }
  });
}
