#ifndef FS_TEST_H
#define FS_TEST_H

#include "sp.h"
#include "sp/sp_test.h"

#if defined(SP_POSIX)
  #include "sys/stat.h"
#endif

#define FS_MAX_SETUP 8
#define FS_MAX_PATHS 8


/////////////
// SANDBOX //
/////////////
static sp_str_t fs_path(sp_test_t* t, sp_str_t relative) {
  return sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), relative);
}

static sp_str_t fs_path_c(sp_test_t* t, const c8* relative) {
  return fs_path(t, sp_cstr_as_str(relative));
}


///////////////////
// SYMLINK PROBE //
///////////////////
static sp_test_once_t fs_symlink_probe = sp_zero;

static sp_err_t fs_probe_symlinks(void* user) {
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

static bool fs_symlinks_available(sp_test_t* t) {
  sp_str_t dir = sp_test_dir(t);
  return sp_test_once(&fs_symlink_probe, fs_probe_symlinks, &dir) == SP_OK;
}

#define fs_skip_if_no_symlinks(t) \
  do { if (!fs_symlinks_available(t)) return sp_test_skip(t, "symlinks not available"); } while (0)


////////////////////
// SHARED HARNESS //
////////////////////
typedef enum {
  FS_SETUP_FILE,
  FS_SETUP_DIR,
  FS_SETUP_SYMLINK,
  FS_SETUP_HARD_LINK,
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
} fs_expected_path_t;

static bool fs_setup_needs_symlinks(const fs_setup_t setup [FS_MAX_SETUP]) {
  sp_for(it, FS_MAX_SETUP) {
    if (!setup[it].path) break;
    if (setup[it].kind == FS_SETUP_SYMLINK) return true;
  }
  return false;
}

#define skip_if_symlinks_needed(T, SETUP) \
  do { if (fs_setup_needs_symlinks(SETUP)) fs_skip_if_no_symlinks(T); } while (0)

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

static void fs_apply_setup(sp_test_t* t, sp_str_t sandbox, const fs_setup_t setup [FS_MAX_SETUP]) {
  sp_mem_t mem = sp_test_arena(t);
  sp_for(it, FS_MAX_SETUP) {
    const fs_setup_t* ent = &setup[it];
    if (!ent->path) break;

    sp_str_t path = sp_fs_join_path(mem, sandbox, sp_str_view(ent->path));
    sp_str_t parent = sp_fs_parent_path(path);

    if (!sp_str_empty(parent) && !sp_str_equal(parent, path) && !sp_fs_exists(parent)) {
      sp_fs_create_dir(parent);
    }

    switch (ent->kind) {
      case FS_SETUP_FILE: {
        sp_fs_create_file_str(path, ent->content ? sp_str_view(ent->content) : sp_str_lit(""));
        break;
      }
      case FS_SETUP_DIR: {
        sp_fs_create_dir(path);
        break;
      }
      case FS_SETUP_SYMLINK: {
        sp_str_t target = sp_fs_join_path(mem, sandbox, sp_str_view(ent->target));
        if (sp_fs_create_sym_link(target, path) != SP_OK) {
          sp_test_fail(t, "failed to create symlink {} -> {}", sp_fmt_str(path), sp_fmt_str(target));
        }
        break;
      }
      case FS_SETUP_HARD_LINK: {
        sp_str_t target = sp_fs_join_path(mem, sandbox, sp_str_view(ent->target));
        if (sp_fs_create_hard_link(target, path) != SP_OK) {
          sp_test_fail(t, "failed to create hard link {} -> {}", sp_fmt_str(path), sp_fmt_str(target));
        }
        break;
      }
      case FS_SETUP_FIFO: {
#if defined(SP_POSIX)
        if (mkfifo(sp_cstr_from_str(mem, path), 0644) != 0) {
          sp_test_fail(t, "failed to create fifo {}", sp_fmt_str(path));
        }
#else
        sp_test_fail(t, "fifo setup requires posix");
#endif
        break;
      }
    }
  }
}

static void fs_expect_paths(sp_test_t* t, sp_str_t sandbox, const fs_expected_path_t expected [FS_MAX_PATHS]) {
  sp_mem_t mem = sp_test_arena(t);
  sp_for(it, FS_MAX_PATHS) {
    const fs_expected_path_t* info = &expected[it];
    if (!info->path) break;

    sp_str_t path = sp_fs_join_path(mem, sandbox, sp_str_view(info->path));

    bool exists = sp_fs_exists(path);
    if (exists != info->exists) {
      sp_test_fail(t, "expected {} {} exist", sp_fmt_str(path), sp_fmt_cstr(info->exists ? "to" : "not to"));
    }

    if (info->exists) {
      fs_expect_kind(t, path, sp_fs_get_kind(path), info->kind);
    }

    if (info->content) {
      sp_str_t actual = sp_zero;
      sp_io_read_file(mem, path, &actual);
      sp_str_t content = sp_str_view(info->content);
      if (!sp_str_equal(actual, content)) {
        sp_test_record(t, (sp_test_failure_t) {
          .message = sp_test_format(t, "content of {}", sp_fmt_str(path)),
          .expected = sp_test_format(t, "{.quote}", sp_fmt_str(content)),
          .actual = sp_test_format(t, "{.quote}", sp_fmt_str(actual)),
        });
      }
    }
  }
}

#endif // FS_TEST_H
