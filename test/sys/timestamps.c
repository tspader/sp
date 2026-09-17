#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  FIELD_BTIME,
  FIELD_MTIME,
} field_t;

typedef enum {
  OP_WRITE,
  OP_SET_PERMS,
} op_t;

typedef struct {
  const c8* name;
  field_t field;
  op_t op;
  bool moves;
} test_t;

static const test_t tests [] = {
  { .name = "write_moves_mtime", .field = FIELD_MTIME, .op = OP_WRITE, .moves = true },
  { .name = "set_perms_keeps_mtime", .field = FIELD_MTIME, .op = OP_SET_PERMS },
  { .name = "write_keeps_btime", .field = FIELD_BTIME, .op = OP_WRITE },
  { .name = "set_perms_keeps_btime", .field = FIELD_BTIME, .op = OP_SET_PERMS },
};

static s64 field_ns(const sp_sys_file_meta_t* meta, field_t field) {
  sp_sys_timespec_t ts = field == FIELD_BTIME ? meta->btime : meta->mtime;
  return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_fs_join_path(mem, sp_test_dir(t), sp_str_lit("A"));
  sp_fs_create_file_str(path, sp_str_lit("A"));

  sp_sys_fd_t root = sp_sys_get_root(0);
  sp_sys_file_meta_t before = sp_zero;
  sp_err_t err = sp_sys_get_path_metadata_s(root, path, &before);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "metadata not supported");
  sp_try(err);

  sp_os_sleep_ms(20);

  switch (c->op) {
    case OP_WRITE: {
      sp_sys_fd_t fd = SP_SYS_INVALID_FD;
      sp_must_ok(t, sp_sys_open_s(root, path, SP_SYS_OPEN_MODE_WO, SP_SYS_OPEN_APPEND, &fd));
      u64 n = 0;
      sp_expect_ok(t, sp_sys_write(fd, "B", 1, &n));
      sp_sys_close(fd);
      break;
    }
    case OP_SET_PERMS: {
      sp_sys_file_perms_t perms = before.perms;
      sp_sys_set_read_only(&perms, true);
      sp_must_ok(t, sp_sys_set_file_perms_s(root, path, perms));
      break;
    }
  }

  sp_sys_file_meta_t after = sp_zero;
  sp_try(sp_sys_get_path_metadata_s(root, path, &after));

  bool moved = field_ns(&before, c->field) != field_ns(&after, c->field);
  if (moved != c->moves) {
    sp_test_fail(t, "expected the field {} move: {} -> {}",
      sp_fmt_cstr(c->moves ? "to" : "not to"),
      sp_fmt_int(field_ns(&before, c->field)), sp_fmt_int(field_ns(&after, c->field)));
  }
  return SP_OK;
}

sp_test_each_fn(sys, timestamps, test_t, tests, run);
