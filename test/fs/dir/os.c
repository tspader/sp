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
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, it->setup));

  sp_path_t dir = it->dir ? sp_path_join(mem, sandbox, sp_cstr_as_str(it->dir)) : sandbox;

  fs_match_t* matches = sp_alloc_n(mem, fs_match_t, FS_MAX_PATHS + it->bulk);
  u32 n = 0;
  sp_carr_for(it->expect, e) {
    if (!it->expect[e].name) break;
    matches[n++] = (fs_match_t) { .key = sp_cstr_as_str(it->expect[e].name), .kind = it->expect[e].kind };
  }
  sp_for(b, it->bulk) {
    sp_str_t name = sp_fmt(mem, "{}{}", sp_fmt_cstr(BULK_PREFIX), sp_fmt_uint(b)).value;
    sp_expect_ok(t, sp_fs_create_file_at(sp_path_join(mem, dir, name)));
    matches[n++] = (fs_match_t) { .key = name, .kind = SP_FS_KIND_FILE };
  }

  sp_sys_dir_t sandbox_dir = sp_sys_dir(SP_SYS_INVALID_FD);
  if (it->relative) {
    sp_try(sp_fs_open_dir_at(sandbox, &sandbox_dir));
  }

  sp_sys_dir_t opened = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_path_t path = it->relative ? sp_path(sandbox_dir, sp_cstr_as_str(it->dir)) : dir;
  sp_err_t open_err = sp_fs_open_dir_at(path, &opened);
  sp_expect_ok(t, open_err);

  if (!open_err) {
    SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
    sp_fs_dir_it_t iter = sp_fs_dir_it(opened);
    sp_fs_dir_entry_t entry = sp_zero;
    while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) {
      fs_match(t, matches, n, entry.name, entry.kind);
    }
    sp_expect_ok(t, iter.err);
    sp_expect_ok(t, sp_fs_dir_it_close(&iter));
    sp_fs_close_dir(opened);
  }
  fs_match_finish(t, matches, n);

  if (sandbox_dir.fd != SP_SYS_INVALID_FD) sp_fs_close_dir(sandbox_dir);
  return SP_OK;
}

sp_test(fs, dir_it_rejects_small_buffer) {
  sp_sys_dir_t dir = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_must_ok(t, sp_fs_open_dir_at(sp_test_dir(t), &dir));

  sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  sp_expect(t, !sp_fs_dir_it_next(&iter, sp_mem_slice(buf, SP_SYS_DIR_MIN_BUF - 1), &entry));
  sp_expect_err_eq(t, iter.err, SP_ERR_SYS_INVALID);
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));
  sp_fs_close_dir(dir);
  return SP_OK;
}

sp_test(fs, dir_it_next_after_close_yields_nothing) {
  static const fs_setup_t setup [FS_MAX_SETUP] = {
    { "A" },
    { "B" },
  };

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, setup));

  sp_sys_dir_t dir = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  sp_expect(t, sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry));
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));
  sp_expect(t, !sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry));
  sp_expect_err_eq(t, iter.err, SP_ERR_SYS_BAD_FD);
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));
  sp_fs_close_dir(dir);
  return SP_OK;
}

sp_test(fs, dir_it_file_handle_is_not_dir) {
  static const fs_setup_t setup [FS_MAX_SETUP] = {
    { "A" },
  };

  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, setup));

  sp_sys_dir_t dir = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));
  sp_sys_fd_t file = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_s(dir, sp_str_lit("A"), SP_SYS_OPEN_MODE_RO, 0, &file));

  sp_fs_dir_it_t iter = sp_fs_dir_it(sp_sys_dir(file));
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  sp_expect(t, !sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry));
  sp_expect_err_eq(t, iter.err, SP_ERR_SYS_NOT_DIR);
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));

  sp_sys_close(file);
  sp_fs_close_dir(dir);
  return SP_OK;
}

sp_test(fs, dir_it_invalid_handle_yields_nothing) {
  sp_fs_dir_it_t iter = sp_fs_dir_it(sp_sys_dir(SP_SYS_INVALID_FD));
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  u32 n = 0;
  while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) n++;
  sp_expect_eq(t, n, (u32)0);
  sp_expect_err_eq(t, iter.err, SP_ERR_SYS_BAD_FD);
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));
  return SP_OK;
}

sp_test(fs, dir_reiterates_handle) {
  static const fs_setup_t setup [FS_MAX_SETUP] = {
    { "A" },
    { "B", FS_SETUP_DIR },
  };

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, setup));

  sp_sys_dir_t dir = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  fs_match_t matches [] = {
    { .key = sp_str_lit("A"), .kind = SP_FS_KIND_FILE },
    { .key = sp_str_lit("B"), .kind = SP_FS_KIND_DIR },
  };
  u32 n = sp_carr_len(matches);

  sp_for(pass, 2) {
    sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
    SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
    sp_fs_dir_entry_t entry = sp_zero;
    while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) fs_match(t, matches, n, entry.name, entry.kind);
    sp_expect_ok(t, iter.err);
    sp_expect_ok(t, sp_fs_dir_it_close(&iter));
    fs_match_finish(t, matches, n);
    sp_for(i, n) matches[i].seen = false;
  }

  sp_fs_it_t walk = sp_fs_it(mem, dir);
  sp_fs_it_entry_t entry = sp_zero;
  while (sp_fs_it_next(&walk, &entry)) fs_match(t, matches, n, entry.name, entry.kind);
  sp_expect_ok(t, walk.err);
  sp_fs_it_deinit(&walk);
  fs_match_finish(t, matches, n);

  sp_fs_close_dir(dir);
  return SP_OK;
}

static sp_da(sp_str_t) dir_it_names(sp_test_t* t, sp_mem_t mem, sp_sys_dir_t dir) {
  sp_da(sp_str_t) names = sp_da_new(mem, sp_str_t);
  sp_fs_dir_it_t iter = sp_fs_dir_it(dir);
  SP_ALIGNED u8 buf [SP_SYS_DIR_MIN_BUF];
  sp_fs_dir_entry_t entry = sp_zero;
  while (sp_fs_dir_it_next(&iter, sp_mem_slice(buf, sizeof(buf)), &entry)) sp_da_push(names, sp_str_copy(mem, entry.name));
  sp_expect_ok(t, iter.err);
  sp_expect_ok(t, sp_fs_dir_it_close(&iter));
  sp_da_sort(names, sp_str_sort_kernel_alphabetical);
  return names;
}

sp_test(fs, dir_it_cwd_matches_opened_cwd) {
  sp_mem_t mem = sp_test_arena(t);
  sp_sys_dir_t opened = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_must_ok(t, sp_fs_open_dir_at(sp_path_cwd_c("."), &opened));
  sp_da(sp_str_t) expect = dir_it_names(t, mem, opened);
  sp_fs_close_dir(opened);

  sp_da(sp_str_t) names = dir_it_names(t, mem, sp_fs_get_cwd());
  sp_must_eq(t, sp_da_size(names), sp_da_size(expect));
  sp_da_for(names, i) sp_expect_str_eq(t, names[i], expect[i]);
  return SP_OK;
}

sp_test(fs, it_cwd_matches_opened_cwd) {
  sp_mem_t mem = sp_test_arena(t);
  sp_sys_dir_t opened = sp_sys_dir(SP_SYS_INVALID_FD);
  sp_must_ok(t, sp_fs_open_dir_at(sp_path_cwd_c("."), &opened));
  sp_da(sp_str_t) expect = dir_it_names(t, mem, opened);
  sp_fs_close_dir(opened);

  sp_da(sp_str_t) names = sp_da_new(mem, sp_str_t);
  sp_fs_it_t walk = sp_fs_it(mem, sp_fs_get_cwd());
  sp_fs_it_entry_t entry = sp_zero;
  while (sp_fs_it_next(&walk, &entry)) {
    sp_fs_it_skip(&walk);
    sp_expect_eq(t, entry.at.dir.fd, sp_fs_get_cwd().fd);
    sp_da_push(names, sp_str_copy(mem, entry.name));
  }
  sp_expect_ok(t, walk.err);
  sp_fs_it_deinit(&walk);
  sp_da_sort(names, sp_str_sort_kernel_alphabetical);

  sp_must_eq(t, sp_da_size(names), sp_da_size(expect));
  sp_da_for(names, i) sp_expect_str_eq(t, names[i], expect[i]);
  return SP_OK;
}
