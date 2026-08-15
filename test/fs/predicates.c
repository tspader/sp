#include "fs.h"

typedef struct {
  const c8* path;
  bool exists;
  bool file;
  bool dir;
  bool symlink;
  bool target_file;
  bool target_dir;
  sp_fs_kind_t kind;
} probe_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  probe_t probes [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "matrix",
    .setup = {
      { "A" },
      { "B", FS_SETUP_DIR },
      { .path = "C", .kind = FS_SETUP_SYMLINK, .target = "A" },
      { .path = "D", .kind = FS_SETUP_SYMLINK, .target = "B" },
    },
    .probes = {
      {
        .path = "A",
        .exists = true,
        .file = true,
        .target_file = true,
        .kind = SP_FS_KIND_FILE,
      },
      {
        .path = "B",
        .exists = true,
        .dir = true,
        .target_dir = true,
        .kind = SP_FS_KIND_DIR,
      },
      {
        .path = "C",
        .exists = true,
        .symlink = true,
        .target_file = true,
        .kind = SP_FS_KIND_SYMLINK,
      },
      {
        .path = "D",
        .exists = true,
        .symlink = true,
        .target_dir = true,
        .kind = SP_FS_KIND_SYMLINK,
      },
      {
        .path = "E",
      },
    },
  },
  {
    .name = "unicode",
    .setup = {
      { "\xc3\xa9t\xc3\xa9.txt" },
      { "\xc3\xb1\x61\x6d\x65", FS_SETUP_DIR },
    },
    .probes = {
      {
        .path = "\xc3\xa9t\xc3\xa9.txt",
        .exists = true,
        .file = true,
        .target_file = true,
        .kind = SP_FS_KIND_FILE,
      },
      {
        .path = "\xc3\xb1\x61\x6d\x65",
        .exists = true,
        .dir = true,
        .target_dir = true,
        .kind = SP_FS_KIND_DIR,
      },
      {
        .path = "missing\xc3\xa9",
      },
    },
  },
};

sp_test_each(fs, predicates, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_carr_for(it->probes, i) {
    const probe_t* probe = &it->probes[i];
    if (!probe->path) break;
    sp_str_t path = sp_fs_join_path(mem, sandbox, sp_str_view(probe->path));

    fs_expect_bool(t, path, "exists", sp_fs_exists(path), probe->exists);
    fs_expect_bool(t, path, "is_regular_file", sp_fs_is_file(path), probe->file);
    fs_expect_bool(t, path, "is_dir", sp_fs_is_dir(path), probe->dir);
    fs_expect_bool(t, path, "is_symlink", sp_fs_is_symlink(path), probe->symlink);
    fs_expect_bool(t, path, "is_target_regular_file", sp_fs_is_target_file(path), probe->target_file);
    fs_expect_bool(t, path, "is_target_dir", sp_fs_is_target_dir(path), probe->target_dir);
    fs_expect_kind(t, path, sp_fs_get_kind(path), probe->kind);
  }
  return SP_OK;
}
