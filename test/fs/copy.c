#include "fs.h"

typedef enum {
  OP_COPY_FILE,
  OP_COPY_TREE,
  OP_COPY,
} op_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  op_t op;
  sp_fs_atomic_mode_t mode;
  const c8* src;
  const c8* dst_dir;
  const c8* dst;
  sp_err_t err;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "file_basic",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_empty",
    .setup = {
      { .path = "A" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "" },
    },
  },
  {
    .name = "file_replaces_existing",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_missing_parent",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "D/B",
    .err = SP_ERR_SYS_NOT_FOUND,
    .expect = {
      { .path = "D" },
    },
  },
  {
    .name = "file_across_dirs",
    .setup = {
      { .path = "A", .content = "A" },
      { "D", FS_SETUP_DIR },
    },
    .src = "A",
    .dst_dir = "D",
    .dst = "B",
    .expect = {
      { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_onto_itself",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_exclusive_new",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .mode = SP_FS_ATOMIC_EXCLUSIVE,
    .src = "A",
    .dst = "B",
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_exclusive_existing",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .mode = SP_FS_ATOMIC_EXCLUSIVE,
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "file_exclusive_dest_is_dir",
    .setup = {
      { .path = "A", .content = "A" },
      { "B", FS_SETUP_DIR },
    },
    .mode = SP_FS_ATOMIC_EXCLUSIVE,
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_DIR },
    },
  },
  {
    .name = "file_dest_parent_is_symlink",
    .setup = {
      { .path = "A", .content = "A" },
      { "D", FS_SETUP_DIR },
      { .path = "L", .kind = FS_SETUP_DIR_SYMLINK, .target = "D" },
    },
    .src = "A",
    .dst = "L/B",
    .expect = {
      { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_dest_is_symlink",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "T", .content = "T" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "T" },
    },
    .src = "A",
    .dst = "L",
    .expect = {
      { .path = "L", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      { .path = "T", .exists = true, .kind = SP_FS_KIND_FILE, .content = "T" },
    },
  },
  {
    .name = "file_source_symlink_followed",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "L", .kind = FS_SETUP_SYMLINK, .target = "A" },
    },
    .src = "L",
    .dst = "B",
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_source_missing",
    .setup = {
      { .path = "B", .content = "B" },
    },
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_NOT_FOUND,
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "file_source_is_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_IS_DIR,
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "B" },
    },
  },
  {
    .name = "file_dest_parent_is_file",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "P", .content = "P" },
    },
    .src = "A",
    .dst = "P/B",
    // a file in the parent chain: POSIX reports ENOTDIR, NT reports PATH_NOT_FOUND
#if defined(SP_WIN32)
    .err = SP_ERR_SYS_NOT_FOUND,
#else
    .err = SP_ERR_SYS_NOT_DIR,
#endif
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
    },
  },
  {
    .name = "file_dest_is_dir",
    .setup = {
      { .path = "A", .content = "A" },
      { "B", FS_SETUP_DIR },
    },
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_IS_DIR,
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_DIR },
    },
  },
  {
    .name = "tree_nested",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/B/D", .content = "D" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "E/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      { .path = "E/B", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "E/B/D", .exists = true, .kind = SP_FS_KIND_FILE, .content = "D" },
    },
  },
  {
    .name = "tree_empty",
    .setup = {
      { "A", FS_SETUP_DIR },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E", .exists = true, .kind = SP_FS_KIND_DIR },
    },
  },
  {
    .name = "tree_empty_subdir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E/B", .exists = true, .kind = SP_FS_KIND_DIR },
    },
  },
  {
    .name = "tree_creates_parents",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "D/E",
    .expect = {
      { .path = "D/E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "tree_across_dirs",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { "D", FS_SETUP_DIR },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst_dir = "D",
    .dst = "E",
    .expect = {
      { .path = "D/E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "tree_into_open_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { "D", FS_SETUP_DIR },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst_dir = "D",
    .dst = ".",
    .expect = {
      { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "tree_replaces_existing",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { "E", FS_SETUP_DIR },
      { .path = "E/B", .content = "E" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "tree_exclusive_existing",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { "E", FS_SETUP_DIR },
      { .path = "E/B", .content = "E" },
    },
    .op = OP_COPY_TREE,
    .mode = SP_FS_ATOMIC_EXCLUSIVE,
    .src = "A",
    .dst = "E",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "E/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "E" },
    },
  },
  {
    .name = "tree_symlink_preserved",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "C" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
    },
  },
  {
    .name = "tree_symlink_to_dir_preserved",
    .setup = {
      { .path = "A/Z/F", .content = "F" },
      { .path = "A/L", .kind = FS_SETUP_DIR_SYMLINK, .target = "Z" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E/Z/F", .exists = true, .kind = SP_FS_KIND_FILE, .content = "F" },
      { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "Z" },
      { .path = "E/L/F", .exists = true, .kind = SP_FS_KIND_FILE, .content = "F" },
    },
  },
  {
    .name = "tree_symlink_replaced",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "C" },
      { "E", FS_SETUP_DIR },
      { .path = "E/L", .kind = FS_SETUP_DIR_SYMLINK, .target = "../A" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "C" },
    },
  },
  {
    .name = "tree_symlink_exclusive_existing",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/L", .kind = FS_SETUP_SYMLINK, .target = "C" },
      { .path = "E/F", .content = "F" },
      { .path = "E/L", .kind = FS_SETUP_SYMLINK, .target = "F" },
    },
    .op = OP_COPY_TREE,
    .mode = SP_FS_ATOMIC_EXCLUSIVE,
    .src = "A",
    .dst = "E",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "E/L", .exists = true, .kind = SP_FS_KIND_SYMLINK, .target = "F" },
    },
  },
  {
    .name = "tree_onto_itself",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "A",
    .expect = {
      { .path = "A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
    },
  },
  {
    .name = "tree_dest_parent_is_file",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { .path = "P", .content = "P" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "P/E",
    .err = SP_ERR_SYS_NOT_DIR,
    .expect = {
      { .path = "P", .exists = true, .kind = SP_FS_KIND_FILE, .content = "P" },
    },
  },
  {
    .name = "tree_dest_is_file",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { .path = "E", .content = "E" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .err = SP_ERR_SYS_EXISTS,
    .expect = {
      { .path = "E", .exists = true, .kind = SP_FS_KIND_FILE, .content = "E" },
    },
  },
  {
    .name = "tree_source_missing",
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .err = SP_ERR_SYS_NOT_FOUND,
    .expect = {
      { .path = "E" },
    },
  },
  {
    .name = "tree_source_is_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .err = SP_ERR_SYS_NOT_DIR,
    .expect = {
      { .path = "E" },
    },
  },
#if defined(SP_POSIX)
  {
    .name = "tree_fifo_inside",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/F", .kind = FS_SETUP_FIFO },
    },
    .op = OP_COPY_TREE,
    .src = "A",
    .dst = "E",
    .err = SP_ERR_SYS_UNSUPPORTED,
    .expect = {
      { .path = "E/F" },
    },
  },
#endif
  {
    .name = "dispatch_file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .op = OP_COPY,
    .src = "A",
    .dst = "B",
    .expect = {
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "dispatch_dir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
    },
    .op = OP_COPY,
    .src = "A",
    .dst = "D",
    .expect = {
      { .path = "D", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
    },
  },
  {
    .name = "dispatch_missing",
    .op = OP_COPY,
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_NOT_FOUND,
    .expect = {
      { .path = "B" },
    },
  },
};

sp_test_each(fs, copy, test_t, tests) {
  if (fs_setup_needs_symlinks(it->setup)) sp_test_skip_without_symlinks();

  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_sys_fd_t dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_fs_open_dir_at(sandbox, &dir));

  sp_path_t dst_base = it->dst_dir ? sp_path_join(mem, sandbox, sp_cstr_as_str(it->dst_dir)) : sandbox;
  sp_sys_fd_t dst_dir = SP_SYS_INVALID_FD;
  sp_must_ok(t, sp_fs_open_dir_at(dst_base, &dst_dir));

  sp_path_t src = sp_path_at(dir, sp_cstr_as_str(it->src));
  sp_path_t dst = sp_path_at(dst_dir, sp_cstr_as_str(it->dst));

  sp_err_t result = SP_OK;
  switch (it->op) {
    case OP_COPY_FILE: result = sp_fs_copy_file_at(src, dst, it->mode); break;
    case OP_COPY_TREE: result = sp_fs_copy_tree_at(src, dst, it->mode); break;
    case OP_COPY:      result = sp_fs_copy_at(src, dst, it->mode); break;
  }
  sp_sys_close(dst_dir);
  sp_sys_close(dir);
  sp_expect_err_eq(t, result, it->err);

  fs_expect_paths(t, sandbox, it->expect);
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

typedef struct {
  const c8* name;
  u32 size;
} size_test_t;

static const size_test_t sizes [] = {
  { .name = "chunk_minus_one", .size = 4095 },
  { .name = "chunk", .size = 4096 },
  { .name = "chunk_plus_one", .size = 4097 },
  { .name = "large", .size = 1 << 20 },
};

sp_test_each(fs, copy_size, size_test_t, sizes) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));

  u8* data = sp_alloc_n(mem, u8, it->size);
  sp_for(i, it->size) {
    data[i] = (u8)(i * 31 + 7);
  }
  sp_must_ok(t, sp_fs_create_file_slice_at(from, sp_mem_slice(data, it->size)));
  sp_must_ok(t, sp_fs_copy_file_at(from, to, SP_FS_ATOMIC_REPLACE));

  sp_str_t copied = sp_zero;
  sp_must_ok(t, sp_io_read_file_at(mem, to, &copied));
  sp_must_eq(t, copied.len, it->size);
  sp_expect_mem_eq(t, copied.data, data, it->size);
  return SP_OK;
}

#if defined(SP_POSIX)
static sp_sys_file_perms_t mode(u32 value) {
  return (sp_sys_file_perms_t) { .value = value };
}

// postcondition: dest has source's bytes and mode, on a fresh inode; timestamps unspecified
sp_test(fs, copy_file_preserves_mode, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));
  sp_must_ok(t, sp_fs_create_file_str_at(from, sp_str_lit("A")));
  sp_must_ok(t, sp_sys_set_file_perms_s(from.dir, from.sub, mode(0755)));

  sp_sys_file_meta_t from_meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(from.dir, from.sub, &from_meta));

  mode_t mask = umask(077);
  sp_err_t copied_err = sp_fs_copy_file_at(from, to, SP_FS_ATOMIC_REPLACE);
  umask(mask);
  sp_must_ok(t, copied_err);

  sp_sys_file_meta_t to_meta = sp_zero;
  sp_must_ok(t, sp_sys_get_path_metadata_s(to.dir, to.sub, &to_meta));
  sp_must_eq(t, to_meta.perms.value, from_meta.perms.value);
  sp_must_eq(t, to_meta.size, from_meta.size);
  sp_must(t, to_meta.id != from_meta.id);

  sp_str_t copied = sp_zero;
  sp_io_read_file_at(mem, to, &copied);
  sp_expect_str_eq(t, copied, sp_str_lit("A"));
  return SP_OK;
}

sp_test(fs, copy_file_replaces_read_only_dest) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("B"));
  sp_must_ok(t, sp_fs_create_file_str_at(from, sp_str_lit("A")));
  sp_must_ok(t, sp_fs_create_file_str_at(to, sp_str_lit("B")));
  sp_must_ok(t, sp_sys_set_file_perms_s(to.dir, to.sub, mode(0444)));

  sp_must_ok(t, sp_fs_copy_file_at(from, to, SP_FS_ATOMIC_REPLACE));

  sp_str_t copied = sp_zero;
  sp_io_read_file_at(mem, to, &copied);
  sp_expect_str_eq(t, copied, sp_str_lit("A"));
  return SP_OK;
}

sp_test(fs, copy_file_unwritable_parent_leaves_nothing) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t dir = sp_path_join(mem, sandbox, sp_str_lit("D"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("D/B"));
  sp_must_ok(t, sp_fs_create_file_str_at(from, sp_str_lit("A")));
  sp_must_ok(t, sp_fs_create_dir_at(dir));

  sp_must_ok(t, sp_sys_set_file_perms_s(dir.dir, dir.sub, mode(0555)));
  sp_err_t result = sp_fs_copy_file_at(from, to, SP_FS_ATOMIC_REPLACE);
  sp_must_ok(t, sp_sys_set_file_perms_s(dir.dir, dir.sub, mode(0755)));

  if (!result) return sp_test_skip(t, "directory permissions not enforced");
  sp_expect_err_eq(t, result, SP_ERR_SYS_ACCESS_DENIED);
  sp_expect(t, !sp_fs_exists_at(to));
  fs_expect_no_temps(t, sandbox);
  return SP_OK;
}

sp_test(fs, copy_tree_unreadable_subdir_fails) {
  sp_mem_t mem = sp_test_arena(t);
  sp_path_t sandbox = sp_test_dir(t);
  sp_path_t from = sp_path_join(mem, sandbox, sp_str_lit("A"));
  sp_path_t locked = sp_path_join(mem, sandbox, sp_str_lit("A/B"));
  sp_path_t to = sp_path_join(mem, sandbox, sp_str_lit("E"));
  sp_must_ok(t, sp_fs_create_dir_at(locked));
  sp_must_ok(t, sp_fs_create_file_str_at(sp_path_join(mem, sandbox, sp_str_lit("A/B/C")), sp_str_lit("C")));

  sp_must_ok(t, sp_sys_set_file_perms_s(locked.dir, locked.sub, mode(0)));
  sp_err_t result = sp_fs_copy_tree_at(from, to, SP_FS_ATOMIC_REPLACE);
  sp_must_ok(t, sp_sys_set_file_perms_s(locked.dir, locked.sub, mode(0755)));

  if (!result) return sp_test_skip(t, "directory permissions not enforced");
  sp_expect_err_eq(t, result, SP_ERR_SYS_ACCESS_DENIED);
  sp_expect(t, !sp_fs_exists_at(sp_path_join(mem, sandbox, sp_str_lit("E/B/C"))));
  return SP_OK;
}
#endif
