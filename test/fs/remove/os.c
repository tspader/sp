#include "fs.h"

#define DEEP_LEVELS 40
#define DEEP_NAME "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" \
                  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  bool dir;
  const c8* cwd;
  const c8* path;
  sp_err_t err;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "file_basic",
    .setup = {
      { "A" },
    },
    .path = "A",
    .expect = {
      { .path = "A" },
    },
  },
  {
    .name = "file_missing",
    .path = "A",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
  {
    .name = "file_is_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .path = "A",
    .err = SP_ERR_SYS_IS_DIR,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
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
      { .path = "A" },
      { .path = "A/B" },
      { .path = "A/C" },
      { .path = "A/C/D" },
      { .path = "A/E" },
    },
  },
  {
    .name = "dir_nested_sub",
    .setup = {
      { "A/B/C" },
    },
    .dir = true,
    .path = "A/B",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "A/B" },
    },
  },
  {
    .name = "dir_missing",
    .dir = true,
    .path = "A",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
  {
    .name = "dir_is_file",
    .setup = {
      { "A" },
    },
    .dir = true,
    .path = "A",
    .err = SP_ERR_SYS_NOT_DIR,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
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
    .err = SP_ERR_SYS_INVALID,
    .expect = {
      { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_dotdot_rejected",
    .setup = {
      { "A/B" },
    },
    .dir = true,
    .path = "A/..",
    .err = SP_ERR_SYS_INVALID,
    .expect = {
      { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_empty_rejected",
    .setup = {
      { "A/B" },
    },
    .dir = true,
    .cwd = "A",
    .path = "",
    .err = SP_ERR_SYS_INVALID,
    .expect = {
      { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_does_not_follow_symlink",
    .setup = {
      { "B" },
      { "A", FS_SETUP_DIR },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "B" },
      { "A/C" },
    },
    .dir = true,
    .path = "A",
    .expect = {
      { .path = "A" },
      { .path = "A/L" },
      { .path = "A/C" },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_symlink_root_removes_link_only",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .dir = true,
    .path = "L",
    .expect = {
      { .path = "L" },
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "dir_symlink_root_trailing_sep_removes_link_only",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .dir = true,
    .path = "L/",
    .expect = {
      { .path = "L" },
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "A/B", .exists = true, .kind = SP_FS_KIND_FILE },
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
      { .path = "A" },
      { .path = "A/F" },
      { .path = "A/B" },
    },
  },
#endif
};

sp_test_each(fs, remove, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t base = it->cwd ? sp_fs_join_path(mem, sandbox, sp_cstr_as_str(it->cwd)) : sandbox;
  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), base, 0, &dir));

  sp_path_t path = { .dir = dir, .sub = sp_cstr_as_str(it->path) };
  sp_err_t result = it->dir ? sp_fs_remove_dir_at(path) : sp_fs_remove_file_at(path);
  sp_expect_err_eq(t, result, it->err);

  sp_sys_file_meta_t meta = sp_zero;
  sp_expect_ok(t, sp_sys_get_file_metadata(dir, &meta));
  sp_sys_close(dir);

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}

sp_test(fs, remove_dir_deeper_than_path_max) {
  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sp_test_dir(t), 0, &dir));

  sp_path_t root = { .dir = dir, .sub = sp_str_lit("A") };
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

  sp_expect_ok(t, sp_fs_remove_dir_at(root));
  sp_expect(t, !sp_fs_exists_at(root));
  sp_sys_close(dir);
  return SP_OK;
}

#if defined(SP_POSIX)
sp_test(fs, remove_dir_unwritable_subdir_fails) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t locked = fs_path_c(t, "A/B");
  sp_must_ok(t, sp_fs_create_dir(locked));
  sp_must_ok(t, sp_fs_create_file(fs_path_c(t, "A/B/C")));

  const c8* locked_c = sp_cstr_from_str(mem, locked);
  sp_must_eq(t, chmod(locked_c, 0555), 0);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_dir_s(sp_sys_get_root(0), sp_test_dir(t), 0, &dir));
  sp_err_t result = sp_fs_remove_dir_at((sp_path_t) { .dir = dir, .sub = sp_str_lit("A") });
  sp_sys_close(dir);
  sp_must_eq(t, chmod(locked_c, 0755), 0);

  if (!result) return sp_test_skip(t, "directory permissions not enforced");
  sp_expect_err_eq(t, result, SP_ERR_SYS_ACCESS_DENIED);
  fs_expect_paths(t, sp_test_dir(t), (const fs_expected_path_t [FS_MAX_PATHS]) {
    { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
    { .path = "A/B", .exists = true, .kind = SP_FS_KIND_DIR },
    { .path = "A/B/C", .exists = true, .kind = SP_FS_KIND_FILE },
  });
  return SP_OK;
}
#endif
