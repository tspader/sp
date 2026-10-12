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
  bool recursive;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "flat",
    .setup = {
      { "R/A" },
      { "R/B" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_FILE },
        { "B", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "recursive",
    .setup = {
      { "R/A", FS_SETUP_DIR },
      { "R/A/B" },
    },
    .recursive = true,
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_DIR },
        { "A/B", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "propagates_error",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
    },
  },
};

sp_test_each(fs, collect, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);
  sp_path_t root = sp_path_join(mem, sandbox, sp_str_lit("R"));

  fs_match_t matches [FS_MAX_PATHS] = sp_zero;
  u32 n = 0;
  sp_carr_for(it->expect.entries, e) {
    const entry_t* want = &it->expect.entries[e];
    if (!want->path) break;
    matches[n++] = (fs_match_t) { .key = sp_cstr_as_str(want->path), .kind = want->kind };
  }

  if (it->recursive) {
    sp_da(sp_fs_it_entry_t) results = sp_zero;
    sp_err_t err = sp_fs_collect_recursive(mem, root.sub, &results);
    sp_expect_err_eq(t, err, it->expect.err);
    sp_da_for(results, i) {
      sp_expect_str_eq(t, results[i].name, sp_fs_get_name(results[i].path));
      fs_match(t, matches, n, results[i].path, results[i].kind);
    }
  }
  else {
    sp_sys_dir_t dir = sp_sys_dir(SP_SYS_INVALID_FD);
    sp_err_t err = sp_fs_open_dir_at(root, &dir);
    sp_expect_err_eq(t, err, it->expect.err);
    if (!err) {
      sp_fs_dir_it_t d = sp_fs_dir_it(dir);
      SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
      sp_fs_dir_entry_t entry = sp_zero;
      while (sp_fs_dir_it_next(&d, sp_mem_slice(buf, sizeof(buf)), &entry)) {
        fs_match(t, matches, n, entry.name, entry.kind);
      }
      sp_expect_ok(t, d.err);
      sp_fs_dir_it_close(&d);
      sp_fs_close_dir(dir);
    }
  }

  fs_match_finish(t, matches, n);
  return SP_OK;
}
