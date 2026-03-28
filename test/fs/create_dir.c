#include "fs.h"

////////////////
// CREATE DIR //
////////////////
typedef enum {
  CREATE_DIR_ENT_FILE,
  CREATE_DIR_ENT_DIR,
  CREATE_DIR_ENT_SYMLINK,
} create_dir_ent_kind_t;

typedef struct {
  const c8* path;
  create_dir_ent_kind_t kind;
  const c8* symlink_target;
} create_dir_setup_ent_t;

typedef struct {
  const c8* path;
  bool exists;
  sp_fs_attr_t attr;
} create_dir_expected_ent_t;

typedef struct {
  const c8* label;
  const c8* target;
  create_dir_setup_ent_t setup[16];
  create_dir_expected_ent_t expected[16];
  bool expect_ok;
} create_dir_test_t;

struct fs_create_dir {
  sp_test_file_manager_t file_manager;
};

UTEST_F_SETUP(fs_create_dir) {
  sp_test_file_manager_init(&ut.file_manager);
  probe_symlinks(ut.file_manager.paths.test);
}

UTEST_F_TEARDOWN(fs_create_dir) {
  sp_test_file_manager_cleanup(&ut.file_manager);
}

static void run_create_dir_test(s32* utest_result, sp_test_file_manager_t* fm, create_dir_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir(sandbox);

  u32 setup_count = 0;
  while (setup_count < 16 && t->setup[setup_count].path) setup_count++;

  sp_for(i, setup_count) {
    create_dir_setup_ent_t* s = &t->setup[i];
    sp_str_t full = sp_fs_join_path(sandbox, sp_str_view(s->path));
    sp_str_t parent = sp_fs_parent_path(full);
    if (!sp_str_empty(parent) && !sp_fs_exists(parent)) {
      sp_fs_create_dir(parent);
    }

    switch (s->kind) {
      case CREATE_DIR_ENT_FILE: {
        sp_test_file_create_ex((sp_test_file_config_t) { .path = full });
        break;
      }
      case CREATE_DIR_ENT_DIR: {
        sp_fs_create_dir(full);
        break;
      }
      case CREATE_DIR_ENT_SYMLINK: {
        sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(s->symlink_target));
        ASSERT_EQ(sp_fs_create_sym_link(target, full), SP_OK);
        break;
      }
    }
  }

  sp_str_t target = sp_fs_join_path(sandbox, sp_str_view(t->target));
  sp_err_t result = sp_fs_create_dir(target);

  if (t->expect_ok && result) {
    SP_TEST_REPORT("{} does not exist with code {}", SP_FMT_STR(target), SP_FMT_S32(result));
    SP_FAIL();
  } else if (!t->expect_ok && !result){
    SP_TEST_REPORT("{} exists with code {}", SP_FMT_STR(target), SP_FMT_S32(result));
    SP_FAIL();
  }

  u32 expected_count = 0;
  while (expected_count < 16 && t->expected[expected_count].path) expected_count++;

  sp_for(i, expected_count) {
    create_dir_expected_ent_t* exp = &t->expected[i];
    sp_str_t expected_path = sp_fs_join_path(sandbox, sp_str_view(exp->path));
    bool exists = sp_fs_exists(expected_path);
    if (exists != exp->exists) {
      if (exp->exists) {
        SP_TEST_REPORT("expected {} to exist", SP_FMT_STR(expected_path));
      } else {
        SP_TEST_REPORT("expected {} not to exist", SP_FMT_STR(expected_path));
      }
      SP_FAIL();
    }

    if (exp->exists) {
      sp_fs_attr_t attr = sp_fs_get_file_attrs(expected_path);
      if (attr != exp->attr) {
        SP_TEST_REPORT(
          "{} had attr {} but expected {}",
          SP_FMT_STR(expected_path),
          SP_FMT_S32(attr),
          SP_FMT_S32(exp->attr)
        );
        SP_FAIL();
      }
    }
  }
}

UTEST_F(fs_create_dir, existing_directory) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "existing_directory",
    .target = "dir1",
    .setup = {
      { .path = "dir1", .kind = CREATE_DIR_ENT_DIR },
    },
    .expected = {
      { .path = "dir1", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
    .expect_ok = true,
  });
}

UTEST_F(fs_create_dir, create_one_level) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "create_one_level",
    .target = "dir1",
    .expected = {
      { .path = "dir1", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
    .expect_ok = true,
  });
}

UTEST_F(fs_create_dir, create_multi_level) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "create_multi_level",
    .target = "dir1/dir2",
    .expected = {
      { .path = "dir1", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
      { .path = "dir1/dir2", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
    },
    .expect_ok = true,
  });
}

UTEST_F(fs_create_dir, destination_is_file) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_is_file",
    .target = "file",
    .setup = {
      { .path = "file", .kind = CREATE_DIR_ENT_FILE },
    },
    .expected = {
      { .path = "file", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
    },
    .expect_ok = false,
  });
}

UTEST_F(fs_create_dir, destination_parent_is_file) {
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_parent_is_file",
    .target = "file/dir1",
    .setup = {
      { .path = "file", .kind = CREATE_DIR_ENT_FILE },
    },
    .expected = {
      { .path = "file", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "file/dir1", .exists = FS_EXPECT_NOT_EXIST, .attr = SP_OS_FILE_ATTR_NONE },
    },
    .expect_ok = false,
  });
}

#if defined(SP_POSIX)
UTEST_F(fs_create_dir, destination_is_symlink_to_directory) {
  SKIP_IF_NO_SYMLINKS();
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_is_symlink_to_directory",
    .target = "sym_name",
    .setup = {
      { .path = "dir", .kind = CREATE_DIR_ENT_DIR },
      { .path = "sym_name", .kind = CREATE_DIR_ENT_SYMLINK, .symlink_target = "dir" },
    },
    .expected = {
      { .path = "dir", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_DIRECTORY },
      { .path = "sym_name", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_SYMLINK },
    },
    .expect_ok = false,
  });
}

UTEST_F(fs_create_dir, destination_is_symlink_to_file) {
  SKIP_IF_NO_SYMLINKS();
  run_create_dir_test(&ur, &ut.file_manager, &(create_dir_test_t) {
    .label = "destination_is_symlink_to_file",
    .target = "sym_name",
    .setup = {
      { .path = "file", .kind = CREATE_DIR_ENT_FILE },
      { .path = "sym_name", .kind = CREATE_DIR_ENT_SYMLINK, .symlink_target = "file" },
    },
    .expected = {
      { .path = "file", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_REGULAR_FILE },
      { .path = "sym_name", .exists = FS_EXPECT_EXIST, .attr = SP_OS_FILE_ATTR_SYMLINK },
    },
    .expect_ok = false,
  });
}
#endif

SP_TEST_MAIN()
