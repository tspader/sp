#include "fs.h"

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  entry_t entries [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "lists_entries_with_kinds",
    .setup = {
      { "A" },
      { "B", FS_SETUP_DIR },
      { "C" },
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
      { ".A" },
      { "B" },
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
      { "\xc3\xb1\x61\x6d\x65.txt" },
      { "\xc3\xbc\x6e\x69", FS_SETUP_DIR },
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
  },
  {
    .name = "nested_dirs",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
      { "C" },
      { "A/D" },
      { "A/B/E" },
    },
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
    .name = "symlink_not_followed",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B" },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "A" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_FILE },
        { "L", SP_FS_KIND_SYMLINK },
      },
    },
  },
#if defined(SP_POSIX)
  {
    .name = "special_file",
    .setup = {
      { .path = "F", .kind = FS_SETUP_FIFO },
    },
    .expect = {
      .entries = {
        // a fifo is listed, with a kind sp_fs can't express
        { "F", SP_FS_KIND_UNKNOWN },
      },
    },
  },
#endif
};

sp_test_each(fs, it, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  fs_match_t matches [FS_MAX_PATHS] = sp_zero;
  u32 n = 0;
  sp_carr_for(it->expect.entries, e) {
    const entry_t* want = &it->expect.entries[e];
    if (!want->path) break;
    matches[n++] = (fs_match_t) { .key = sp_cstr_as_str(want->path), .kind = want->kind };
  }

  sp_sys_dir_t root = sp_zero;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &root));

  sp_fs_it_t walk = sp_fs_it(mem, root);
  sp_fs_it_entry_t entry = sp_zero;
  while (sp_fs_it_next(&walk, &entry)) {
    sp_expect_str_eq(t, entry.name, sp_fs_get_name(entry.path));
    sp_expect_str_eq(t, entry.at.sub, entry.name);
    sp_sys_file_meta_t meta = sp_zero;
    sp_expect_ok(t, sp_fs_get_link_metadata_at(entry.at, &meta));
    sp_expect_eq(t, (u32)meta.kind, (u32)entry.kind);
    fs_match(t, matches, n, entry.path, entry.kind);
  }
  sp_expect_ok(t, walk.err);
  sp_expect(t, !sp_fs_it_next(&walk, &entry));
  sp_expect_ok(t, walk.err);
  sp_fs_it_deinit(&walk);
  sp_fs_close_dir(root);

  fs_match_finish(t, matches, n);
  return SP_OK;
}
