#include "fs.h"

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  sp_err_t err;
  entry_t entries [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* root;
  bool recursive;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "lists_entries_with_kinds",
    .setup = {
      { "R/A" },
      { "R/B", FS_SETUP_DIR },
      { "R/C" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_DIR },
        { "C", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "hidden_file",
    .setup = {
      { "R/.A" },
      { "R/B" },
    },
    .expect = {
      .entries = {
        { ".A", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "unicode_entries",
    .setup = {
      { "R/\xc3\xb1\x61\x6d\x65.txt" },
      { "R/\xc3\xbc\x6e\x69", FS_SETUP_DIR },
    },
    .expect = {
      .entries = {
        { "\xc3\xb1\x61\x6d\x65.txt", SP_FS_KIND_FILE },
        { "\xc3\xbc\x6e\x69", SP_FS_KIND_DIR },
      },
    },
  },
  {
    .name = "empty_dir",
    .setup = {
      { "R", FS_SETUP_DIR },
    },
  },
  {
    .name = "does_not_recurse",
    .setup = {
      { "R/A", FS_SETUP_DIR },
      { "R/A/B" },
      { "R/C" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "C", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "recursive_nested_dirs",
    .setup = {
      { "R/A", FS_SETUP_DIR },
      { "R/A/B", FS_SETUP_DIR },
      { "R/C" },
      { "R/A/D" },
      { "R/A/B/E" },
    },
    .recursive = true,
    .expect = {
      .entries = {
        { "C", SP_FS_KIND_FILE },
        { "A", SP_FS_KIND_DIR },
        { "A/D", SP_FS_KIND_FILE },
        { "A/B", SP_FS_KIND_DIR },
        { "A/B/E", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "recursive_symlink_not_followed",
    .setup = {
      { "R/A", FS_SETUP_DIR },
      { "R/A/B" },
      { .path = "R/L", .kind = FS_SETUP_SYMLINK, .target = "R/A" },
    },
    .recursive = true,
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_FILE },
        { "L", SP_FS_KIND_SYMLINK },
      },
    },
  },
  {
    .name = "trailing_slash_root",
    .setup = {
      { "R/A" },
    },
    .root = "R/",
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "nonexistent",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
  {
    .name = "file_not_dir",
    .setup = {
      { "R" },
    },
    .expect = {
      .err = SP_ERR_SYS_NOT_DIR,
    },
  },
#if defined(SP_POSIX)
  {
    .name = "special_file",
    .setup = {
      { "R", FS_SETUP_DIR },
      { .path = "R/F", .kind = FS_SETUP_FIFO },
    },
    .expect = {
      .entries = {
        // a fifo is listed, with a kind sp_fs can't express
        { "F" },
      },
    },
  },
#endif
};

sp_test_each(fs, it, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t base = sp_fs_join_path(mem, sandbox, sp_str_lit("R"));
  sp_str_t root = sp_fmt(mem, "{}/{}", sp_fmt_str(sandbox), sp_fmt_cstr(it->root ? it->root : "R")).value;

  fs_match_t matches [FS_MAX_PATHS] = sp_zero;
  u32 n = 0;
  sp_carr_for(it->expect.entries, e) {
    const entry_t* want = &it->expect.entries[e];
    if (!want->path) break;
    matches[n++] = (fs_match_t) { .key = sp_fs_join_path(mem, base, sp_cstr_as_str(want->path)), .kind = want->kind };
  }

  sp_fs_it_t walk = it->recursive ? sp_fs_it_new_recursive(mem, root) : sp_fs_it_new(mem, root);
  while (sp_fs_it_next(&walk)) {
    sp_expect_str_eq(t, walk.entry.name, sp_fs_get_name(walk.entry.path));
    fs_match(t, matches, n, walk.entry.path, walk.entry.kind);
  }
  sp_expect_err_eq(t, walk.err, it->expect.err);
  sp_expect(t, !sp_fs_it_next(&walk));
  sp_expect_err_eq(t, walk.err, it->expect.err);
  sp_fs_it_deinit(&walk);

  fs_match_finish(t, matches, n);
  return SP_OK;
}
