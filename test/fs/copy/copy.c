#include "copy.h"

static const copy_any_mode_t tests [] = {
  {
    .name = "file",
    .setup = {
      { .path = "A", .content = "A" },
    },
    .src = "A",
    .dst = "B",
    .expect = {
      .paths = {
        { .path = "B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "A" },
      },
    },
  },
  {
    .name = "dir",
    .setup = {
      { "A", FS_SETUP_DIR },
      { .path = "A/B", .content = "B" },
    },
    .src = "A",
    .dst = "D",
    .expect = {
      .paths = {
        { .path = "D", .exists = true, .kind = SP_FS_KIND_DIR },
        { .path = "D/B", .exists = true, .kind = SP_FS_KIND_FILE, .content = "B" },
      },
    },
  },
  {
    .name = "missing",
    .src = "A",
    .dst = "B",
    .expect = {
      .err = SP_ERR_SYS_NOT_FOUND,
      .paths = {
        { .path = "B" },
      },
    },
  },
};

sp_test_each(fs, copy, copy_any_mode_t, tests) {
  return copy_run(t, COPY_ANY, it->setup, it->mode, it->src, it->dst_dir, it->dst, &it->expect);
}
