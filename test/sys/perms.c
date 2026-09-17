#include "sp.h"
#include "sp/sp_test.h"

#define PERMS_TEST_MAX_OPS 2

typedef enum {
  OP_NONE,
  OP_READ_ONLY,
  OP_EXECUTABLE,
} op_kind_t;

typedef struct {
  op_kind_t kind;
  bool value;
} op_t;

typedef struct {
  bool read_only;
  bool executable;
} expect_t;

typedef struct {
  const c8* name;
  op_t ops [PERMS_TEST_MAX_OPS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "fresh_file_is_writable" },
  { .name = "set_read_only", .ops = { { OP_READ_ONLY, true } }, .expect = { .read_only = true } },
  { .name = "clear_read_only", .ops = { { OP_READ_ONLY, true }, { OP_READ_ONLY, false } } },
  { .name = "set_executable", .ops = { { OP_EXECUTABLE, true } }, .expect = { .executable = true } },
  { .name = "clear_executable", .ops = { { OP_EXECUTABLE, true }, { OP_EXECUTABLE, false } } },
  { .name = "read_only_keeps_executable", .ops = { { OP_EXECUTABLE, true }, { OP_READ_ONLY, true } }, .expect = { .read_only = true, .executable = true } },
};

static sp_err_t run(sp_test_t* t, test_t* it) {
  sp_str_t path = sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_fs_create_file(path);
  sp_sys_fd_t root = sp_sys_get_root(0);

  sp_sys_file_meta_t meta = sp_zero;
  sp_err_t err = sp_sys_get_path_metadata_s(root, path, &meta);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "metadata not supported");
  sp_try(err);

  sp_carr_for_until(it->ops, i, it->ops[i].kind != OP_NONE) {
    op_t op = it->ops[i];
    switch (op.kind) {
      case OP_NONE:       break;
      case OP_READ_ONLY:  sp_sys_set_read_only(&meta.perms, op.value); break;
      case OP_EXECUTABLE: sp_sys_set_executable(&meta.perms, op.value); break;
    }
  }
  sp_must_ok(t, sp_sys_set_file_perms_s(root, path, meta.perms));

  sp_sys_file_meta_t after = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(root, path, &after));
  sp_expect_eq(t, sp_sys_is_read_only(after.perms), it->expect.read_only);
#if !defined(SP_WIN32)
  sp_expect_eq(t, (after.perms.value & 0111) != 0, it->expect.executable);
#endif
  return SP_OK;
}

sp_test_each_fn(sys, perms, test_t, tests, run);

sp_test(sys, default_perms_then_read_only) {
  sp_str_t path = sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_fs_create_file(path);
  sp_sys_fd_t root = sp_sys_get_root(0);

  sp_err_t err = sp_sys_set_file_perms_s(root, path, sp_sys_default_file_perms);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "set_file_perms not supported");
  sp_try(err);

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(root, path, &meta));
  sp_expect(t, !sp_sys_is_read_only(meta.perms));

  sp_sys_set_read_only(&meta.perms, true);
  sp_must_ok(t, sp_sys_set_file_perms_s(root, path, meta.perms));

  sp_sys_file_meta_t after = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(root, path, &after));
  sp_expect(t, sp_sys_is_read_only(after.perms));
  return SP_OK;
}

sp_test(sys, mkdir_applies_perms) {
  sp_test_skip_on_win32();
  sp_str_t path = sp_fs_join_path(sp_test_arena(t), sp_test_dir(t), sp_str_lit("D"));
  sp_sys_fd_t root = sp_sys_get_root(0);

  sp_sys_file_perms_t perms = sp_sys_default_dir_perms;
  sp_sys_set_read_only(&perms, true);
  sp_err_t err = sp_sys_mkdir_s(root, path, perms);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "mkdir not supported");
  sp_try(err);

  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(root, path, &meta));
  sp_expect(t, sp_sys_is_read_only(meta.perms));
  return SP_OK;
}
