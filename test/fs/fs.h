#ifndef FS_TEST_H
#define FS_TEST_H

#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_POSIX)
  #include "sys/stat.h"
#endif

#define FS_MAX_SETUP 8
#define FS_MAX_PATHS 8
#define FS_DEEP_LEVELS 40
#define FS_DEEP_NAME "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" \
                     "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"


////////////////////
// SHARED HARNESS //
////////////////////
typedef enum {
  FS_SETUP_FILE,
  FS_SETUP_DIR,
  FS_SETUP_SYMLINK,
  FS_SETUP_DIR_SYMLINK,
  FS_SETUP_FIFO,
} fs_setup_kind_t;

typedef struct {
  const c8* path;
  fs_setup_kind_t kind;
  union {
    const c8* target;
    const c8* content;
  };
} fs_setup_t;

typedef struct {
  const c8* path;
  bool exists;
  sp_fs_kind_t kind;
  const c8* content;
  const c8* target;
} fs_expected_path_t;

static bool fs_setup_needs_symlinks(const fs_setup_t setup [FS_MAX_SETUP]) {
  sp_for(it, FS_MAX_SETUP) {
    if (!setup[it].path) break;
    if (setup[it].kind == FS_SETUP_SYMLINK || setup[it].kind == FS_SETUP_DIR_SYMLINK) return true;
  }
  return false;
}

static void fs_expect_bool(sp_test_t* t, sp_str_t path, const c8* label, bool actual, bool expected) {
  if (actual == expected) return;

  sp_test_fail(
    t,
    "{} {} was {} but expected {}",
    sp_fmt_cstr(label),
    sp_fmt_str(path),
    sp_fmt_cstr(actual ? "true" : "false"),
    sp_fmt_cstr(expected ? "true" : "false")
  );
}

static void fs_expect_kind(sp_test_t* t, sp_str_t path, sp_fs_kind_t actual, sp_fs_kind_t expected) {
  if (actual == expected) return;

  sp_test_fail(
    t,
    "{} had kind {} but expected {}",
    sp_fmt_str(path),
    sp_fmt_int(actual),
    sp_fmt_int(expected)
  );
}

typedef struct {
  sp_str_t key;
  sp_fs_kind_t kind;
  bool seen;
} fs_match_t;

static void fs_match(sp_test_t* t, fs_match_t* matches, u32 n, sp_str_t key, sp_fs_kind_t kind) {
  sp_for(it, n) {
    fs_match_t* match = &matches[it];
    if (!sp_str_equal(match->key, key)) continue;
    if (match->seen) sp_test_fail(t, "{} produced twice", sp_fmt_str(key));
    match->seen = true;
    fs_expect_kind(t, key, kind, match->kind);
    return;
  }
  sp_test_fail(t, "unexpected entry {}", sp_fmt_str(key));
}

static void fs_match_finish(sp_test_t* t, fs_match_t* matches, u32 n) {
  sp_for(it, n) {
    if (!matches[it].seen) sp_test_fail(t, "never produced {}", sp_fmt_str(matches[it].key));
  }
}

static sp_err_t fs_apply_setup(sp_test_t* t, sp_path_t sandbox, const fs_setup_t setup [FS_MAX_SETUP]) {
  sp_mem_t mem = sp_test_arena(t);
  sp_for(it, FS_MAX_SETUP) {
    const fs_setup_t* ent = &setup[it];
    if (!ent->path) break;
    sp_test_kv_c(t, "setup", ent->path);

    sp_path_t path = sp_path_join(mem, sandbox, sp_cstr_as_str(ent->path));
    sp_path_t parent = sp_path_parent(mem, path);
    sp_fs_create_dir_at(parent);

    switch (ent->kind) {
      case FS_SETUP_FILE: {
        sp_try(sp_fs_create_file_str_at(path, ent->content ? sp_cstr_as_str(ent->content) : sp_str_lit("")));
        break;
      }
      case FS_SETUP_DIR: {
        sp_try(sp_fs_create_dir_at(path));
        break;
      }
      case FS_SETUP_SYMLINK:
      case FS_SETUP_DIR_SYMLINK: {
        sp_fs_kind_t kind = ent->kind == FS_SETUP_DIR_SYMLINK ? SP_FS_KIND_DIR : SP_FS_KIND_FILE;
        sp_try(sp_fs_create_sym_link_at(sp_cstr_as_str(ent->target), path, kind));
        break;
      }
      case FS_SETUP_FIFO: {
#if defined(SP_POSIX)
        sp_str_t dir = sp_zero;
        sp_try(sp_fs_canonicalize_path_at(mem, parent, &dir));
        sp_str_t fifo = sp_fs_join_path(mem, dir, sp_fs_get_name(path.sub));
        if (mkfifo(sp_cstr_from_str(mem, fifo), 0644)) return SP_ERR_SYS;
        break;
#else
        return SP_ERR_SYS_UNSUPPORTED;
#endif
      }
    }
  }
  sp_test_kv_clear(t, "setup");
  return SP_OK;
}

static sp_err_t fs_create_deep_tree(sp_path_t root) {
  sp_try(sp_fs_create_dir_at(root));

  sp_sys_dir_t cur = sp_zero;
  sp_try(sp_fs_open_dir_at(root, &cur));
  sp_for(level, FS_DEEP_LEVELS) {
    sp_path_t child = sp_path(cur, sp_str_lit(FS_DEEP_NAME));
    sp_sys_dir_t next = sp_zero;
    sp_err_t err = sp_fs_create_dir_at(child);
    if (!err) err = sp_fs_open_dir_at(child, &next);
    sp_fs_close_dir(cur);
    sp_try(err);
    cur = next;
  }
  sp_err_t err = sp_fs_create_file_at(sp_path(cur, sp_str_lit("F")));
  sp_fs_close_dir(cur);
  return err;
}

static void fs_expect_no_temps(sp_test_t* t, sp_path_t sandbox) {
  sp_sys_dir_t dir = sp_zero;
  sp_err_t err = sp_fs_open_dir_at(sandbox, &dir);
  sp_expect_ok(t, err);
  if (err) return;

  sp_fs_it_entry_t entry = sp_zero;
  sp_fs_it_t it = sp_fs_it(sp_test_arena(t), dir);
  while (sp_fs_it_next(&it, &entry)) {
    if (sp_str_ends_with(entry.name, sp_str_lit(".tmp"))) {
      sp_test_fail(t, "temp file left behind: {}", sp_fmt_str(entry.path));
    }
  }
  sp_expect_ok(t, it.err);
  sp_fs_it_deinit(&it);
  sp_fs_close_dir(dir);
}

static void fs_expect_paths(sp_test_t* t, sp_path_t sandbox, const fs_expected_path_t expected [FS_MAX_PATHS]) {
  sp_mem_t mem = sp_test_arena(t);
  sp_for(it, FS_MAX_PATHS) {
    const fs_expected_path_t* info = &expected[it];
    if (!info->path) break;

    sp_str_t label = sp_cstr_as_str(info->path);
    sp_path_t path = sp_path_join(mem, sandbox, label);

    bool exists = sp_fs_exists_at(path);
    if (exists != info->exists) {
      sp_test_fail(t, "expected {} {} exist", sp_fmt_str(label), sp_fmt_cstr(info->exists ? "to" : "not to"));
    }

    if (info->exists) {
      fs_expect_kind(t, label, sp_fs_get_kind_at(path), info->kind);
    }

    if (info->content) {
      sp_str_t actual = sp_zero;
      sp_io_read_file_at(mem, path, &actual);
      sp_str_t content = sp_cstr_as_str(info->content);
      if (!sp_str_equal(actual, content)) {
        sp_test_record(t, (sp_test_failure_t) {
          .message = sp_test_format(t, "content of {}", sp_fmt_str(label)),
          .expected = sp_test_format(t, "{.quote}", sp_fmt_str(content)),
          .actual = sp_test_format(t, "{.quote}", sp_fmt_str(actual)),
        });
      }
    }

    if (info->target) {
      c8 buf [SP_PATH_MAX];
      sp_str_t actual = sp_zero;
      sp_expect_ok(t, sp_sys_readlink_s(path.dir, path.sub, buf, sizeof(buf), &actual));
      sp_expect_str_eq_c(t, sp_fs_normalize_path(mem, actual), info->target);
    }
  }
}

#endif // FS_TEST_H
