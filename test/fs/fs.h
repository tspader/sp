#ifndef FS_TEST_H
#define FS_TEST_H

#include "sp.h"
#include "test.h"
#include "utest.h"

#if defined(SP_POSIX)
  #include "sys/stat.h"
#endif

static bool are_symlinks_available = false;

static void probe_symlinks(sp_mem_t a, sp_str_t test_dir) {
  static bool probed = false;
  if (probed) return;
  probed = true;

  sp_str_t target = sp_fs_join_path(a, test_dir, sp_str_lit(".symlink_probe_target"));
  sp_str_t link = sp_fs_join_path(a, test_dir, sp_str_lit(".symlink_probe_link"));
  sp_fs_create_file(target);
  are_symlinks_available = sp_fs_create_sym_link(target, link) == SP_OK;
  if (are_symlinks_available) sp_fs_remove_file(link);
  sp_fs_remove_file(target);
}

#define FS_EXPECT_EXIST true
#define FS_EXPECT_NOT_EXIST false

#define SKIP_IF_NO_SYMLINKS() \
  if (!are_symlinks_available) { UTEST_SKIP("symlinks not available"); }


//////////////////
// fs FIXTURE   //
//////////////////
struct fs {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(fs) {
  SKIP_ON_WASM()
  sp_test_file_manager_init(&ut.file_manager);
  probe_symlinks(ut.file_manager.mem, ut.file_manager.paths.test);
}

UTEST_F_TEARDOWN(fs) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

//////////////////////
// SHARED HARNESS   //
//////////////////////
typedef enum {
  FS_SETUP_FILE,
  FS_SETUP_DIR,
  FS_SETUP_SYMLINK,
  FS_SETUP_HARD_LINK,
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
  sp_fs_kind_t attr;
  const c8* content;
} fs_expected_path_t;

static u32 fs_count_setup(fs_setup_t* setup) {
  u32 n = 0;
  while (n < 16 && setup[n].path) n++;
  return n;
}

static u32 fs_count_expected_paths(fs_expected_path_t* expected) {
  u32 n = 0;
  while (n < 16 && expected[n].path) n++;
  return n;
}

static void fs_expect_bool(s32* utest_result, sp_str_t path, const c8* label, bool actual, bool expected) {
  if (actual == expected) return;

  SP_TEST_REPORT(
    "{} {} was {} but expected {}",
    sp_fmt_cstr(label),
    sp_fmt_str(path),
    sp_fmt_cstr(actual ? "true" : "false"),
    sp_fmt_cstr(expected ? "true" : "false")
  );
  SP_FAIL();
}

static void fs_expect_attr(s32* utest_result, sp_str_t path, sp_fs_kind_t actual, sp_fs_kind_t expected) {
  if (actual == expected) return;

  SP_TEST_REPORT(
    "{} had attr {} but expected {}",
    sp_fmt_str(path),
    sp_fmt_int(actual),
    sp_fmt_int(expected)
  );
  SP_FAIL();
}

static void fs_expect_paths(s32* utest_result, sp_test_file_manager_t* fs, sp_str_t sandbox, fs_expected_path_t* expected) {
  u32 expected_count = fs_count_expected_paths(expected);
  sp_for(i, expected_count) {
    fs_expected_path_t* info = &expected[i];
    sp_str_t path = sp_fs_join_path(fs->mem, sandbox, sp_str_view(info->path));

    bool exists = sp_fs_exists(path);
    if (exists != info->exists) {
      if (info->exists) {
        SP_TEST_REPORT("expected {} to exist", sp_fmt_str(path));
      } else {
        SP_TEST_REPORT("expected {} not to exist", sp_fmt_str(path));
      }
      SP_FAIL();
    }

    if (info->exists) {
      fs_expect_attr(utest_result, path, sp_fs_get_kind(path), info->attr);
    }

    if (info->content) {
      sp_str_t actual = sp_zero;
      sp_io_read_file(fs->mem, path, &actual);
      sp_str_t expected_content = sp_str_view(info->content);
      if (!sp_str_equal(actual, expected_content)) {
        SP_TEST_REPORT("{} content was {} but expected {}", sp_fmt_str(path), sp_fmt_str(actual), sp_fmt_str(expected_content));
        SP_FAIL();
      }
    }
  }
}

static void fs_apply_setup(s32* utest_result, sp_test_file_manager_t* fs, sp_str_t sandbox, fs_setup_t* setup) {
  u32 setup_count = fs_count_setup(setup);
  sp_for(i, setup_count) {
    fs_setup_t* ent = &setup[i];
    sp_str_t path = sp_fs_join_path(fs->mem, sandbox, sp_str_view(ent->path));
    sp_str_t parent = sp_fs_parent_path(path);

    if (!sp_str_empty(parent) && !sp_str_equal(parent, path) && !sp_fs_exists(parent)) {
      sp_fs_create_dir(parent);
    }

    switch (ent->kind) {
      case FS_SETUP_FILE: {
        sp_test_file_create_ex((sp_test_file_config_t) {
          .path = path,
          .content = ent->content ? sp_str_view(ent->content) : sp_str_lit(""),
        });
        break;
      }
      case FS_SETUP_DIR: {
        sp_fs_create_dir(path);
        break;
      }
      case FS_SETUP_SYMLINK: {
        sp_str_t target = sp_fs_join_path(fs->mem, sandbox, sp_str_view(ent->target));
        if (sp_fs_create_sym_link(target, path) != SP_OK) {
          SP_TEST_REPORT("failed to create symlink {} -> {}", sp_fmt_str(path), sp_fmt_str(target));
          SP_FAIL();
        }
        break;
      }
      case FS_SETUP_HARD_LINK: {
        sp_str_t target = sp_fs_join_path(fs->mem, sandbox, sp_str_view(ent->target));
        if (sp_fs_create_hard_link(target, path) != SP_OK) {
          SP_TEST_REPORT("failed to create hard link {} -> {}", sp_fmt_str(path), sp_fmt_str(target));
          SP_FAIL();
        }
        break;
      }
    }
  }
}

#endif // FS_TEST_H
