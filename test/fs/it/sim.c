#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

#define MAX_WALK 4

#define A16 "AAAAAAAAAAAAAAAA"
#define A240 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16
#define A256 A240 A16
#define A2044 A256 A256 A256 A256 A256 A256 A256 A240 "AAAAAAAAAAAA"
#define A2045 A2044 "A"
#define A2048 A256 A256 A256 A256 A256 A256 A256 A256

typedef struct {
  const c8* path;
  const c8* name;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  sp_err_t open;
  sp_err_t walk;
  entry_t entries [MAX_WALK];
  u32 opens;
} expect_t;

typedef struct {
  const c8* name;
  const c8* root;
  bool recursive;
  u32 stop_after;
  sim_dir_t dirs [SIM_MAX_DIRS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "resumes_parent_after_child",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "T/B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { "T/B", "B", SP_FS_KIND_DIR },
        { "T/B/D", "D", SP_FS_KIND_FILE },
        { "T/C", "C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "descends_empty_subdir",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B" },
    },
    .expect = {
      .entries = { { "T/B", "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "root_is_sep",
    .root = "/",
    .dirs = {
      { .path = "/", .entries = { { "A", SP_FS_KIND_DIR } } },
    },
    .expect = {
      .entries = { { "/A", "A", SP_FS_KIND_DIR } },
      .opens = 1,
    },
  },
  {
    .name = "subdir_open_error_unwinds",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .walk = SP_ERR_SYS_NOT_FOUND,
      .opens = 2,
    },
  },
  {
    .name = "read_error_unwinds",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "T/B", .read = SP_ERR_SYS_IO },
    },
    .expect = {
      .walk = SP_ERR_SYS_IO,
      .entries = { { "T/B", "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "deinit_closes_open_frames",
    .root = "T",
    .recursive = true,
    .stop_after = 1,
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "T/B" },
    },
    .expect = {
      .entries = { { "T/B", "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "path_fits_max",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { A2048, SP_FS_KIND_DIR } } },
      { .path = "T/" A2048, .entries = { { A2044, SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { "T/" A2048, A2048, SP_FS_KIND_DIR },
        { "T/" A2048 "/" A2044, A2044, SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "path_too_long",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { A2048, SP_FS_KIND_DIR } } },
      { .path = "T/" A2048, .entries = { { A2045, SP_FS_KIND_FILE } } },
    },
    .expect = {
      .walk = SP_ERR_SYS_NAME_TOO_LONG,
      .entries = { { "T/" A2048, A2048, SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "root_too_long",
    .root = "T/" A2048 "/" A2045,
    .expect = {
      .open = SP_ERR_SYS_NAME_TOO_LONG,
      .walk = SP_ERR_SYS_NAME_TOO_LONG,
    },
  },
  {
    .name = "kind_none_falls_back_to_stat",
    .root = "T",
    .recursive = true,
    .dirs = {
      { .path = "T", .entries = { { "B", SP_FS_KIND_NONE, .stat = SP_FS_KIND_DIR } } },
      { .path = "T/B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { "T/B", "B", SP_FS_KIND_DIR },
        { "T/B/C", "C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
};

sp_test_each(fs, it_sim, test_t, tests, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);
  sp_str_t root = sp_cstr_as_str(it->root);

  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  sp_fs_it_t walk = it->recursive ? sp_fs_it_new_recursive(mem, root) : sp_fs_it_new(mem, root);
  sp_expect_err_eq(t, walk.err, it->expect.open);
  sp_expect(t, !walk.entry.path.data);

  u32 produced = 0;
  while (sp_fs_it_next(&walk)) {
    if (produced < MAX_WALK && it->expect.entries[produced].path) {
      const entry_t* want = &it->expect.entries[produced];
      sp_expect_str_eq_c(t, walk.entry.path, want->path);
      sp_expect_str_eq_c(t, walk.entry.name, want->name);
      sp_expect_eq(t, (u32)walk.entry.kind, (u32)want->kind);
    }
    else {
      sp_test_fail(t, "walker produced unexpected entry {}", sp_fmt_str(walk.entry.path));
    }
    produced++;
    if (produced == it->stop_after) break;
  }
  sp_expect_err_eq(t, walk.err, it->expect.walk);
  sp_fs_it_deinit(&walk);

  sim_end(&s);

  u32 expected = 0;
  sp_carr_for(it->expect.entries, e) {
    if (!it->expect.entries[e].path) break;
    expected++;
  }
  sp_expect_eq(t, produced, expected);
  sp_expect_eq(t, s.count.opens, it->expect.opens);
  sp_expect_eq(t, s.count.closes, s.count.dirs);
  return SP_OK;
}
