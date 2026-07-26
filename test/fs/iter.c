#include "fs.h"

#define FS_ITER_MAX_ENTRIES 8
#define FS_ITER_BULK_PREFIX "R"

#define FS_ITER_A16 "AAAAAAAAAAAAAAAA"
#define FS_ITER_A255 \
  FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 \
  FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 \
  FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 FS_ITER_A16 \
  "AAAAAAAAAAAAAAA"
#define FS_ITER_E "\xe2\x82\xac"
#define FS_ITER_E5 FS_ITER_E FS_ITER_E FS_ITER_E FS_ITER_E FS_ITER_E
#define FS_ITER_E25 FS_ITER_E5 FS_ITER_E5 FS_ITER_E5 FS_ITER_E5 FS_ITER_E5
#define FS_ITER_E255 FS_ITER_E25 FS_ITER_E25 FS_ITER_E25 FS_ITER_E5 FS_ITER_E5

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
} fs_iter_entry_t;

typedef struct {
  sp_err_t open;
  sp_err_t walk;
} fs_iter_expect_t;

typedef struct {
  const c8* label;
  fs_setup_t setup [16];
  const c8* dir;
  bool relative;
  u32 bulk;
  u64 cap;
  bool close_handle;
  fs_iter_entry_t entries [FS_ITER_MAX_ENTRIES];
  fs_iter_expect_t expect;
} fs_iter_test_t;

typedef struct {
  sp_str_t name;
  sp_fs_kind_t kind;
  bool seen;
} fs_iter_expected_t;

static void run_fs_iter_test(s32* utest_result, sp_test_file_manager_t* fm, fs_iter_test_t t) {
  sp_str_t sandbox = sp_test_file_create_dir(fm, t.label);
  sp_str_t dir_abs = t.dir ? sp_fs_join_path(fm->mem, sandbox, sp_cstr_as_str(t.dir)) : sandbox;
  sp_err_t walk = SP_OK;

  fs_apply_setup(utest_result, fm, sandbox, t.setup);

  sp_da(fs_iter_expected_t) expected = sp_da_new(fm->mem, fs_iter_expected_t);
  sp_carr_for(t.entries, it) {
    if (!t.entries[it].name) break;
    sp_da_push(expected, ((fs_iter_expected_t) {
      .name = sp_cstr_as_str(t.entries[it].name),
      .kind = t.entries[it].kind,
    }));
  }
  sp_for(it, t.bulk) {
    sp_str_t name = sp_fmt(fm->mem, "{}{}", sp_fmt_cstr(FS_ITER_BULK_PREFIX), sp_fmt_uint(it)).value;
    sp_test_file_create_ex((sp_test_file_config_t) {
      .path = sp_fs_join_path(fm->mem, dir_abs, name),
    });
    sp_da_push(expected, ((fs_iter_expected_t) {
      .name = name,
      .kind = SP_FS_KIND_FILE,
    }));
  }
  sp_sys_fd_t sandbox_fd = SP_SYS_INVALID_FD;
  if (t.relative) {
    sandbox_fd = sp_sys_open_dir_s(sp_sys_get_root(0), sandbox);
    if (sandbox_fd == SP_SYS_INVALID_FD) {
      SP_TEST_REPORT("failed to open sandbox {}", sp_fmt_str(sandbox));
      SP_FAIL();
      return;
    }
  }

  SP_ALIGNED u8 buf [4096];
  u64 cap = t.cap ? t.cap : sizeof(buf);

  sp_fs_dir_t iter = sp_zero;
  sp_err_t err = t.relative
    ? sp_fs_dir_open(&iter, sandbox_fd, sp_cstr_as_str(t.dir), sp_mem_slice(buf, cap))
    : sp_fs_dir_open(&iter, sp_sys_get_root(0), dir_abs, sp_mem_slice(buf, cap));

  EXPECT_EQ(err, t.expect.open);
  if (err != SP_OK) goto done;

  if (t.close_handle) {
    sp_sys_close((sp_sys_fd_t)iter.dir.handle);
  }

  while (true) {
    sp_fs_dir_entry_t entry = sp_zero;
    walk = sp_fs_dir_next(&iter, &entry);
    if (walk != SP_OK) break;
    if (!entry.name.data) break;

    if (sp_str_equal(entry.name, sp_str_lit(".")) || sp_str_equal(entry.name, sp_str_lit(".."))) {
      SP_TEST_REPORT("iterator produced dot entry {}", sp_fmt_str(entry.name));
      SP_FAIL();
      continue;
    }

    bool matched = false;
    sp_da_for(expected, it) {
      if (!sp_str_equal(entry.name, expected[it].name)) continue;
      matched = true;
      if (expected[it].seen) {
        SP_TEST_REPORT("iterator produced {} twice", sp_fmt_str(entry.name));
        SP_FAIL();
      }
      expected[it].seen = true;
      if (entry.kind != expected[it].kind) {
        SP_TEST_REPORT("{} has kind {} but expected {}", sp_fmt_str(entry.name), sp_fmt_int((s64)entry.kind), sp_fmt_int((s64)expected[it].kind));
        SP_FAIL();
      }
      break;
    }
    if (!matched) {
      SP_TEST_REPORT("iterator produced unexpected entry {}", sp_fmt_str(entry.name));
      SP_FAIL();
    }
  }

  EXPECT_EQ(walk, t.expect.walk);
  if (t.expect.walk == SP_OK) {
    sp_da_for(expected, it) {
      if (!expected[it].seen) {
        SP_TEST_REPORT("iterator never produced {}", sp_fmt_str(expected[it].name));
        SP_FAIL();
      }
    }
  }

  if (!t.close_handle) sp_fs_dir_close(&iter);
done:
  if (sandbox_fd != SP_SYS_INVALID_FD) sp_sys_close(sandbox_fd);
}

UTEST_F(fs, iter_lists_entries_with_kinds) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_kinds",
    .setup = {
      { "A", FS_SETUP_FILE },
      { "B", FS_SETUP_DIR },
      { "C", FS_SETUP_FILE },
    },
    .entries = {
      { "A", SP_FS_KIND_FILE },
      { "B", SP_FS_KIND_DIR },
      { "C", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, iter_symlink_kind) {
  SKIP_IF_NO_SYMLINKS();
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_symlink",
    .setup = {
      { "A", FS_SETUP_FILE },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .entries = {
      { "A", SP_FS_KIND_FILE },
      { "L", SP_FS_KIND_SYMLINK },
    },
  });
}

UTEST_F(fs, iter_empty_directory) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_empty",
  });
}

UTEST_F(fs, iter_refuses_missing_directory) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_missing",
    .dir = "M",
    .expect = {
      .open = SP_ERR_SYS_NOT_FOUND,
    },
  });
}

UTEST_F(fs, iter_refuses_file) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_file",
    .setup = {
      { "A", FS_SETUP_FILE },
    },
    .dir = "A",
    .expect = {
      .open = SP_ERR_SYS_NOT_DIR,
    },
  });
}

UTEST_F(fs, iter_refuses_undersized_buffer) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_small_buf",
    .cap = SP_SYS_DIR_MIN_BUF - 1,
    .expect = {
      .open = SP_ERR_SYS_BUG,
    },
  });
}

UTEST_F(fs, iter_refills_across_batches) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_bulk",
    .bulk = 96,
    .cap = SP_SYS_DIR_MIN_BUF,
  });
}

UTEST_F(fs, iter_honors_dirfd) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_dirfd",
    .setup = {
      { "S", FS_SETUP_DIR },
      { "S/A", FS_SETUP_FILE },
    },
    .dir = "S",
    .relative = true,
    .entries = {
      { "A", SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, iter_max_length_name) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_longname",
    .setup = {
      { FS_ITER_A255, FS_SETUP_FILE },
    },
    .entries = {
      { FS_ITER_A255, SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(fs, iter_long_multibyte_name) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_longname_mb",
    .setup = {
      { FS_ITER_E255, FS_SETUP_FILE },
    },
    .entries = {
      { FS_ITER_E255, SP_FS_KIND_FILE },
    },
  });
}

#if defined(SP_LINUX)
UTEST_F(fs, iter_reports_walk_failure_instead_of_end) {
  run_fs_iter_test(&ur, &ut.file_manager, (fs_iter_test_t) {
    .label = "iter_bad_fd",
    .setup = {
      { "A", FS_SETUP_FILE },
      { "B", FS_SETUP_FILE },
    },
    .close_handle = true,
    .expect = {
      .walk = SP_ERR_SYS_BAD_FD,
    },
  });
}
#endif
