#include "harness.h"

static const sys_case_t tests [] = {
  {
    .name = "replaces_existing_target",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B" } },
    },
    .expect = {
      { .path = "A" },
      { .path = "B", .exists = true, .content = "A" },
    },
  },
  {
    .name = "replaces_read_only_target",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .kind = SYS_SETUP_READ_ONLY_FILE, .content = "B" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B" } },
    },
    .expect = {
      { .path = "A" },
      { .path = "B", .exists = true, .content = "A" },
    },
  },
  {
    .name = "refuses_missing_source",
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B", .err = SP_ERR_SYS_NOT_FOUND } },
    },
    .expect = {
      { .path = "B" },
    },
  },
  {
    .name = "renames_directory",
    .setup = {
      { .path = "D", .kind = SYS_SETUP_DIR },
      { .path = "D/A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "D", .to = "E" } },
    },
    .expect = {
      { .path = "D" },
      { .path = "E/A", .exists = true, .content = "A" },
    },
  },
  {
    .name = "honors_dirfds",
    .setup = {
      { .path = "D", .kind = SYS_SETUP_DIR },
      { .path = "E", .kind = SYS_SETUP_DIR },
      { .path = "D/A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .slot = 0, .path = "D" } },
      { .kind = SYS_STEP_OPEN_DIR, .open_dir = { .slot = 1, .path = "E" } },
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .from_dir = SYS_DIR_SLOT_0, .to = "B", .to_dir = SYS_DIR_SLOT_1 } },
    },
    .expect = {
      { .path = "D/A" },
      { .path = "E/B", .exists = true, .content = "A" },
    },
  },
#if defined(SP_WASM)
  {
    .name = "no_replace_unsupported",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B", .flags = SP_SYS_RENAME_NO_REPLACE, .err = SP_ERR_SYS_UNSUPPORTED } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
      { .path = "B" },
    },
  },
#else
  {
    .name = "no_replace_moves_to_free_name",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B", .flags = SP_SYS_RENAME_NO_REPLACE } },
    },
    .expect = {
      { .path = "A" },
      { .path = "B", .exists = true, .content = "A" },
    },
  },
  {
    .name = "no_replace_refuses_file",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .content = "B" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B", .flags = SP_SYS_RENAME_NO_REPLACE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
      { .path = "B", .exists = true, .content = "B" },
    },
  },
  {
    .name = "no_replace_refuses_directory",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "B", .kind = SYS_SETUP_DIR },
      { .path = "B/C", .content = "C" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B", .flags = SP_SYS_RENAME_NO_REPLACE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
      { .path = "B/C", .exists = true, .content = "C" },
    },
  },
  {
    .name = "no_replace_refuses_symlink",
    .setup = {
      { .path = "A", .content = "A" },
      { .path = "T", .content = "T" },
      { .path = "B", .kind = SYS_SETUP_SYMLINK, .target = "T" },
    },
    .steps = {
      { .kind = SYS_STEP_RENAME, .rename = { .from = "A", .to = "B", .flags = SP_SYS_RENAME_NO_REPLACE, .err = SP_ERR_SYS_EXISTS } },
    },
    .expect = {
      { .path = "A", .exists = true, .content = "A" },
      { .path = "B", .exists = true, .content = "T" },
    },
  },
#endif
};

sp_test_each_fn(sys, rename, sys_case_t, tests, sys_case_run);
