#include "fs.h"

typedef enum {
  COPY_FILE,
  COPY_DIR,
  COPY_LINK,
} copy_action_t;

typedef struct {
  const c8* path;
  bool exists;
  sp_fs_kind_t attr;
  const c8* content;
} copy_expect_t;

typedef struct {
  const c8* label;
  fs_setup_t setup[16];
  copy_action_t action;
  const c8* src;
  const c8* dst;
  copy_expect_t expect[16];
} copy_test_t;

static void run_copy_test(s32* utest_result, sp_test_file_manager_t* fm, copy_test_t* t) {
  sp_str_t sandbox = sp_test_file_path(fm, sp_str_view(t->label));
  sp_fs_create_dir_a(sandbox);
  fs_apply_setup(utest_result, fm, sandbox, t->setup);

  sp_str_t src = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(t->src));
  sp_str_t dst = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(t->dst));

  switch (t->action) {
    case COPY_FILE: sp_fs_copy_file_a(src, dst); break;
    case COPY_DIR:  sp_fs_copy_a(src, dst); break;
    case COPY_LINK: sp_fs_link_a(src, dst, SP_FS_LINK_COPY); break;
  }

  sp_for(i, 16) {
    copy_expect_t* exp = &t->expect[i];
    if (!exp->path) break;

    sp_str_t path = sp_fs_join_path_a(fm->mem, sandbox, sp_str_view(exp->path));
    bool exists = sp_fs_exists_a(path);
    fs_expect_bool(utest_result, path, "exists", exists, exp->exists);

    if (exp->exists && exists) {
      fs_expect_attr(utest_result, path, sp_fs_get_kind_a(path), exp->attr);
      if (exp->content) {
        sp_str_t file_content = SP_ZERO_INITIALIZE();
        sp_io_read_file_a(fm->mem, path, &file_content);
        SP_EXPECT_STR_EQ(file_content, sp_str_view(exp->content));
      }
    }
  }
}

UTEST_F(fs, copy_file_basic) {
  run_copy_test(&ur, &ut.file_manager, &(copy_test_t) {
    .label = "copy_file_basic",
    .setup = {
      { .path = "source.txt", .kind = FS_SETUP_FILE, .content = "hello world" },
    },
    .action = COPY_FILE,
    .src = "source.txt",
    .dst = "dest.txt",
    .expect = {
      { .path = "source.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "hello world" },
      { .path = "dest.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "hello world" },
    },
  });
}

UTEST_F(fs, copy_file_via_link) {
  run_copy_test(&ur, &ut.file_manager, &(copy_test_t) {
    .label = "copy_file_via_link",
    .setup = {
      { .path = "source.txt", .kind = FS_SETUP_FILE, .content = "test content" },
    },
    .action = COPY_LINK,
    .src = "source.txt",
    .dst = "copy.txt",
    .expect = {
      { .path = "source.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "test content" },
      { .path = "copy.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "test content" },
    },
  });
}

UTEST_F(fs, copy_dir_basic) {
  run_copy_test(&ur, &ut.file_manager, &(copy_test_t) {
    .label = "copy_dir_basic",
    .setup = {
      { .path = "src", .kind = FS_SETUP_DIR },
      { .path = "src/a.txt", .kind = FS_SETUP_FILE, .content = "aaa" },
      { .path = "src/b.txt", .kind = FS_SETUP_FILE, .content = "bbb" },
      { .path = "dst", .kind = FS_SETUP_DIR },
    },
    .action = COPY_DIR,
    .src = "src",
    .dst = "dst",
    .expect = {
      { .path = "dst/src", .exists = true, .attr = SP_FS_KIND_DIR },
      { .path = "dst/src/a.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "aaa" },
      { .path = "dst/src/b.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "bbb" },
    },
  });
}

UTEST_F(fs, copy_dir_nested) {
  run_copy_test(&ur, &ut.file_manager, &(copy_test_t) {
    .label = "copy_dir_nested",
    .setup = {
      { .path = "src", .kind = FS_SETUP_DIR },
      { .path = "src/sub", .kind = FS_SETUP_DIR },
      { .path = "src/a.txt", .kind = FS_SETUP_FILE, .content = "top" },
      { .path = "src/sub/b.txt", .kind = FS_SETUP_FILE, .content = "deep" },
      { .path = "dst", .kind = FS_SETUP_DIR },
    },
    .action = COPY_DIR,
    .src = "src",
    .dst = "dst",
    .expect = {
      { .path = "dst/src", .exists = true, .attr = SP_FS_KIND_DIR },
      { .path = "dst/src/a.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "top" },
      { .path = "dst/src/sub", .exists = true, .attr = SP_FS_KIND_DIR },
      { .path = "dst/src/sub/b.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "deep" },
    },
  });
}

UTEST_F(fs, copy_dir_with_nonalphanumeric) {
  run_copy_test(&ur, &ut.file_manager, &(copy_test_t) {
    .label = "copy_dir_with_nonalphanumeric",
    .setup = {
      { .path = "foo.bar", .kind = FS_SETUP_DIR },
      { .path = "baz", .kind = FS_SETUP_DIR },
    },
    .action = COPY_DIR,
    .src = "foo.bar",
    .dst = "baz",
    .expect = {
      { .path = "baz/foo.bar", .exists = true, .attr = SP_FS_KIND_DIR },
    },
  });
}

UTEST_F(fs, unicode_copy_file) {
  run_copy_test(&ur, &ut.file_manager, &(copy_test_t) {
    .label = "unicode_copy_file",
    .setup = {
      { .path = "\xc3\xb6riginal.txt", .kind = FS_SETUP_FILE, .content = "hello" },
    },
    .action = COPY_FILE,
    .src = "\xc3\xb6riginal.txt",
    .dst = "\xc3\xbc\x63opy.txt",
    .expect = {
      { .path = "\xc3\xb6riginal.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "hello" },
      { .path = "\xc3\xbc\x63opy.txt", .exists = true, .attr = SP_FS_KIND_FILE, .content = "hello" },
    },
  });
}

#if defined(SP_POSIX)
UTEST_F(fs, copy_preserves_file_attributes) {
  sp_mem_t a = ut.file_manager.mem;
  sp_str_t source_file = sp_test_file_create_empty(&ut.file_manager, SP_LIT("source_attrs.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = source_file,
    .content = SP_LIT("preserved content"),
  });

  ASSERT_EQ(chmod(sp_cstr_from_str_a(a, source_file), 0755), 0);

  struct stat original_stat = {0};
  ASSERT_EQ(stat(sp_cstr_from_str_a(a, source_file), &original_stat), 0);

  sp_str_t copy_file = sp_test_file_path(&ut.file_manager, SP_LIT("copy_attrs.txt"));
  ASSERT_EQ(sp_fs_copy_a(source_file, copy_file), SP_OK);
  ASSERT_TRUE(sp_fs_is_file_a(copy_file));

  struct stat copy_stat = {0};
  ASSERT_EQ(stat(sp_cstr_from_str_a(a, copy_file), &copy_stat), 0);

  ASSERT_EQ(original_stat.st_mode, copy_stat.st_mode);
  ASSERT_EQ(original_stat.st_size, copy_stat.st_size);
  sp_str_t preserved = SP_ZERO_INITIALIZE();
  sp_io_read_file_a(a, copy_file, &preserved);
  SP_EXPECT_STR_EQ(preserved, SP_LIT("preserved content"));
}
#endif

SP_TEST_MAIN()
