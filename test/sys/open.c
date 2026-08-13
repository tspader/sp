#include "harness.h"

static const sys_case_t sys_open_cases [] = {
  {
    .name = "read_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  },
  {
    .name = "write_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  },
  {
    .name = "ro_forbids_write",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "B", .err = SP_ERR_SYS_BAD_FD } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  },
  {
    .name = "write_forbids_read",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_READ, .read = { .count = 1, .err = SP_ERR_SYS_BAD_FD } },
    },
  },
  {
    .name = "read_write_allows_both",
    .setup = {
      { .path = "file.bin", .content = "AB" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_RW } },
      { .kind = SYS_STEP_READ, .read = { .count = 2, .expect = "AB" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "CD" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "ABCD" },
    },
  },
  {
    .name = "write_preserves_content",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAA" },
    },
  },
  {
    .name = "write_refuses_directory",
    .setup = {
      { .path = "dir", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "dir", .mode = SP_SYS_OPEN_MODE_WO, .err = SP_ERR_SYS_IS_DIR } },
    },
  },
  {
    .name = "create_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  },
  {
    .name = "create_opens_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAA" },
    },
  },
  {
    .name = "excl_refuses_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_EXCLUSIVE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AAAA" },
    },
  },
  {
    .name = "excl_refuses_symlink",
    .setup = {
      { .path = "lnk", .kind = SYS_SETUP_SYMLINK, .target = "victim" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "lnk", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_EXCLUSIVE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "victim" },
    },
  },
  {
    .name = "excl_implies_create",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_EXCLUSIVE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true },
    },
  },
  {
    .name = "truncate_empties_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  },
  {
    .name = "truncate_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE, .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "file.bin" },
    },
  },
  {
    .name = "create_truncate_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  },
  {
    .name = "create_truncate_empties_existing_file",
    .setup = {
      { .path = "file.bin", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "" },
    },
  },
  {
    .name = "append_preserves_content",
    .setup = {
      { .path = "file.bin", .content = "AA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "BB" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "AABB" },
    },
  },
  {
    .name = "create_append_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "A" } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  },
  {
    // NT cannot truncate through an append-only handle, so the combination
    // is rejected everywhere.
    .name = "append_truncate_rejected",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_APPEND | SP_SYS_OPEN_TRUNCATE, .err = SP_ERR_SYS_INVALID } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  },
  {
    // POSIX leaves RO|TRUNCATE undefined (linux truncates, NT errors), so the
    // combination is rejected everywhere.
    .name = "ro_truncate_rejected",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_RO, .flags = SP_SYS_OPEN_TRUNCATE, .err = SP_ERR_SYS_INVALID } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  },
  {
    // 2^64 - 1 is WriteFile's write-to-EOF sentinel; POSIX rejects it as a
    // negative offset. The sentinel must not leak through as silent append.
    .name = "pwrite_refuses_max_offset",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "file.bin", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_PWRITE, .pwrite = { .data = "B", .offset = (u64)-1, .err = SP_ERR_SYS_INVALID } },
    },
    .expect = {
      { .path = "file.bin", .exists = true, .content = "A" },
    },
  },
};

static const sys_case_t sys_open_dir_cases [] = {
  {
    .name = "refuses_file",
    .setup = {
      { .path = "file.bin", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "file.bin", .err = SP_ERR_SYS_NOT_DIR } },
    },
  },
  {
    .name = "refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "dir", .err = SP_ERR_SYS_NOT_FOUND } },
    },
  },
};

sp_test_each_fn(sys, open, sys_case_t, sys_open_cases, sys_case_run);
sp_test_each_fn(sys, open_dir, sys_case_t, sys_open_dir_cases, sys_case_run);
