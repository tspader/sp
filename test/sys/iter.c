#include "sys.h"

#define SYS_ITER_MAX_EXPECT 8
#define SYS_ITER_BULK_MAX 128
#define SYS_ITER_BULK_PREFIX "refill_entry_"

typedef struct {
  const c8* name;
  sp_fs_kind_t kind;
} sys_iter_entry_t;

typedef struct {
  const c8* label;
  sys_setup_t setup [SYS_TEST_MAX_SETUP];
  const c8* dir;
  bool relative;
  u32 bulk;
  bool open_fails;
  sys_iter_entry_t entries [SYS_ITER_MAX_EXPECT];
} sys_iter_test_t;

UTEST_EMPTY_FIXTURE(sys_iter)

static void run_sys_iter_test(s32* utest_result, sys_iter_test_t t) {
  sp_test_file_manager_t fm = sp_zero;
  sp_test_file_manager_init(&fm);

  sp_str_t sandbox = sp_test_file_create_dir(&fm, t.label);
  sp_sys_fd_t sandbox_fd = sp_sys_open_dir_s(sp_sys_get_root(0), sandbox);
  if (sandbox_fd == SP_SYS_INVALID_FD) {
    SP_TEST_REPORT("failed to open sandbox {}", sp_fmt_str(sandbox));
    SP_FAIL();
    sp_test_file_manager_cleanup(&fm);
    return;
  }

  bool opened = false;
  sp_sys_fs_it_t iter = sp_zero;
  sp_str_t dir_abs = t.dir ? sp_fs_join_path(fm.mem, sandbox, sp_cstr_as_str(t.dir)) : sandbox;
  sp_str_t bulk_prefix = sp_str_lit(SYS_ITER_BULK_PREFIX);
  sp_sys_fs_entry_t entry = sp_zero;
  bool seen [SYS_ITER_MAX_EXPECT] = sp_zero;
  u8 bulk_seen [SYS_ITER_BULK_MAX] = sp_zero;
  SP_ALIGNED u8 buf [2048];
  s32 rc = 0;

  if (!sys_apply_setup(utest_result, &fm, sandbox, t.setup)) goto done;

  sp_for(it, t.bulk) {
    sp_str_t name = sp_fmt(fm.mem, "{}{}", sp_fmt_cstr(SYS_ITER_BULK_PREFIX), sp_fmt_uint(it)).value;
    sp_test_file_create_ex((sp_test_file_config_t) {
      .path = sp_fs_join_path(fm.mem, dir_abs, name),
    });
  }

  if (t.relative) {
    rc = sp_sys_fs_it_open_s(sandbox_fd, &iter, sp_cstr_as_str(t.dir), sp_mem_slice(buf, sizeof(buf)));
  }
  else {
    rc = sp_sys_fs_it_open_s(sp_sys_get_root(0), &iter, dir_abs, sp_mem_slice(buf, sizeof(buf)));
  }

  if (t.open_fails) {
    if (rc == 0) {
      SP_TEST_REPORT("open of {} succeeded but expected failure", sp_fmt_str(dir_abs));
      SP_FAIL();
      sp_sys_fs_it_close(&iter);
    }
    goto done;
  }
  if (rc != 0) {
    SP_TEST_REPORT("failed to open iterator on {}", sp_fmt_str(dir_abs));
    SP_FAIL();
    goto done;
  }
  opened = true;

  while (sp_sys_fs_it_next(&iter, &entry) == 0) {
    sp_str_t name = sp_str((c8*)entry.name, entry.len);

    if (sp_str_equal(name, sp_str_lit(".")) || sp_str_equal(name, sp_str_lit(".."))) {
      SP_TEST_REPORT("iterator produced dot entry {}", sp_fmt_str(name));
      SP_FAIL();
      continue;
    }

    bool matched = false;
    sp_carr_for(t.entries, it) {
      if (!t.entries[it].name) break;
      if (!sp_str_equal_cstr(name, t.entries[it].name)) continue;
      matched = true;
      if (seen[it]) {
        SP_TEST_REPORT("iterator produced {} twice", sp_fmt_str(name));
        SP_FAIL();
      }
      seen[it] = true;
      if (entry.kind != t.entries[it].kind) {
        SP_TEST_REPORT("{} has kind {} but expected {}", sp_fmt_str(name), sp_fmt_int((s64)entry.kind), sp_fmt_int((s64)t.entries[it].kind));
        SP_FAIL();
      }
      break;
    }
    if (matched) continue;

    if (t.bulk && sp_str_starts_with(name, bulk_prefix)) {
      u32 index = 0;
      bool valid = sp_parse_u32_ex(sp_str_sub(name, bulk_prefix.len, name.len - bulk_prefix.len), &index);
      if (valid && index < t.bulk) {
        if (bulk_seen[index]) {
          SP_TEST_REPORT("iterator produced {} twice", sp_fmt_str(name));
          SP_FAIL();
        }
        bulk_seen[index] = 1;
        continue;
      }
    }

    SP_TEST_REPORT("iterator produced unexpected entry {}", sp_fmt_str(name));
    SP_FAIL();
  }

  sp_carr_for(t.entries, it) {
    if (!t.entries[it].name) break;
    if (!seen[it]) {
      SP_TEST_REPORT("iterator never produced {}", sp_fmt_cstr(t.entries[it].name));
      SP_FAIL();
    }
  }
  sp_for(it, t.bulk) {
    if (!bulk_seen[it]) {
      SP_TEST_REPORT("iterator never produced {}{}", sp_fmt_cstr(SYS_ITER_BULK_PREFIX), sp_fmt_uint(it));
      SP_FAIL();
    }
  }

done:
  if (opened) sp_sys_fs_it_close(&iter);
  sp_sys_close(sandbox_fd);
  sp_test_file_manager_cleanup(&fm);
}

UTEST_F(sys_iter, lists_entries_with_kinds) {
  run_sys_iter_test(utest_result, (sys_iter_test_t) {
    .label = "sys_iter_lists_entries_with_kinds",
    .setup = {
      { .path = "a.bin", .content = "A" },
      { .path = "sub", .kind = SYS_SETUP_DIR },
      { .path = "b.bin", .content = "B" },
    },
    .entries = {
      { .name = "a.bin", .kind = SP_FS_KIND_FILE },
      { .name = "sub", .kind = SP_FS_KIND_DIR },
      { .name = "b.bin", .kind = SP_FS_KIND_FILE },
    },
  });
}

UTEST_F(sys_iter, empty_directory_has_no_entries) {
  run_sys_iter_test(utest_result, (sys_iter_test_t) {
    .label = "sys_iter_empty_directory_has_no_entries",
  });
}

UTEST_F(sys_iter, refuses_missing_directory) {
  run_sys_iter_test(utest_result, (sys_iter_test_t) {
    .label = "sys_iter_refuses_missing_directory",
    .dir = "missing",
    .open_fails = true,
  });
}

UTEST_F(sys_iter, refills_buffer_across_batches) {
  run_sys_iter_test(utest_result, (sys_iter_test_t) {
    .label = "sys_iter_refills_buffer_across_batches",
    .bulk = 96,
  });
}

UTEST_F(sys_iter, honors_dirfd) {
  run_sys_iter_test(utest_result, (sys_iter_test_t) {
    .label = "sys_iter_honors_dirfd",
    .setup = {
      { .path = "sub", .kind = SYS_SETUP_DIR },
      { .path = "sub/a.bin", .content = "A" },
    },
    .dir = "sub",
    .relative = true,
    .entries = {
      { .name = "a.bin", .kind = SP_FS_KIND_FILE },
    },
  });
}
