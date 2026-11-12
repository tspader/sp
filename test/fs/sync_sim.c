#include "fs.h"

#define SYNC_SIM_BUFFER 16

typedef enum {
  OP_WRITE_ATOMIC,
  OP_COMMIT,
  OP_SYNC_COMMIT,
  OP_COPY_FILE,
} op_t;

typedef struct {
  sp_err_t err;
  u32 syncs;
  u64 synced;
  fs_expected_path_t paths [FS_MAX_PATHS];
} expect_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  op_t op;
  const c8* from;
  const c8* path;
  const c8* content;
  sp_err_t sync;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "write_atomic_syncs_written_data",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .op = OP_WRITE_ATOMIC,
    .path = "A",
    .content = "B",
    .expect = {
      .syncs = 1,
      .synced = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "write_atomic_sync_fails",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .op = OP_WRITE_ATOMIC,
    .path = "A",
    .content = "B",
    .sync = SP_ERR_SYS_IO,
    .expect = {
      .err = SP_ERR_SYS_IO,
      .syncs = 1,
      .synced = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "commit_does_not_sync",
    .op = OP_COMMIT,
    .path = "A",
    .content = "B",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "sync_flushes_writer",
    .op = OP_SYNC_COMMIT,
    .path = "A",
    .content = "B",
    .expect = {
      .syncs = 1,
      .synced = 1,
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "copy_file_does_not_sync",
    .setup = {
      { .path = "B", .content = "B" },
    },
    .op = OP_COPY_FILE,
    .from = "B",
    .path = "A",
    .expect = {
      .paths = {
        { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
};

typedef struct {
  sp_err_t err;
  u32 calls;
  u64 size;
} probe_t;

static probe_t probe;

static sp_err_t sync_probe(sp_sys_fd_t fd) {
  sp_sys_file_meta_t meta = sp_zero;
  sp_sys_get_file_metadata_p(fd, &meta);
  probe.calls++;
  probe.size = (u64)meta.size;
  if (probe.err) return probe.err;
  return sp_sys_sync_p(fd);
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_must_ok(t, fs_apply_setup(t, sandbox, c->setup));

  sp_path_t path = sp_path_join(mem, sandbox, sp_cstr_as_str(c->path));
  sp_str_t content = sp_cstr_as_str(c->content);

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.sync = sync_probe;
  probe = (probe_t) { .err = c->sync };

  sp_err_t err = SP_OK;
  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  switch (c->op) {
    case OP_WRITE_ATOMIC: {
      err = sp_fs_write_atomic_at(path, content);
      break;
    }
    case OP_COMMIT:
    case OP_SYNC_COMMIT: {
      u8 buffer [SYNC_SIM_BUFFER];
      sp_fs_atomic_t af = sp_zero;
      err = sp_fs_atomic_open_at(&af, path);
      if (!err) err = sp_io_writer_set_buffer(sp_fs_atomic_writer(&af), buffer, sizeof(buffer));
      if (!err) err = sp_io_write_str(sp_fs_atomic_writer(&af), content, SP_NULLPTR);
      if (!err && c->op == OP_SYNC_COMMIT) err = sp_fs_atomic_sync(&af);
      if (!err) err = sp_fs_atomic_commit(&af, SP_FS_ATOMIC_REPLACE);
      else      sp_fs_atomic_abort(&af);
      break;
    }
    case OP_COPY_FILE: {
      err = sp_fs_copy_file_at(sp_path_join(mem, sandbox, sp_cstr_as_str(c->from)), path, SP_FS_ATOMIC_REPLACE);
      break;
    }
  }
  sp_sys_set_vtable(saved);

  sp_expect_err_eq(t, err, c->expect.err);
  sp_expect_eq(t, probe.calls, c->expect.syncs);
  sp_expect_eq(t, probe.size, c->expect.synced);
  fs_expect_paths(t, sandbox, c->expect.paths);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test_each_fn(fs, sync_sim, test_t, tests, run, .serial = true);
