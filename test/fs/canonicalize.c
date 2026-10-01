#include "fs.h"

typedef struct {
  sp_err_t err;
  bool no_trailing_slash;
  bool no_backslash;
  const c8* name;
  bool exists;
  bool idempotent;
  const c8* same_as;
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* input;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "dot_slash_dotdot",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
    },
    .input = "A/./B/..",
    .expect = {
      .no_backslash = true,
      .name = "A",
      .exists = true,
    },
  },
  {
    .name = "unicode",
    .setup = {
      { "\xc3\xa9t\xc3\xa9", FS_SETUP_DIR },
    },
    .input = "\xc3\xa9t\xc3\xa9",
    .expect = {
      .no_backslash = true,
      .exists = true,
    },
  },
  {
    .name = "regular_file",
    .setup = {
      { "A" },
    },
    .input = "A",
    .expect = {
      .no_backslash = true,
      .name = "A",
      .exists = true,
    },
  },
  {
    .name = "directory",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .input = "A",
    .expect = {
      .no_trailing_slash = true,
      .no_backslash = true,
      .name = "A",
      .exists = true,
    },
  },
  {
    .name = "nested_dotdot",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
      { "A/B/C", FS_SETUP_DIR },
    },
    .input = "A/B/C/../../B",
    .expect = {
      .no_backslash = true,
      .name = "B",
      .exists = true,
    },
  },
  {
    .name = "nonexistent_is_not_found",
    .input = "/this/path/does/not/exist/at/all",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "empty_input",
    .input = "",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "nonexistent_with_dotdot",
    .input = "no_such_dir/../also_missing.txt",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "resolves_symlink_to_file",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .input = "L",
    .expect = {
      .no_backslash = true,
      .name = "A",
      .exists = true,
      .same_as = "A",
    },
  },
  {
    .name = "resolves_symlink_to_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .input = "L",
    .expect = {
      .no_backslash = true,
      .name = "A",
      .exists = true,
    },
  },
  {
    .name = "resolves_chained_symlinks",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
      { .path = "M", .kind = FS_SETUP_SYMLINK, .target = "L" },
    },
    .input = "M",
    .expect = {
      .no_backslash = true,
      .name = "A",
      .exists = true,
    },
  },
  {
    .name = "idempotent",
    .setup = {
      { "A" },
    },
    .input = "A",
    .expect = {
      .exists = true,
      .idempotent = true,
    },
  },
};

sp_test_each(fs, canonicalize, test_t, tests) {
  sp_test_skip_on_wasm()
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_path_t input = sp_path(dir, sp_str_view(it->input));
  sp_str_t result = sp_zero;
  sp_expect_err_eq(t, sp_fs_canonicalize_path_at(mem, input, &result), it->expect.err);

  if (it->expect.err) {
    sp_sys_close(dir);
    return SP_OK;
  }

  sp_expect_gt(t, result.len, 0u);
  if (it->expect.no_trailing_slash && result.len > 0) {
    sp_expect_ne(t, result.data[result.len - 1], '/');
  }
  if (it->expect.no_backslash) {
    sp_expect(t, !sp_str_contains(result, sp_str_lit("\\")));
  }
  if (it->expect.name) {
    sp_expect_str_eq_c(t, sp_fs_get_name(result), it->expect.name);
  }
  if (it->expect.exists) {
    sp_expect(t, sp_fs_exists_at(sp_path(dir, result)));
  }
  if (it->expect.idempotent) {
    sp_str_t again = sp_zero;
    sp_expect_ok(t, sp_fs_canonicalize_path_at(mem, sp_path(dir, result), &again));
    sp_expect_str_eq(t, again, result);
  }
  if (it->expect.same_as) {
    sp_str_t other = sp_zero;
    sp_expect_ok(t, sp_fs_canonicalize_path_at(mem, sp_path(dir, sp_str_view(it->expect.same_as)), &other));
    sp_expect_str_eq(t, result, other);
  }
  sp_sys_close(dir);
  return SP_OK;
}

sp_test(fs, canon_dot_resolves_to_cwd) {
  sp_test_skip_on_wasm()

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t canonical = sp_zero;
  sp_str_t cwd = sp_zero;
  sp_must_ok(t, sp_fs_canonicalize_path(mem, sp_str_lit("."), &canonical));
  sp_must_ok(t, sp_fs_get_cwd_path(mem, &cwd));
  sp_expect_str_eq(t, canonical, cwd);
  return SP_OK;
}
