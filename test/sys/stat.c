#include "sys.h"

UTEST_EMPTY_FIXTURE(sys_stat)

UTEST_F(sys_stat, reports_file_kind_and_size) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_reports_file_kind_and_size",
    .setup = {
      { .path = "file.bin", .content = "ABCDE" },
    },
    .steps = {
      { .kind = SYS_STEP_STAT, .stat = { .path = "file.bin", .kind = SP_FS_KIND_FILE, .size = 5 } },
    },
  });
}

UTEST_F(sys_stat, reports_directory_kind) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_reports_directory_kind",
    .setup = {
      { .path = "dir", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_STAT, .stat = { .path = "dir", .kind = SP_FS_KIND_DIR } },
    },
  });
}

UTEST_F(sys_stat, refuses_missing_path) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_STAT, .stat = { .path = "file.bin", .fail = true } },
    },
  });
}

UTEST_F(sys_stat, follows_symlinks) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_follows_symlinks",
    .setup = {
      { .path = "target.bin", .content = "AB" },
      { .path = "lnk", .kind = SYS_SETUP_SYMLINK, .target = "target.bin" },
    },
    .steps = {
      { .kind = SYS_STEP_STAT, .stat = { .path = "lnk", .kind = SP_FS_KIND_FILE, .size = 2 } },
    },
  });
}

UTEST_F(sys_stat, lstat_reports_symlink) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_lstat_reports_symlink",
    .setup = {
      { .path = "target.bin", .content = "AB" },
      { .path = "lnk", .kind = SYS_SETUP_SYMLINK, .target = "target.bin" },
    },
    .steps = {
      { .kind = SYS_STEP_LSTAT, .lstat = { .path = "lnk", .kind = SP_FS_KIND_SYMLINK } },
    },
  });
}

UTEST_F(sys_stat, fstat_reports_file_kind_and_size) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_fstat_reports_file_kind_and_size",
    .setup = {
      { .path = "file.bin", .content = "ABC" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin" } },
      { .kind = SYS_STEP_FSTAT, .fstat = { .kind = SP_FS_KIND_FILE, .size = 3 } },
    },
  });
}

UTEST_F(sys_stat, fstat_reports_directory_kind) {
  run_sys_test(utest_result, (sys_test_t) {
    .label = "sys_stat_fstat_reports_directory_kind",
    .setup = {
      { .path = "dir", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "dir" } },
      { .kind = SYS_STEP_FSTAT, .fstat = { .kind = SP_FS_KIND_DIR } },
    },
  });
}
