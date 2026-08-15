#include "fs.h"

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  bool recursive;
  sp_err_t err;
  expect_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "empty_dir",
    .setup = {
      { "R", FS_SETUP_DIR },
    },
  },
  {
    .name = "nonexistent",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
  {
    .name = "file_not_dir",
    .setup = {
      { "R" },
    },
    .err = SP_ERR_SYS_NOT_DIR,
  },
  {
    .name = "single_file",
    .setup = {
      { "R/A" },
    },
    .expect = {
      { "A", SP_FS_KIND_FILE },
    },
  },
  {
    .name = "multiple_files",
    .setup = {
      { "R/A" },
      { "R/B" },
      { "R/C" },
    },
    .expect = {
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_FILE },
      { "C", SP_FS_KIND_FILE },
    },
  },
  {
    .name = "subdirectory",
    .setup = {
      { "R/A", FS_SETUP_DIR },
    },
    .expect = {
      { "A", SP_FS_KIND_DIR },
    },
  },
  {
    .name = "mixed_types",
    .setup = {
      { "R/A" },
      { "R/B", FS_SETUP_DIR },
      { .path = "R/L", .kind = FS_SETUP_SYMLINK, .target = "R/A" },
    },
    .expect = {
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_DIR },
      { "L", SP_FS_KIND_SYMLINK },
    },
  },
  {
    .name = "hidden_file",
    .setup = {
      { "R/.A" },
      { "R/B" },
    },
    .expect = {
      { ".A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_FILE },
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
      { "A", SP_FS_KIND_DIR },
      { "C", SP_FS_KIND_FILE },
    },
  },
  {
    .name = "recursive_empty_dir",
    .setup = {
      { "R", FS_SETUP_DIR },
    },
    .recursive = true,
  },
  {
    .name = "recursive_flat_dir",
    .setup = {
      { "R/A" },
      { "R/B" },
      { "R/C" },
    },
    .recursive = true,
    .expect = {
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_FILE },
      { "C", SP_FS_KIND_FILE },
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
      { "C", SP_FS_KIND_FILE },
      { "A", SP_FS_KIND_DIR },
      { "A/D", SP_FS_KIND_FILE },
      { "A/B", SP_FS_KIND_DIR },
      { "A/B/E", SP_FS_KIND_FILE },
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
      { "A", SP_FS_KIND_DIR },
      { "A/B", SP_FS_KIND_FILE },
      { "L", SP_FS_KIND_SYMLINK },
    },
  },
  {
    .name = "recursive_nonexistent",
    .recursive = true,
    .err = SP_ERR_SYS_NOT_FOUND,
  },
  {
    .name = "recursive_file_not_dir",
    .setup = {
      { "R" },
    },
    .recursive = true,
    .err = SP_ERR_SYS_NOT_DIR,
  },
  {
    .name = "unicode_entries",
    .setup = {
      { "R/\xc3\xb1\x61\x6d\x65.txt" },
      { "R/\xc3\xbc\x6e\x69", FS_SETUP_DIR },
    },
    .expect = {
      { "\xc3\xb1\x61\x6d\x65.txt", SP_FS_KIND_FILE },
      { "\xc3\xbc\x6e\x69", SP_FS_KIND_DIR },
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
      // a fifo is listed, with a kind sp_fs can't express
      { "F" },
    },
  },
#endif
};

static void collect(sp_test_t* t, const test_t* c, sp_str_t root, sp_str_t base) {
  sp_mem_t mem = sp_test_arena(t);
  sp_da(sp_fs_entry_t) results;
  sp_err_t err = c->recursive
    ? sp_fs_collect_recursive(mem, base, &results)
    : sp_fs_collect(mem, base, &results);
  sp_expect_err_eq(t, err, c->err);

  u32 expected = 0;
  sp_carr_for(c->expect, it) {
    if (!c->expect[it].path) break;
    expected++;
  }

  sp_expect_eq(t, sp_da_size(results), expected);

  sp_da_for(results, n) {
    sp_fs_entry_t entry = results[n];
    sp_expect(t, !sp_str_contains(entry.path, sp_str_lit("//")));
    sp_str_t tail = sp_str_sub(entry.path, entry.path.len - entry.name.len, entry.name.len);
    sp_expect(t, sp_str_equal(tail, entry.name));
  }

  sp_carr_for(c->expect, i) {
    const expect_t* want = &c->expect[i];
    if (!want->path) break;
    sp_str_t path = sp_fs_join_path(mem, root, sp_str_view(want->path));

    bool found = false;
    sp_da_for(results, n) {
      if (!sp_str_equal(results[n].path, path)) continue;
      sp_expect_eq(t, results[n].kind, want->kind);
      found = true;
      break;
    }
    sp_expect(t, found);
  }
}

sp_test_each(fs, collect, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t root = sp_fs_join_path(mem, sandbox, sp_str_lit("R"));
  collect(t, it, root, root);
  collect(t, it, root, sp_str_concat(mem, root, sp_str_lit("/")));
  return SP_OK;
}
