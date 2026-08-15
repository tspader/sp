#include "fs.h"

typedef enum {
  OP_COPY_FILE,
  OP_COPY,
  OP_LINK,
} op_t;

typedef struct {
  const c8* name;
  fs_setup_t setup [FS_MAX_SETUP];
  op_t op;
  const c8* src;
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
    .name = "file_via_link",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .op = OP_LINK,
    .src = "A",
    .dst = "B",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
  {
    .name = "file_through_symlink",
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
    .src = "A",
    .dst = "B",
    .err = SP_ERR_SYS_NOT_FOUND,
    .expect = {
      { .path = "B" },
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
#if defined(SP_POSIX)
  {
    .name = "file_source_is_fifo",
    .setup = {
      { .path = "F", .kind = FS_SETUP_FIFO },
    },
    .src = "F",
    .dst = "B",
    .err = SP_ERR_SYS_UNSUPPORTED,
    .expect = {
      { .path = "B" },
    },
  },
  {
    .name = "source_is_fifo",
    .setup = {
      { .path = "F", .kind = FS_SETUP_FIFO },
    },
    .op = OP_COPY,
    .src = "F",
    .dst = "B",
    .err = SP_ERR_SYS_UNSUPPORTED,
    .expect = {
      { .path = "B" },
    },
  },
#endif
  {
    .name = "dir_basic",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
      { .path = "A/C", .content = "C" },
      { "D", FS_SETUP_DIR },
    },
    .op = OP_COPY,
    .src = "A",
    .dst = "D",
    .expect = {
      { .path = "D/A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "D/A/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      { .path = "D/A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
    },
  },
  {
    .name = "dir_nested",
    .setup = {
      { "A", FS_SETUP_DIR },
      { "A/B", FS_SETUP_DIR },
      { .path = "A/C", .content = "C" },
      { .path = "A/B/D", .content = "D" },
      { "E", FS_SETUP_DIR },
    },
    .op = OP_COPY,
    .src = "A",
    .dst = "E",
    .expect = {
      { .path = "E/A", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "E/A/C", .exists = true, .kind = SP_FS_KIND_FILE, .content = "C" },
      { .path = "E/A/B", .exists = true, .kind = SP_FS_KIND_DIR },
      { .path = "E/A/B/D", .exists = true, .kind = SP_FS_KIND_FILE, .content = "D" },
    },
  },
  {
    .name = "dir_with_nonalphanumeric",
    .setup = {
      { "A.B", FS_SETUP_DIR },
      { "C", FS_SETUP_DIR },
    },
    .op = OP_COPY,
    .src = "A.B",
    .dst = "C",
    .expect = {
      { .path = "C/A.B", .exists = true, .kind = SP_FS_KIND_DIR },
    },
  },
  {
    .name = "unicode_file",
    .setup = {
      { .path = "\xc3\xb6riginal.txt", .content = "A" },
    },
    .src = "\xc3\xb6riginal.txt",
    .dst = "\xc3\xbc\x63opy.txt",
    .expect = {
      { .path = "\xc3\xb6riginal.txt", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      { .path = "\xc3\xbc\x63opy.txt", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
    },
  },
};

sp_test_each(fs, copy, test_t, tests) {
  skip_if_symlinks_needed(t, it->setup);

  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  fs_apply_setup(t, sandbox, it->setup);

  sp_str_t src = sp_fs_join_path(mem, sandbox, sp_str_view(it->src));
  sp_str_t dst = sp_fs_join_path(mem, sandbox, sp_str_view(it->dst));

  sp_err_t result = SP_OK;
  switch (it->op) {
    case OP_COPY_FILE: result = sp_fs_copy_file(src, dst); break;
    case OP_COPY:      result = sp_fs_copy(src, dst); break;
    case OP_LINK:      result = sp_fs_link(src, dst, SP_FS_LINK_COPY); break;
  }
  sp_expect_err_eq(t, result, it->err);

  fs_expect_paths(t, sandbox, it->expect);
  return SP_OK;
}

#if defined(SP_POSIX)
sp_test(fs, copy_preserves_file_attributes) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t source = fs_path(t, sp_str_lit("A"));
  sp_fs_create_file_str(source, sp_str_lit("A"));

  sp_must_eq(t, chmod(sp_cstr_from_str(mem, source), 0755), 0);

  struct stat original_stat = sp_zero;
  sp_must_eq(t, stat(sp_cstr_from_str(mem, source), &original_stat), 0);

  sp_str_t copy = fs_path(t, sp_str_lit("B"));
  sp_must_ok(t, sp_fs_copy(source, copy));
  sp_must(t, sp_fs_is_file(copy));

  struct stat copy_stat = sp_zero;
  sp_must_eq(t, stat(sp_cstr_from_str(mem, copy), &copy_stat), 0);

  sp_must_eq(t, original_stat.st_mode, copy_stat.st_mode);
  sp_must_eq(t, original_stat.st_size, copy_stat.st_size);
  sp_str_t preserved = sp_zero;
  sp_io_read_file(mem, copy, &preserved);
  sp_expect_str_eq(t, preserved, sp_str_lit("A"));
  return SP_OK;
}
#endif
