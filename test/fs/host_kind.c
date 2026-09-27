#include "sp.h"
#include "sp/sp_test.h"

typedef enum {
  FN_IS_SEP,
  FN_IS_ROOT,
  FN_IS_ABSOLUTE,
  FN_TRIM_PATH,
  FN_GET_NAME,
  FN_PARENT_PATH,
  FN_GET_EXT,
  FN_GET_STEM,
  FN_REPLACE_EXT,
  FN_JOIN_PATH,
  FN_NORMALIZE_PATH,
} fn_t;

typedef struct {
  const c8* name;
  fn_t fn;
  const c8* input;
} test_t;

static const test_t tests [] = {
  { .name = "is_sep",         .fn = FN_IS_SEP,         .input = "\\" },
  { .name = "is_root",        .fn = FN_IS_ROOT,        .input = "C:\\" },
  { .name = "is_absolute",    .fn = FN_IS_ABSOLUTE,    .input = "\\A" },
  { .name = "trim_path",      .fn = FN_TRIM_PATH,      .input = "A\\" },
  { .name = "get_name",       .fn = FN_GET_NAME,       .input = "A\\B" },
  { .name = "parent_path",    .fn = FN_PARENT_PATH,    .input = "A\\B" },
  { .name = "get_ext",        .fn = FN_GET_EXT,        .input = "A.B\\C" },
  { .name = "get_stem",       .fn = FN_GET_STEM,       .input = "A.B\\C" },
  { .name = "replace_ext",    .fn = FN_REPLACE_EXT,    .input = "A.B\\C" },
  { .name = "join_path",      .fn = FN_JOIN_PATH,      .input = "A\\" },
  { .name = "normalize_path", .fn = FN_NORMALIZE_PATH, .input = "A\\B" },
};

sp_test_each(fs, host_kind, test_t, tests) {
  sp_mem_t mem = sp_test_arena(t);
  sp_fs_path_kind_t kind = sp_os_get_path_kind();
  sp_str_t path = sp_cstr_as_str(it->input);
  sp_str_t other = sp_str_lit("D");

  switch (it->fn) {
    case FN_IS_SEP:         sp_expect_eq(t, sp_fs_is_sep(path.data[0]), sp_fs_is_sep_for(path.data[0], kind)); break;
    case FN_IS_ROOT:        sp_expect_eq(t, sp_fs_is_root(path), sp_fs_is_root_for(path, kind)); break;
    case FN_IS_ABSOLUTE:    sp_expect_eq(t, sp_fs_is_absolute(path), sp_fs_is_absolute_for(path, kind)); break;
    case FN_TRIM_PATH:      sp_expect_str_eq(t, sp_fs_trim_path(path), sp_fs_trim_path_for(path, kind)); break;
    case FN_GET_NAME:       sp_expect_str_eq(t, sp_fs_get_name(path), sp_fs_get_name_for(path, kind)); break;
    case FN_PARENT_PATH:    sp_expect_str_eq(t, sp_fs_parent_path(path), sp_fs_parent_path_for(path, kind)); break;
    case FN_GET_EXT:        sp_expect_str_eq(t, sp_fs_get_ext(path), sp_fs_get_ext_for(path, kind)); break;
    case FN_GET_STEM:       sp_expect_str_eq(t, sp_fs_get_stem(path), sp_fs_get_stem_for(path, kind)); break;
    case FN_REPLACE_EXT:    sp_expect_str_eq(t, sp_fs_replace_ext(mem, path, other), sp_fs_replace_ext_for(mem, path, other, kind)); break;
    case FN_JOIN_PATH:      sp_expect_str_eq(t, sp_fs_join_path(mem, path, other), sp_fs_join_path_for(mem, path, other, kind)); break;
    case FN_NORMALIZE_PATH: sp_expect_str_eq(t, sp_fs_normalize_path(mem, path), sp_fs_normalize_path_for(mem, path, kind)); break;
  }
  return SP_OK;
}
