#include "fs.h"

typedef enum {
  OP_OPEN,
  OP_OPEN_ABS,
  OP_STAT,
  OP_RENAME,
  OP_LINK,
} op_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* cwd;
  op_t op;
  const c8* path;
  const c8* dest;
  const c8* content;
  bool win32;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "stat_dotdot",
    .setup = {
      { .path = "A/F", .content = "A" },
    },
    .cwd = "A",
    .op = OP_STAT,
    .path = "../A/F",
  },
  {
    .name = "open_dotdot",
    .setup = {
      { .path = "A/F", .content = "A" },
    },
    .cwd = "A",
    .path = "../A/F",
    .content = "A",
  },
  {
    .name = "rename_dotdot",
    .setup = {
      { .path = "A/F", .content = "A" },
    },
    .cwd = "A",
    .op = OP_RENAME,
    .path = "F",
    .dest = "../A/G",
    .expect = {
      { .path = "A/F" },
      { .path = "A/G", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "link_dotdot",
    .setup = {
      { .path = "A/F", .content = "A" },
    },
    .cwd = "A",
    .op = OP_LINK,
    .path = "F",
    .dest = "../A/G",
    .expect = {
      { .path = "A/G", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "open_mixed_separators",
    .setup = {
      { .path = "A/F", .content = "A" },
    },
    .cwd = "A",
    .path = "..\\A/F",
    .win32 = true,
  },
  {
    .name = "open_dot_segments",
    .setup = {
      { .path = "A/F", .content = "A" },
    },
    .cwd = "A",
    .path = "./././F",
  },
  {
    .name = "open_absolute_ignores_fd",
    .setup = {
      { .path = "A/F", .content = "A" },
      { .path = "B/F", .content = "B" },
    },
    .cwd = "A",
    .op = OP_OPEN_ABS,
    .path = "B/F",
    .content = "B",
  },
  {
    .name = "open_deep_dotdot",
    .setup = {
      { .path = "A/B/F", .content = "A" },
      { .path = "A/G", .content = "B" },
    },
    .cwd = "A/B",
    .path = "../G",
    .content = "B",
  },
};

sp_test_each(fs, fd_relative, test_t, tests) {
#if !defined(SP_WIN32)
  if (it->win32) return sp_test_skip(t, "windows only");
#endif
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t cwd_path = sp_fs_join_path(mem, sandbox, sp_str_view(it->cwd));
  sp_sys_fd_t cwd = SP_SYS_INVALID_FD;
  if (sp_sys_open_dir_s(sp_sys_get_root(0), cwd_path, &cwd) != SP_OK) {
    sp_test_fail(t, "failed to open dir {}", sp_fmt_str(cwd_path));
    return SP_ERR;
  }

  switch (it->op) {
    case OP_STAT: {
      sp_sys_file_meta_t metadata = sp_zero;
      sp_expect_ok(t, sp_sys_get_path_metadata_s(cwd, sp_str_view(it->path), &metadata));
      break;
    }
    case OP_OPEN:
    case OP_OPEN_ABS: {
      sp_str_t path = it->op == OP_OPEN_ABS
        ? sp_fs_join_path(mem, sandbox, sp_str_view(it->path))
        : sp_str_view(it->path);
      sp_sys_fd_t fd = SP_SYS_INVALID_FD;
      sp_expect_ok(t, sp_sys_open_s(cwd, path, SP_SYS_OPEN_MODE_RO, 0, &fd));

      if (fd != SP_SYS_INVALID_FD) {
        if (it->content) {
          c8 buf [64] = sp_zero;
          u64 n = 0;
          sp_expect_ok(t, sp_sys_read(fd, buf, sizeof(buf), &n));
          sp_expect(t, sp_mem_is_equal(buf, it->content, n));
        }
        sp_sys_close(fd);
      }
      break;
    }
    case OP_RENAME: {
      sp_expect_ok(t, sp_sys_rename_s(cwd, sp_str_view(it->path), cwd, sp_str_view(it->dest)));
      break;
    }
    case OP_LINK: {
      sp_expect_ok(t, sp_sys_link_s(cwd, sp_str_view(it->path), cwd, sp_str_view(it->dest)));
      break;
    }
  }

  sp_sys_close(cwd);
  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}
