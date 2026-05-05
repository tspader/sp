#include "fs.h"

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  const c8* input;
  bool expect_nonempty;
  bool expect_empty;
  bool expect_no_trailing_slash;
  bool expect_no_backslash;
  const c8* expect_name;
  bool expect_exists;
} canon_test_t;

static void run_canon_test(s32* utest_result, sp_test_file_manager_t* fm, canon_test_t* t) {
  sp_str_t input;

  if (t->setup[0].path) {
    sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
    sp_fs_create_dir_a(sandbox);
    fs_apply_setup(utest_result, fm, sandbox, t->setup);
    input = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(t->input));
  } else {
    input = sp_str_view(t->input);
  }

  sp_str_t result = sp_fs_canonicalize_path_a(fm->mem, input);

  if (t->expect_nonempty && result.len == 0) {
    SP_TEST_REPORT("{}: expected nonempty", sp_fmt_cstr(t->label));
    SP_FAIL();
  }
  if (t->expect_empty && result.len != 0) {
    SP_TEST_REPORT("{}: expected empty, got {}", sp_fmt_cstr(t->label), sp_fmt_str(result));
    SP_FAIL();
  }
  if (t->expect_no_trailing_slash && result.len > 0 && result.data[result.len - 1] == '/') {
    SP_TEST_REPORT("{}: trailing slash", sp_fmt_cstr(t->label));
    SP_FAIL();
  }
  if (t->expect_no_backslash && result.len > 0) {
    sp_for(i, result.len) {
      if (result.data[i] == '\\') {
        SP_TEST_REPORT("{}: backslash in result", sp_fmt_cstr(t->label));
        SP_FAIL();
        break;
      }
    }
  }
  if (t->expect_name) {
    sp_str_t name = sp_fs_get_name(result);
    if (!sp_str_equal_cstr(name, t->expect_name)) {
      SP_TEST_REPORT("{}: expected name {}", sp_fmt_cstr(t->label), sp_fmt_cstr(t->expect_name));
      SP_FAIL();
    }
  }
  if (t->expect_exists && !sp_fs_exists_a(result)) {
    SP_TEST_REPORT("{}: expected result to exist", sp_fmt_cstr(t->label));
    SP_FAIL();
  }
}

// ---- existing file/dir cases ----

UTEST_F(fs, canon_dot_slash_dotdot) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_dot_slash_dotdot",
    .setup = {
      { .path = "dir", .kind = FS_SETUP_DIR },
      { .path = "dir/child", .kind = FS_SETUP_DIR },
    },
    .input = "dir/./child/..",
    .expect_nonempty = true,
    .expect_name = "dir",
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_unicode) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_unicode",
    .setup = {
      { .path = "\xc3\xa9t\xc3\xa9", .kind = FS_SETUP_DIR },
    },
    .input = "\xc3\xa9t\xc3\xa9",
    .expect_nonempty = true,
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_regular_file) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_regular_file",
    .setup = {
      { .path = "file.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .input = "file.txt",
    .expect_nonempty = true,
    .expect_exists = true,
    .expect_name = "file.txt",
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_directory) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_directory",
    .setup = {
      { .path = "mydir", .kind = FS_SETUP_DIR },
    },
    .input = "mydir",
    .expect_nonempty = true,
    .expect_exists = true,
    .expect_name = "mydir",
    .expect_no_trailing_slash = true,
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_nested_dotdot) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_nested_dotdot",
    .setup = {
      { .path = "a", .kind = FS_SETUP_DIR },
      { .path = "a/b", .kind = FS_SETUP_DIR },
      { .path = "a/b/c", .kind = FS_SETUP_DIR },
    },
    .input = "a/b/c/../../b",
    .expect_nonempty = true,
    .expect_name = "b",
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

// ---- nonexistent path cases: canonicalize returns empty ----

UTEST_F(fs, canon_nonexistent_returns_empty) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_nonexistent_returns_empty",
    .input = "/this/path/does/not/exist/at/all",
    .expect_empty = true,
  });
}

UTEST_F(fs, canon_empty_input) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_empty_input",
    .input = "",
    .expect_empty = true,
  });
}

UTEST_F(fs, canon_nonexistent_relative) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_nonexistent_relative",
    .input = "no_such_file.txt",
    .expect_empty = true,
  });
}

UTEST_F(fs, canon_nonexistent_with_dotdot) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_nonexistent_with_dotdot",
    .input = "no_such_dir/../also_missing.txt",
    .expect_empty = true,
  });
}

// ---- result is always normalized ----

UTEST_F(fs, canon_result_is_normalized) {
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_result_is_normalized",
    .setup = {
      { .path = "norm_test", .kind = FS_SETUP_DIR },
    },
    .input = "norm_test",
    .expect_nonempty = true,
    .expect_no_trailing_slash = true,
    .expect_no_backslash = true,
    .expect_exists = true,
  });
}

// ---- symlink resolution ----

UTEST_F(fs, canon_resolves_symlink_to_file) {
  SKIP_IF_NO_SYMLINKS();
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_resolves_symlink_to_file",
    .setup = {
      { .path = "real.txt", .kind = FS_SETUP_FILE, .content = "data" },
      { .path = "link.txt", .kind = FS_SETUP_SYMLINK, .target = "real.txt" },
    },
    .input = "link.txt",
    .expect_nonempty = true,
    .expect_name = "real.txt",
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_resolves_symlink_to_dir) {
  SKIP_IF_NO_SYMLINKS();
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_resolves_symlink_to_dir",
    .setup = {
      { .path = "real_dir", .kind = FS_SETUP_DIR },
      { .path = "link_dir", .kind = FS_SETUP_SYMLINK, .target = "real_dir" },
    },
    .input = "link_dir",
    .expect_nonempty = true,
    .expect_name = "real_dir",
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_resolves_chained_symlinks) {
  SKIP_IF_NO_SYMLINKS();
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_resolves_chained_symlinks",
    .setup = {
      { .path = "origin.txt", .kind = FS_SETUP_FILE, .content = "chain" },
      { .path = "link1.txt", .kind = FS_SETUP_SYMLINK, .target = "origin.txt" },
      { .path = "link2.txt", .kind = FS_SETUP_SYMLINK, .target = "link1.txt" },
    },
    .input = "link2.txt",
    .expect_nonempty = true,
    .expect_name = "origin.txt",
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

UTEST_F(fs, canon_symlink_with_dotdot) {
  SKIP_IF_NO_SYMLINKS();
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_symlink_with_dotdot",
    .setup = {
      { .path = "target.txt", .kind = FS_SETUP_FILE, .content = "here" },
      { .path = "sub", .kind = FS_SETUP_DIR },
      { .path = "sub/up_link.txt", .kind = FS_SETUP_SYMLINK, .target = "target.txt" },
    },
    .input = "sub/up_link.txt",
    .expect_nonempty = true,
    .expect_name = "target.txt",
    .expect_exists = true,
    .expect_no_backslash = true,
  });
}

// ---- idempotency ----

UTEST_F(fs, canon_idempotent) {
  sp_mem_t a = ut.file_manager.mem;
  run_canon_test(&ur, &ut.file_manager, &(canon_test_t) {
    .label = "canon_idempotent",
    .setup = {
      { .path = "stable.txt", .kind = FS_SETUP_FILE, .content = "x" },
    },
    .input = "stable.txt",
    .expect_nonempty = true,
    .expect_exists = true,
  });

  // run a second pass: canonicalizing an already-canonical path should be the same
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("canon_idempotent"));
  sp_str_t path = sp_fs_join_path_a(a, sandbox, SP_LIT("stable.txt"));
  sp_str_t first = sp_fs_canonicalize_path_a(a, path);
  sp_str_t second = sp_fs_canonicalize_path_a(a, first);
  SP_EXPECT_STR_EQ(first, second);
}

UTEST_F(fs, canon_exe_idempotent) {
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t exe = sp_fs_get_exe_path_a(a);
  sp_str_t canonical = sp_fs_canonicalize_path_a(a, exe);
  SP_EXPECT_STR_EQ(canonical, exe);
}

// ---- cwd interaction ----

UTEST_F(fs, canon_cwd_matches_dot) {
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t old_cwd = sp_fs_get_cwd_a(a);
  sp_str_t sandbox = sp_test_file_path(&ut.file_manager, SP_LIT("canon_cwd"));
  sp_fs_create_dir_a(sandbox);

  ASSERT_EQ(sp_sys_chdir(sp_cstr_from_str_a(a, sandbox)), 0);
  sp_str_t cwd = sp_fs_get_cwd_a(a);
  sp_str_t canonical_dot = sp_fs_canonicalize_path_a(a, SP_LIT("."));
  SP_EXPECT_STR_EQ(cwd, canonical_dot);
  ASSERT_EQ(sp_sys_chdir(sp_cstr_from_str_a(a, old_cwd)), 0);
}

SP_TEST_MAIN()
