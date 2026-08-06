#include "sp.h"
#include "sp/sp_test.h"

#define FS_ITER_MAX_SETUP 8
#define FS_ITER_MAX_ENTRIES 8
#define FS_ITER_BULK_PREFIX "R"

#define A16 "AAAAAAAAAAAAAAAA"
#define A255 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 "AAAAAAAAAAAAAAA"
#define EE "\xe2\x82\xac"
#define E5 EE EE EE EE EE
#define E25 E5 E5 E5 E5 E5
#define E255 E25 E25 E25 E5 E5

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
  const c8* target;
} setup_t;

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
  bool seen;
} entry_t;

typedef struct {
  entry_t entries [FS_ITER_MAX_ENTRIES];
} expect_t;

typedef struct {
  const c8* name;
  setup_t setup [FS_ITER_MAX_SETUP];
  const c8* dir;
  bool relative;
  u32 bulk;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "lists_entries_with_kinds",
    .setup = {
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_DIR },
      { "C", SP_FS_KIND_FILE },
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
    .name = "symlink_kind",
    .setup = {
      { "A", SP_FS_KIND_FILE },
      { .path = "L", .kind = SP_FS_KIND_SYMLINK, .target = "A" },
    },
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_FILE },
        { "L", SP_FS_KIND_SYMLINK },
      },
    },
  },
  {
    .name = "empty_directory",
  },
  {
    .name = "honors_dirfd",
    .setup = {
      { "S", SP_FS_KIND_DIR },
      { "S/A", SP_FS_KIND_FILE },
    },
    .dir = "S",
    .relative = true,
    .expect = {
      .entries = {
        { "A", SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "max_length_name",
    .setup = {
      { A255, SP_FS_KIND_FILE },
    },
    .expect = {
      .entries = {
        { A255, SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "long_multibyte_name",
    .setup = {
      { E255, SP_FS_KIND_FILE },
    },
    .expect = {
      .entries = {
        { E255, SP_FS_KIND_FILE },
      },
    },
  },
  {
    .name = "refills_across_batches",
    .bulk = 96,
  },
};

static sp_err_t probe_symlinks(void* user) {
  sp_str_t dir = *(sp_str_t*)user;

  sp_mem_arena_marker_t scratch = sp_mem_begin_scratch();
  sp_str_t target = sp_fs_join_path(scratch.mem, dir, sp_str_lit("probe_target"));
  sp_str_t link = sp_fs_join_path(scratch.mem, dir, sp_str_lit("probe_link"));

  sp_fs_create_file(target);
  sp_err_t err = sp_fs_create_sym_link(target, link);
  if (!err) sp_fs_remove_file(link);
  sp_fs_remove_file(target);

  sp_mem_end_scratch(scratch);
  return err;
}

static bool is_symlink_needed(const test_t* c) {
  sp_carr_for(c->setup, it) {
    if (!c->setup[it].path) break;
    if (c->setup[it].kind == SP_FS_KIND_SYMLINK) return true;
  }
  return false;
}

sp_test_each(fs, iter, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);

  static sp_test_once_t probe = sp_zero;
  if (is_symlink_needed(it) && sp_test_once(&probe, probe_symlinks, &sandbox)) {
    return sp_test_skip(t, "symlinks not available");
  }

  sp_carr_for(it->setup, s) {
    const setup_t* entry = &it->setup[s];
    if (!entry->path) break;
    sp_str_t path = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(entry->path));

    switch (entry->kind) {
      case SP_FS_KIND_FILE: {
        sp_expect_ok(t, sp_fs_create_file(path));
        break;
      }
      case SP_FS_KIND_DIR: {
        sp_expect_ok(t, sp_fs_create_dir(path));
        break;
      }
      case SP_FS_KIND_SYMLINK: {
        sp_str_t target = sp_fs_join_path(mem, sandbox, sp_cstr_as_str(entry->target));
        sp_expect_ok(t, sp_fs_create_sym_link(target, path));
        break;
      }
      case SP_FS_KIND_NONE: {
        break;
      }
    }
  }

  sp_str_t dir = it->dir ? sp_fs_join_path(mem, sandbox, sp_cstr_as_str(it->dir)) : sandbox;

  sp_str_t* bulk_names = sp_alloc_n(mem, sp_str_t, it->bulk ? it->bulk : 1);
  bool* bulk_seen = sp_alloc_n(mem, bool, it->bulk ? it->bulk : 1);
  sp_for(b, it->bulk) {
    bulk_names[b] = sp_fmt(mem, "{}{}", sp_fmt_cstr(FS_ITER_BULK_PREFIX), sp_fmt_uint(b)).value;
    sp_expect_ok(t, sp_fs_create_file(sp_fs_join_path(mem, dir, bulk_names[b])));
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

  sp_err_t walk = SP_OK;
  if (!open_err) {
    while (true) {
      sp_fs_dir_entry_t entry = sp_zero;
      walk = sp_fs_dir_next(&iter, &entry);
      if (walk) break;
      if (!entry.name.data) break;

      if (sp_str_equal(entry.name, sp_str_lit(".")) || sp_str_equal(entry.name, sp_str_lit(".."))) {
        sp_test_fail(t, "iterator produced dot entry {}", sp_fmt_str(entry.name));
        continue;
      }

      bool matched = false;
      sp_carr_for(it->expect.entries, e) {
        entry_t* want = &it->expect.entries[e];
        if (!want->name) break;
        if (!sp_str_equal_cstr(entry.name, want->name)) continue;
        matched = true;
        if (want->seen) sp_test_fail(t, "iterator produced {} twice", sp_fmt_str(entry.name));
        want->seen = true;
        sp_expect_eq(t, (u32)entry.kind, (u32)want->kind);
        break;
      }
      sp_for(b, it->bulk) {
        if (matched) break;
        if (!sp_str_equal(entry.name, bulk_names[b])) continue;
        matched = true;
        if (bulk_seen[b]) sp_test_fail(t, "iterator produced {} twice", sp_fmt_str(entry.name));
        bulk_seen[b] = true;
        sp_expect_eq(t, (u32)entry.kind, (u32)SP_FS_KIND_FILE);
      }
      if (!matched) {
        sp_test_fail(t, "iterator produced unexpected entry {}", sp_fmt_str(entry.name));
      }
    }
    sp_expect_ok(t, walk);
    sp_fs_dir_close(&iter);
  }

  sp_carr_for(it->expect.entries, e) {
    if (!it->expect.entries[e].name) break;
    if (!it->expect.entries[e].seen) {
      sp_test_fail(t, "iterator never produced {}", sp_fmt_cstr(it->expect.entries[e].name));
    }
  }
  sp_for(b, it->bulk) {
    if (!bulk_seen[b]) {
      sp_test_fail(t, "iterator never produced {}", sp_fmt_str(bulk_names[b]));
    }
  }

  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
  return SP_OK;
}
