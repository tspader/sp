#include "fs.h"

#define DEEP_LEVELS 40
#define DEEP_NAME "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" \
                  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

typedef struct {
  sp_err_t err;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  bool dir;
  const c8* cwd;
  const c8* path;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "file_basic",
    .setup = {
      { "A" },
    },
    .path = "A",
    .expect = {
      .paths = {
        { .path = "A" },
      },
    },
  },
  {
    .name = "file_missing",
    .path = "A",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "file_is_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .path = "A",
    .expect = {
      .err = SP_ERR_SYS_IS_DIR,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "dir_recursive",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { "A/C", FS_SETUP_DIR },
      { "A/C/D" },
      { "A/E", FS_SETUP_DIR },
    },
    .dir = true,
    .path = "A",
    .expect = {
      .paths = {
        { .path = "A" },
        { .path = "A/B" },
        { .path = "A/C" },
        { .path = "A/C/D" },
        { .path = "A/E" },
      },
    },
  },
  {
    .name = "dir_multi_component_sub",
    .setup = {
      { "A/B/C" },
    },
    .dir = true,
    .path = "A/B",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B" },
      },
    },
  },
  {
    .name = "dir_unicode_child_round_trips",
    .setup = {
      { "\xc3\xa4\x62\x63", FS_SETUP_DIR },
      { "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt" },
    },
    .dir = true,
    .path = "\xc3\xa4\x62\x63",
    .expect = {
      .paths = {
        { .path = "\xc3\xa4\x62\x63" },
        { .path = "\xc3\xa4\x62\x63/\xc3\xbc\x66\x69\x6c\x65.txt" },
      },
    },
  },
  {
    .name = "dir_missing",
    .dir = true,
    .path = "A",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "dir_is_file",
    .setup = {
      { "A" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "dir_dot_rejected",
    .setup = {
      { "A/B" },
    },
    .dir = true,
    .cwd = "A",
    .path = ".",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "dir_dotdot_rejected",
    .setup = {
      { "A/B" },
    },
    .dir = true,
    .path = "A/..",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
#if defined(SP_WIN32)
  {
    .name = "dir_dotdot_backslash_rejected",
    .setup = {
      { "A/B" },
    },
    .dir = true,
    .path = "A\\..",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
#endif
  {
    .name = "dir_empty_rejected",
    .setup = {
      { "A/B" },
    },
    .dir = true,
    .cwd = "A",
    .path = "",
    .expect = {
      .err = SP_ERR_SYS_INVALID,
      .paths = {
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "dir_does_not_follow_symlink",
    .setup = {
      { "B", FS_SETUP_DIR },
      { "B/C" },
      { "A", FS_SETUP_DIR },
      { .path = "A/L", .kind = FS_SETUP_DIR_SYMLINK, .target = "../B" },
      { "A/D" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      .paths = {
        { .path = "A" },
        { .path = "A/L" },
        { .path = "A/D" },
        { .path = "B", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "B/C", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "dir_symlink_root_removes_link_only",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .dir = true,
    .path = "L",
    .expect = {
      .paths = {
        { .path = "L" },
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "dir_symlink_root_trailing_sep_removes_link_only",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .dir = true,
    .path = "L/",
    .expect = {
      .paths = {
        { .path = "L" },
        { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
      },
    },
  },
#if defined(SP_POSIX)
  {
    .name = "dir_with_fifo",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/F", .kind = FS_SETUP_FIFO },
      { "A/B" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      .paths = {
        { .path = "A" },
        { .path = "A/F" },
        { .path = "A/B" },
      },
    },
  },
#endif
};

sp_test_each(fs, remove, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_path_resolve(sp_test_dir(t));
  fs_apply_setup(t, sandbox, it->setup);

  sp_path_t base = it->cwd ? sp_path_join(mem, sandbox, sp_cstr_as_str(it->cwd)) : sandbox;
  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(base.dir, base.sub, 0, &dir));

  sp_path_t path = { .dir = dir, .sub = sp_cstr_as_str(it->path) };
  sp_err_t result = it->dir ? sp_fs_remove_dir_at(path) : sp_fs_remove_file_at(path);
  sp_expect_err_eq(t, result, it->expect.err);

  sp_sys_file_meta_t meta = sp_zero;
  sp_expect_ok(t, sp_sys_get_file_metadata(dir, &meta));
  sp_sys_close(dir);

  fs_expect_paths(t, sandbox, it->expect.paths);
  return SP_OK;
}

sp_test(fs, remove_dir_deeper_than_path_max_is_name_too_long) {
  sp_path_t root = sp_path_join(sp_test_arena(t), sp_path_resolve(sp_test_dir(t)), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_create_dir_at(root));

  sp_sys_fd_t cur = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(root.dir, root.sub, 0, &cur));
  sp_for(level, DEEP_LEVELS) {
    sp_path_t child = { .dir = cur, .sub = sp_str_lit(DEEP_NAME) };
    sp_must_ok(t, sp_fs_create_dir_at(child));

    sp_sys_fd_t next = SP_SYS_INVALID_FD;
    sp_must_ok(t, sp_sys_open_dir_s(child.dir, child.sub, 0, &next));
    sp_sys_close(cur);
    cur = next;
  }
  sp_must_ok(t, sp_fs_create_file_at((sp_path_t) { .dir = cur, .sub = sp_str_lit("F") }));
  sp_sys_close(cur);

  sp_expect_err_eq(t, sp_fs_remove_dir_at(root), SP_ERR_SYS_NAME_TOO_LONG);
  sp_expect(t, sp_fs_exists_at(root));

  sp_must_ok(t, sp_sys_open_dir_s(root.dir, root.sub, 0, &cur));
  sp_for(level, DEEP_LEVELS / 2) {
    sp_sys_fd_t next = SP_SYS_INVALID_FD;
    sp_must_ok(t, sp_sys_open_dir_s(cur, sp_str_lit(DEEP_NAME), 0, &next));
    sp_sys_close(cur);
    cur = next;
  }
  sp_expect_ok(t, sp_fs_remove_dir_at((sp_path_t) { .dir = cur, .sub = sp_str_lit(DEEP_NAME) }));
  sp_sys_close(cur);

  sp_expect_ok(t, sp_fs_remove_dir_at(root));
  sp_expect(t, !sp_fs_exists_at(root));
  return SP_OK;
}

#if defined(SP_POSIX)
sp_test(fs, remove_dir_unwritable_subdir_fails) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_path_resolve(sp_test_dir(t));
  sp_path_t locked = sp_path_join(mem, sandbox, sp_str_lit("A/B"));
  sp_must_ok(t, sp_fs_create_dir_at(locked));
  sp_must_ok(t, sp_fs_create_file_at(sp_path_join(mem, sandbox, sp_str_lit("A/B/C"))));
  sp_must_ok(t, sp_sys_set_file_perms_s(locked.dir, locked.sub, (sp_sys_file_perms_t) { .value = 0555 }));

  sp_err_t result = sp_fs_remove_dir_at(sp_path_join(mem, sandbox, sp_str_lit("A")));

  if (!result) return sp_test_skip(t, "directory permissions not enforced");
  sp_must_ok(t, sp_sys_set_file_perms_s(locked.dir, locked.sub, (sp_sys_file_perms_t) { .value = 0755 }));
  sp_expect_err_eq(t, result, SP_ERR_SYS_ACCESS_DENIED);
  fs_expect_paths(t, sandbox, (const fs_expected_path_t [FS_MAX_PATHS]) {
    { .path = "A/B", .exists = true, .kind = SP_FS_KIND_DIR },
    { .path = "A/B/C", .exists = true, .kind = SP_FS_KIND_FILE },
  });
  return SP_OK;
}
#endif
