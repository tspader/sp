#include "fs.h"

typedef struct {
  bool empty;
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
    .name = "nonexistent_returns_empty",
    .input = "/this/path/does/not/exist/at/all",
    .expect = {
      .empty = true,
    },
  },
  {
    .name = "empty_input",
    .input = "",
    .expect = {
      .empty = true,
    },
  },
  {
    .name = "nonexistent_relative",
    .input = "no_such_file.txt",
    .expect = {
      .empty = true,
    },
  },
  {
    .name = "nonexistent_with_dotdot",
    .input = "no_such_dir/../also_missing.txt",
    .expect = {
      .empty = true,
    },
  },
  {
    .name = "result_is_normalized",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .input = "A",
    .expect = {
      .no_trailing_slash = true,
      .no_backslash = true,
      .exists = true,
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
    },
  },
  {
    .name = "resolves_symlink_to_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
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
    .name = "symlink_with_dotdot",
    .setup = {
      { "A" },
      { "B", FS_SETUP_DIR },
      { .path = "B/L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .input = "B/L",
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
  {
    .name = "through_symlink",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .input = "L",
    .expect = {
      .same_as = "A",
    },
  },
};

sp_test_each(fs, canonicalize, test_t, tests) {
  sp_test_skip_on_wasm()
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t input = sp_str_view(it->input);
  if (it->setup[0].path) {
    sp_str_t sandbox = sp_test_dir(t);
    fs_apply_setup(t, sandbox, it->setup);
    input = sp_fs_join_path(mem, sandbox, input);
  }

  sp_str_t result = sp_fs_canonicalize_path(mem, input);

  if (it->expect.empty) {
    if (result.len != 0) sp_test_fail(t, "expected empty, got {}", sp_fmt_str(result));
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
    sp_expect(t, sp_fs_exists(result));
  }
  if (it->expect.idempotent) {
    sp_expect_str_eq(t, sp_fs_canonicalize_path(mem, result), result);
  }
  if (it->expect.same_as) {
    sp_str_t other = sp_fs_join_path(mem, sp_test_dir(t), sp_str_view(it->expect.same_as));
    sp_expect_str_eq(t, result, sp_fs_canonicalize_path(mem, other));
  }
  return SP_OK;
}

sp_test(fs, canon_dot_resolves_to_cwd) {
  sp_test_skip_on_wasm()

  sp_mem_t mem = sp_test_arena(t);
  sp_expect_str_eq(t, sp_fs_canonicalize_path(mem, sp_str_lit(".")), sp_fs_get_cwd(mem));
  return SP_OK;
}
