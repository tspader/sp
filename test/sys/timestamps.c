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
  sp_path_t path = sp_path_join(sp_test_arena(t), sp_test_dir(t), sp_str_lit("A"));
  sp_fs_create_file_str_at(path, sp_str_lit("A"));

  sp_sys_file_meta_t before = sp_zero;
  sp_err_t err = sp_sys_get_path_metadata_s(path.dir, path.sub, &before);
  if (err == SP_ERR_SYS_UNSUPPORTED) return sp_test_skip(t, "metadata not supported");
  sp_try(err);

  sp_os_sleep_ms(20);

  switch (c->op) {
    case OP_WRITE: {
      sp_sys_fd_t fd = SP_SYS_INVALID_FD;
      sp_must_ok(t, sp_sys_open_s(path.dir, path.sub, SP_SYS_OPEN_MODE_WO, SP_SYS_OPEN_APPEND, &fd));
      u64 n = 0;
      sp_expect_ok(t, sp_sys_write(fd, "B", 1, &n));
      sp_sys_close(fd);
      break;
    }
    case OP_SET_PERMS: {
      sp_test_skip_on_wasm();
      sp_sys_file_perms_t perms = before.perms;
      sp_sys_set_read_only(&perms, true);
      sp_must_ok(t, sp_sys_set_file_perms_s(path.dir, path.sub, perms));
      break;
    }
  }

  sp_sys_file_meta_t after = sp_zero;
  sp_try(sp_sys_get_path_metadata_s(path.dir, path.sub, &after));

  bool moved = field_ns(&before, c->field) != field_ns(&after, c->field);
  if (moved != c->moves) {
    sp_test_fail(t, "expected the field {} move: {} -> {}",
      sp_fmt_cstr(c->moves ? "to" : "not to"),
      sp_fmt_int(field_ns(&before, c->field)), sp_fmt_int(field_ns(&after, c->field)));
  }
  return SP_OK;
}

sp_test_each_fn(sys, timestamps, test_t, tests, run);

typedef enum {
  SETUP_FILE,
  SETUP_DIR,
  SETUP_LINK,
} setup_t;

typedef struct {
  const c8* set;
  const c8* kept;
} set_expect_t;

typedef struct {
  const c8* name;
  setup_t setup;
  const c8* path;
  set_expect_t expect;
} set_test_t;

static const set_test_t set_tests [] = {
  { .name = "file", .setup = SETUP_FILE, .path = "A", .expect = { .set = "A" } },
  { .name = "dir", .setup = SETUP_DIR, .path = "A", .expect = { .set = "A" } },
  { .name = "follows_link", .setup = SETUP_LINK, .path = "L", .expect = { .set = "A", .kept = "L" } },
};

static const sp_sys_timespec_t set_atime = { .tv_sec = 1000000000 };
static const sp_sys_timespec_t set_mtime = { .tv_sec = 1100000000 };

static sp_err_t run_set(sp_test_t* t, set_test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t a = sp_path_join(mem, sandbox, sp_str_lit("A"));

  switch (c->setup) {
    case SETUP_FILE: {
      sp_must_ok(t, sp_fs_create_file_at(a));
      break;
    }
    case SETUP_DIR: {
      sp_must_ok(t, sp_fs_create_dir_at(a));
      break;
    }
    case SETUP_LINK: {
      sp_test_skip_without_symlinks();
      sp_must_ok(t, sp_fs_create_file_at(a));
      sp_must_ok(t, sp_fs_create_sym_link_at(sp_str_lit("A"), sp_path_join(mem, sandbox, sp_str_lit("L")), SP_FS_KIND_FILE));
      break;
    }
  }

  sp_path_t path = sp_path_join(mem, sandbox, sp_cstr_as_str(c->path));
  sp_must_ok(t, sp_sys_set_times_s(path.dir, path.sub, set_atime, set_mtime));

  sp_path_t set = sp_path_join(mem, sandbox, sp_cstr_as_str(c->expect.set));
  sp_sys_file_meta_t meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(set.dir, set.sub, &meta));
  sp_expect_eq(t, meta.atime.tv_sec, set_atime.tv_sec);
  sp_expect_eq(t, meta.atime.tv_nsec, set_atime.tv_nsec);
  sp_expect_eq(t, meta.mtime.tv_sec, set_mtime.tv_sec);
  sp_expect_eq(t, meta.mtime.tv_nsec, set_mtime.tv_nsec);

  if (c->expect.kept) {
    sp_path_t kept = sp_path_join(mem, sandbox, sp_cstr_as_str(c->expect.kept));
    sp_sys_file_meta_t link = sp_zero;
    sp_must_ok(t, sp_sys_get_link_metadata_s(kept.dir, kept.sub, &link));
    sp_expect_ne(t, link.mtime.tv_sec, set_mtime.tv_sec);
  }
  return SP_OK;
}

sp_test_each_fn(sys, set_times, set_test_t, set_tests, run_set);
