#include "fs.h"

#define BULK_PREFIX "R"

#define A16 "AAAAAAAAAAAAAAAA"
#define A255 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 "AAAAAAAAAAAAAAA"
#define EE "\xe2\x82\xac"
#define E5 EE EE EE EE EE
#define E25 E5 E5 E5 E5 E5
#define E255 E25 E25 E25 E5 E5

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  const c8* dir;
  bool relative;
  u32 bulk;
  entry_t expect [FS_MAX_PATHS];
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
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_DIR },
      { "C", SP_FS_KIND_FILE },
    },
  },
  {
    .name = "symlink_kind",
    .setup = {
      { "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .expect = {
      { "A", SP_FS_KIND_FILE },
      { "L", SP_FS_KIND_SYMLINK },
    },
  },
  {
    .name = "empty_directory",
  },
  {
    .name = "honors_dirfd",
    .setup = {
      { "S", FS_SETUP_DIR },
      { "S/A" },
    },
    .dir = "S",
    .relative = true,
    .expect = {
      { "A", SP_FS_KIND_FILE },
    },
  },
  {
    .name = "max_length_name",
    .setup = {
      { A255 },
    },
    .expect = {
      { A255, SP_FS_KIND_FILE },
    },
  },
  {
    .name = "long_multibyte_name",
    .setup = {
      { E255 },
    },
    .expect = {
      { E255, SP_FS_KIND_FILE },
    },
  },
  {
    .name = "refills_across_batches",
    .bulk = 96,
  },
};

sp_test_each(fs, dir, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t dir = it->dir ? sp_fs_join_path(mem, sandbox, sp_cstr_as_str(it->dir)) : sandbox;

  fs_match_t* matches = sp_alloc_n(mem, fs_match_t, FS_MAX_PATHS + it->bulk);
  u32 n = 0;
  sp_carr_for(it->expect, e) {
    if (!it->expect[e].name) break;
    matches[n++] = (fs_match_t) { .key = sp_cstr_as_str(it->expect[e].name), .kind = it->expect[e].kind };
  }
  sp_for(b, it->bulk) {
    sp_str_t name = sp_fmt(mem, "{}{}", sp_fmt_cstr(BULK_PREFIX), sp_fmt_uint(b)).value;
    sp_expect_ok(t, sp_fs_create_file(sp_fs_join_path(mem, dir, name)));
    matches[n++] = (fs_match_t) { .key = name, .kind = SP_FS_KIND_FILE };
  }

  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  if (it->relative) {
    sp_try(sp_sys_open_dir_s(sp_sys_get_root(0), sandbox, &sandbox_fd));
  }

  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_t iter = sp_zero;
  sp_err_t open_err = it->relative
    ? sp_fs_dir_open(&iter, sandbox_fd, sp_cstr_as_str(it->dir), sp_mem_slice(buf, sizeof(buf)))
    : sp_fs_dir_open(&iter, sp_sys_get_root(0), dir, sp_mem_slice(buf, sizeof(buf)));
  sp_expect_ok(t, open_err);

  if (!open_err) {
    sp_err_t walk = SP_OK;
    while (true) {
      sp_fs_dir_entry_t entry = sp_zero;
      walk = sp_fs_dir_next(&iter, &entry);
      if (walk) break;
      if (!entry.name.data) break;
      fs_match(t, matches, n, entry.name, entry.kind);
    }
    sp_expect_ok(t, walk);
    sp_expect_ok(t, sp_fs_dir_close(&iter));
  }
  fs_match_finish(t, matches, n);

  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
  return SP_OK;
}
