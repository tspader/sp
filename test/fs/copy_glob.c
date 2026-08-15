#include "fs.h"

typedef struct {
  const c8* name;
  bool missing_src;
  fs_setup_t src [FS_MAX_SETUP];
  const c8* glob;
  sp_err_t err;
  fs_expected_path_t expect [FS_MAX_PATHS];
} test_t;

static const test_t tests [] = {
  {
    .name = "star",
    .src = {
      { "A" },
      { "B" },
    },
    .glob = "*",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE },
    },
  },
  {
    .name = "exact_name",
    .src = {
      { "A" },
      { "B" },
    },
    .glob = "A",
    .expect = {
      { .path = "A", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "B" },
    },
  },
  {
    .name = "star_suffix",
    .src = {
      { "A.C" },
      { "B.C" },
      { "D.E" },
    },
    .glob = "*.C",
    .expect = {
      { .path = "A.C", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "B.C", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "D.E" },
    },
  },
  {
    .name = "star_prefix",
    .src = {
      { "AB" },
      { "AC" },
      { "D" },
    },
    .glob = "A*",
    .expect = {
      { .path = "AB", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "AC", .exists = true, .kind = SP_FS_KIND_FILE },
      { .path = "D" },
    },
  },
  {
    .name = "no_match",
    .src = {
      { "A" },
      { "B" },
    },
    .glob = "C",
    .expect = {
      { .path = "A" },
      { .path = "B" },
    },
  },
  {
    .name = "empty_src",
    .glob = "*",
  },
  {
    .name = "missing_src",
    .missing_src = true,
    .glob = "*",
    .err = SP_ERR_SYS_NOT_FOUND,
  },
};

sp_test_each(fs, copy_glob, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t sandbox = sp_test_dir(t);
  sp_str_t src = sp_fs_join_path(mem, sandbox, sp_str_lit("src"));
  sp_str_t dst = sp_fs_join_path(mem, sandbox, sp_str_lit("dst"));
  if (!it->missing_src) {
    sp_fs_create_dir(src);
    fs_apply_setup(t, src, it->src);
  }

  sp_expect_err_eq(t, sp_fs_copy_glob(src, sp_str_view(it->glob), dst), it->err);

  // the destination is created by copy_glob itself, and only after the
  // source has been read successfully
  sp_expect_eq(t, sp_fs_is_dir(dst), it->err == SP_OK);
  fs_expect_paths(t, dst, it->expect);
  return SP_OK;
}
