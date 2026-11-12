#include "sp/sp_test.h"

#define ROOT_FD 9000
#define MAX_SUB 64

typedef enum {
  OP_FILE_READER,
  OP_FILE_WRITER,
  OP_GET_PATH_METADATA,
  OP_GET_LINK_METADATA,
  OP_CANONICALIZE_PATH,
  OP_READ_FILE,
  OP_CREATE_DIR,
  OP_CREATE_FILE,
  OP_CREATE_FILE_SLICE,
  OP_CREATE_FILE_STR,
  OP_CREATE_FILE_CSTR,
  OP_CREATE_HARD_LINK,
  OP_CREATE_SYM_LINK,
  OP_REMOVE_FILE,
  OP_REMOVE_DIR,
  OP_ATOMIC_OPEN,
  OP_ATOMIC_OPEN_STAGED,
  OP_WRITE_ATOMIC_SLICE_STAGED,
  OP_WRITE_ATOMIC_SLICE,
  OP_WRITE_ATOMIC,
  OP_WRITE_ATOMIC_CSTR,
} op_t;

typedef struct {
  const c8* sub;
  const c8* alias;
} expect_t;

typedef struct {
  const c8* name;
  op_t op;
  expect_t expect;
} test_t;

static const test_t tests [] = {
  { .name = "file_reader_from_path", .op = OP_FILE_READER, .expect = { .sub = "F" } },
  { .name = "file_writer_from_path", .op = OP_FILE_WRITER, .expect = { .sub = "F" } },
  { .name = "get_path_metadata", .op = OP_GET_PATH_METADATA, .expect = { .sub = "F" } },
  { .name = "get_link_metadata", .op = OP_GET_LINK_METADATA, .expect = { .sub = "F" } },
  { .name = "canonicalize_path", .op = OP_CANONICALIZE_PATH, .expect = { .sub = "F" } },
  { .name = "read_file", .op = OP_READ_FILE, .expect = { .sub = "F" } },
  { .name = "create_dir", .op = OP_CREATE_DIR, .expect = { .sub = "F" } },
  { .name = "create_file", .op = OP_CREATE_FILE, .expect = { .sub = "F" } },
  { .name = "create_file_slice", .op = OP_CREATE_FILE_SLICE, .expect = { .sub = "F" } },
  { .name = "create_file_str", .op = OP_CREATE_FILE_STR, .expect = { .sub = "F" } },
  { .name = "create_file_cstr", .op = OP_CREATE_FILE_CSTR, .expect = { .sub = "F" } },
  { .name = "create_hard_link", .op = OP_CREATE_HARD_LINK, .expect = { .sub = "F", .alias = "G" } },
  { .name = "create_sym_link", .op = OP_CREATE_SYM_LINK, .expect = { .sub = "F" } },
  { .name = "remove_file", .op = OP_REMOVE_FILE, .expect = { .sub = "F" } },
  { .name = "remove_dir", .op = OP_REMOVE_DIR, .expect = { .sub = "F" } },
  { .name = "atomic_open", .op = OP_ATOMIC_OPEN },
  { .name = "atomic_open_staged", .op = OP_ATOMIC_OPEN_STAGED },
  { .name = "write_atomic_slice_staged", .op = OP_WRITE_ATOMIC_SLICE_STAGED },
  { .name = "write_atomic_slice", .op = OP_WRITE_ATOMIC_SLICE },
  { .name = "write_atomic", .op = OP_WRITE_ATOMIC },
  { .name = "write_atomic_cstr", .op = OP_WRITE_ATOMIC_CSTR },
};

typedef struct {
  bool seen;
  sp_sys_fd_t dir;
  c8 sub [MAX_SUB];
  sp_sys_fd_t alias_dir;
  c8 alias [MAX_SUB];
} probe_t;

static probe_t probe;

static sp_sys_dir_t get_root(s32 it) {
  if (it < 0 || it > 1) return sp_sys_dir(SP_SYS_INVALID_FD);
  return sp_sys_dir((sp_sys_fd_t)(ROOT_FD + it));
}

static sp_err_t get_root_label(s32 it, c8* buf, u64 size, u64* len) {
  *len = 0;
  if (it < 0 || it > 1) return SP_ERR_SYS_BAD_FD;

  sp_str_t label = it ? sp_str_lit("/A") : sp_str_lit("/B");
  if (label.len >= size) return SP_ERR_SYS_NAME_TOO_LONG;
  sp_cstr_copy_to_n(label.data, label.len, buf, size);
  *len = label.len;
  return SP_OK;
}

static sp_err_t record(sp_sys_dir_t dir, const c8* path, u32 len) {
  if (!probe.seen) {
    probe.seen = true;
    probe.dir = dir.fd;
    sp_cstr_copy_to_n(path, len, probe.sub, MAX_SUB);
  }
  return SP_ERR_SYS_ACCESS_DENIED;
}

static sp_err_t probe_open(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_open_mode_t mode, u32 flags, sp_sys_fd_t* out) {
  *out = SP_SYS_INVALID_FD;
  return record(dir, path, len);
}

static sp_err_t probe_mkdir(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_perms_t perms) {
  return record(dir, path, len);
}

static sp_err_t probe_unlink(sp_sys_dir_t dir, const c8* path, u32 len) {
  return record(dir, path, len);
}

static sp_err_t probe_link(sp_sys_dir_t from_dir, const c8* existing, u32 existing_len, sp_sys_dir_t to_dir, const c8* alias, u32 alias_len) {
  probe.alias_dir = to_dir.fd;
  sp_cstr_copy_to_n(alias, alias_len, probe.alias, MAX_SUB);
  return record(from_dir, existing, existing_len);
}

static sp_err_t probe_symlink(const c8* existing, u32 existing_len, sp_sys_dir_t to_dir, const c8* alias, u32 alias_len, sp_fs_kind_t kind) {
  return record(to_dir, alias, alias_len);
}

static sp_err_t probe_metadata(sp_sys_dir_t dir, const c8* path, u32 len, sp_sys_file_meta_t* st) {
  return record(dir, path, len);
}

static sp_err_t run(sp_test_t* t, test_t* c) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t path = sp_str_lit("/A/F");
  sp_str_t other = sp_str_lit("/A/G");
  sp_mem_slice_t slice = sp_mem_slice((u8*)"A", 1);

  sp_sys_vtable_t vt = sp_sys_vtable_platform;
  vt.get_root = get_root;
  vt.get_root_label = get_root_label;
  vt.open = probe_open;
  vt.mkdir = probe_mkdir;
  vt.unlink = probe_unlink;
  vt.link = probe_link;
  vt.symlink = probe_symlink;
  vt.get_path_metadata = probe_metadata;
  vt.get_link_metadata = probe_metadata;
  probe = sp_zero_s(probe_t);

  sp_io_file_reader_t reader = sp_zero;
  sp_io_file_writer_t writer = sp_zero;
  sp_fs_atomic_t af = sp_zero;
  sp_sys_file_meta_t meta = sp_zero;
  sp_str_t str = sp_zero;

  const sp_sys_vtable_t* saved = sp_sys_set_vtable(&vt);
  switch (c->op) {
    case OP_FILE_READER:               sp_io_file_reader_from_path(&reader, path); break;
    case OP_FILE_WRITER:               sp_io_file_writer_from_path(&writer, path); break;
    case OP_GET_PATH_METADATA:         sp_fs_get_path_metadata(path, &meta); break;
    case OP_GET_LINK_METADATA:         sp_fs_get_link_metadata(path, &meta); break;
    case OP_CANONICALIZE_PATH:         sp_fs_canonicalize_path(mem, path, &str); break;
    case OP_READ_FILE:                 sp_io_read_file(mem, path, &str); break;
    case OP_CREATE_DIR:                sp_fs_create_dir(path); break;
    case OP_CREATE_FILE:               sp_fs_create_file(path); break;
    case OP_CREATE_FILE_SLICE:         sp_fs_create_file_slice(path, slice); break;
    case OP_CREATE_FILE_STR:           sp_fs_create_file_str(path, sp_str_lit("A")); break;
    case OP_CREATE_FILE_CSTR:          sp_fs_create_file_cstr(path, "A"); break;
    case OP_CREATE_HARD_LINK:          sp_fs_create_hard_link(path, other); break;
    case OP_CREATE_SYM_LINK:           sp_fs_create_sym_link(sp_str_lit("A"), path, SP_FS_KIND_FILE); break;
    case OP_REMOVE_FILE:               sp_fs_remove_file(path); break;
    case OP_REMOVE_DIR:                sp_fs_remove_dir(path); break;
    case OP_ATOMIC_OPEN:               sp_fs_atomic_open(&af, path); break;
    case OP_ATOMIC_OPEN_STAGED:        sp_fs_atomic_open_staged(&af, path, sp_str_lit("S")); break;
    case OP_WRITE_ATOMIC_SLICE_STAGED: sp_fs_write_atomic_slice_staged(path, sp_str_lit("S"), slice); break;
    case OP_WRITE_ATOMIC_SLICE:        sp_fs_write_atomic_slice(path, slice); break;
    case OP_WRITE_ATOMIC:              sp_fs_write_atomic(path, sp_str_lit("A")); break;
    case OP_WRITE_ATOMIC_CSTR:         sp_fs_write_atomic_cstr(path, "A"); break;
  }
  sp_sys_set_vtable(saved);

  sp_must(t, probe.seen);
  sp_expect_eq(t, probe.dir, (sp_sys_fd_t)(ROOT_FD + 1));
  if (c->expect.sub) sp_expect_str_eq_c(t, sp_cstr_as_str(probe.sub), c->expect.sub);
  if (c->expect.alias) {
    sp_expect_eq(t, probe.alias_dir, (sp_sys_fd_t)(ROOT_FD + 1));
    sp_expect_str_eq_c(t, sp_cstr_as_str(probe.alias), c->expect.alias);
  }
  return SP_OK;
}

sp_test_each_fn(fs, twins_sim, test_t, tests, run, .serial = true);
