#include "harness.h"

static const sys_case_t sys_open_cases [] = {
  {
    .name = "read_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "A" },
    },
  },
  {
    .name = "write_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "A" },
    },
  },
  {
    .name = "ro_forbids_write",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "B", .err = SP_ERR_SYS_BAD_FD } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
  {
    .name = "write_forbids_read",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_READ, .read = { .count = 1, .err = SP_ERR_SYS_BAD_FD } },
    },
  },
  {
    .name = "read_write_allows_both",
    .setup = {
      { .path = "A", .content = "AB" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_RW } },
      { .kind = SYS_STEP_READ, .read = { .count = 2, .expect = "AB" } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "CD" } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "ABCD" },
    },
  },
  {
    .name = "write_preserves_content",
    .setup = {
      { .path = "A", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "AAAA" },
    },
  },
  {
    .name = "write_refuses_directory",
    .setup = {
      { .path = "D", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "D", .mode = SP_SYS_OPEN_MODE_WO, .err = SP_ERR_SYS_IS_DIR } },
    },
  },
  {
    .name = "create_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "" },
    },
  },
  {
    .name = "create_opens_existing_file",
    .setup = {
      { .path = "A", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "AAAA" },
    },
  },
  {
    .name = "create_applies_perms",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE, .read_only = true } },
    },
    .expect = {
      { .path = "A", .exists = true, .read_only = true },
    },
  },
  {
    .name = "create_keeps_perms_of_existing_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE, .read_only = true } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
  {
    .name = "excl_refuses_existing_file",
    .setup = {
      { .path = "A", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_EXCLUSIVE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "AAAA" },
    },
  },
  {
    .name = "excl_refuses_symlink",
    .setup = {
      { .path = "L", .kind = SYS_SETUP_SYMLINK, .target = "T" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "L", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_EXCLUSIVE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "T" },
    },
  },
  {
    .name = "excl_implies_create",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_EXCLUSIVE } },
    },
    .expect = {
      { .path = "A", .exists = true },
    },
  },
  {
    .name = "truncate_empties_existing_file",
    .setup = {
      { .path = "A", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "" },
    },
  },
  {
    .name = "truncate_refuses_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE, .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "A" },
    },
  },
  {
    .name = "create_truncate_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "" },
    },
  },
  {
    .name = "create_truncate_empties_existing_file",
    .setup = {
      { .path = "A", .content = "AAAA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "" },
    },
  },
  {
    .name = "truncate_keeps_perms_of_existing_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_TRUNCATE, .read_only = true } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "" },
    },
  },
  {
    .name = "create_truncate_keeps_perms_of_existing_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE, .read_only = true } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "" },
    },
  },
  {
    .name = "append_preserves_content",
    .setup = {
      { .path = "A", .content = "AA" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "BB" } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "AABB" },
    },
  },
  {
    .name = "create_append_creates_missing_file",
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_CREATE | SP_SYS_OPEN_APPEND } },
      { .kind = SYS_STEP_WRITE, .write = { .data = "A" } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
  {
    // NT cannot truncate through an append-only handle, so the combination
    // is rejected everywhere.
    .name = "append_truncate_rejected",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO, .flags = SP_SYS_OPEN_APPEND | SP_SYS_OPEN_TRUNCATE, .err = SP_ERR_SYS_INVALID } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
  {
    // POSIX leaves RO|TRUNCATE undefined (linux truncates, NT errors), so the
    // combination is rejected everywhere.
    .name = "ro_truncate_rejected",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_RO, .flags = SP_SYS_OPEN_TRUNCATE, .err = SP_ERR_SYS_INVALID } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
  {
    // 2^64 - 1 is WriteFile's write-to-EOF sentinel; POSIX rejects it as a
    // negative offset. The sentinel must not leak through as silent append.
    .name = "pwrite_refuses_max_offset",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN, .open = { .path = "A", .mode = SP_SYS_OPEN_MODE_WO } },
      { .kind = SYS_STEP_PWRITE, .pwrite = { .data = "B", .offset = (u64)-1, .err = SP_ERR_SYS_INVALID } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
    },
  },
};

static const sys_case_t sys_open_dir_cases [] = {
  {
    .name = "refuses_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "A", .err = SP_ERR_SYS_NOT_DIR } },
    },
  },
  {
    .name = "refuses_missing_path",
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "D", .err = SP_ERR_SYS_NOT_FOUND } },
    },
  },
  {
    .name = "accepts_trailing_slash",
    .setup = {
      { .path = "A", .kind = SYS_SETUP_DIR },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "A/" } },
    },
  },
  {
    .name = "follows_dir_symlink",
    .setup = {
      { .path = "D", .kind = SYS_SETUP_DIR },
      { .path = "L", .kind = SYS_SETUP_DIR_SYMLINK, .target = "D" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "L" } },
    },
  },
  {
    .name = "nofollow_refuses_dir_symlink",
    .setup = {
      { .path = "D", .kind = SYS_SETUP_DIR },
      { .path = "L", .kind = SYS_SETUP_DIR_SYMLINK, .target = "D" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "L", .flags = SP_SYS_OPEN_DIR_NO_FOLLOW, .err = SP_ERR_SYS_LOOP } },
    },
  },
  {
    .name = "nofollow_refuses_file_symlink",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "L", .kind = SYS_SETUP_SYMLINK, .target = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .path = "L", .flags = SP_SYS_OPEN_DIR_NO_FOLLOW, .err = SP_ERR_SYS_LOOP } },
    },
  },
};

sp_test_each_fn(sys, open, sys_case_t, sys_open_cases, sys_case_run);
sp_test_each_fn(sys, open_dir, sys_case_t, sys_open_dir_cases, sys_case_run);

#if defined(SP_WIN32)
sp_test(sys, open_truncate_keeps_hidden) {
  sp_path_t path = sp_path_join(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_must_ok(t, sp_fs_write_at(path, sp_str_lit("A"), SP_FS_WRITE_TRUNCATE, sp_sys_default_file_perms));

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(path.dir, path.sub, &meta));
  meta.perms.value |= FILE_ATTRIBUTE_HIDDEN;
  sp_must_ok(t, sp_sys_set_file_perms_s(path.dir, path.sub, meta.perms));

  sp_sys_fd_t fd = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_sys_open_s(path.dir, path.sub, SP_SYS_OPEN_MODE_WO, SP_SYS_OPEN_CREATE | SP_SYS_OPEN_TRUNCATE, sp_sys_default_file_perms, &fd));
  sp_sys_close(fd);

  sp_sys_file_meta_t after = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(path.dir, path.sub, &after));
  sp_expect_eq(t, after.size, 0);
  sp_expect(t, after.perms.value & FILE_ATTRIBUTE_HIDDEN);
  return SP_OK;
}
#endif
