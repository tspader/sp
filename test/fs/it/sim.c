#include "sp.h"
#include "sp/sp_test.h"
#include "sim.h"

#define MAX_WALK 4

#define A16 "AAAAAAAAAAAAAAAA"
#define A240 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16 A16
#define A256 A240 A16
#define A2044 A256 A256 A256 A256 A256 A256 A256 A240 "AAAAAAAAAAAA"
#define A2046 A2044 "AA"
#define A2047 A2044 "AAA"
#define A2048 A256 A256 A256 A256 A256 A256 A256 A256

typedef struct {
  const c8* path;
  sp_fs_kind_t kind;
} entry_t;

typedef struct {
  sp_err_t walk;
  sp_err_t enter;
  entry_t entries [MAX_WALK];
  u32 opens;
} expect_t;

typedef struct {
  const c8* name;
  u32 stop_after;
  bool enter_first;
  sim_dir_t dirs [SIM_MAX_DIRS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "enter_before_first_entry_is_invalid",
    .enter_first = true,
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .enter = SP_ERR_SYS_INVALID,
      .entries = { { "B", SP_FS_KIND_FILE } },
      .opens = 1,
    },
  },
  {
    .name = "resumes_parent_after_child",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { "B", SP_FS_KIND_DIR },
        { "B/D", SP_FS_KIND_FILE },
        { "C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "descends_empty_subdir",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B" },
    },
    .expect = {
      .entries = { { "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "subdir_open_error_unwinds",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .walk = SP_ERR_SYS_NOT_FOUND,
      .entries = { { "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "subdir_it_open_error_unwinds",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .it_open = SP_ERR_SYS_IO },
    },
    .expect = {
      .walk = SP_ERR_SYS_IO,
      .entries = { { "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "read_error_unwinds",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .read = SP_ERR_SYS_IO },
    },
    .expect = {
      .walk = SP_ERR_SYS_IO,
      .entries = { { "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "enter_after_error_returns_error",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR }, { "C", SP_FS_KIND_FILE } } },
      { .path = "B", .read = SP_ERR_SYS_IO },
    },
    .expect = {
      .walk = SP_ERR_SYS_IO,
      .enter = SP_ERR_SYS_IO,
      .entries = { { "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "enter_after_end_is_invalid",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B" },
    },
    .expect = {
      .enter = SP_ERR_SYS_INVALID,
      .entries = { { "B", SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "deinit_closes_open_frames",
    .stop_after = 2,
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_DIR } } },
      { .path = "B", .entries = { { "D", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { "B", SP_FS_KIND_DIR },
        { "B/D", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "path_fits_max",
    .dirs = {
      { .entries = { { A2048, SP_FS_KIND_DIR } } },
      { .path = A2048, .entries = { { A2046, SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { A2048, SP_FS_KIND_DIR },
        { A2048 "/" A2046, SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
  {
    .name = "path_too_long",
    .dirs = {
      { .entries = { { A2048, SP_FS_KIND_DIR } } },
      { .path = A2048, .entries = { { A2047, SP_FS_KIND_FILE } } },
    },
    .expect = {
      .walk = SP_ERR_SYS_NAME_TOO_LONG,
      .entries = { { A2048, SP_FS_KIND_DIR } },
      .opens = 2,
    },
  },
  {
    .name = "kind_none_falls_back_to_stat",
    .dirs = {
      { .entries = { { "B", SP_FS_KIND_NONE, .stat = SP_FS_KIND_DIR } } },
      { .path = "B", .entries = { { "C", SP_FS_KIND_FILE } } },
    },
    .expect = {
      .entries = {
        { "B", SP_FS_KIND_DIR },
        { "B/C", SP_FS_KIND_FILE },
      },
      .opens = 2,
    },
  },
};

sp_test_each(fs, it_sim, test_t, tests, .serial = true) {
  sp_mem_t mem = sp_test_arena(t);

  sim_t s = sp_zero;
  sim_begin(&s, it->dirs);

  sp_sys_dir_t root = sp_zero;
  sp_err_t err = sp_fs_open_dir_at(sim_root(), &root);
  sp_expect_ok(t, err);

  u32 produced = 0;
  if (!err) {
    sp_fs_it_t walk = sp_fs_it(mem, root);
    if (it->enter_first) sp_expect_err_eq(t, sp_fs_it_enter(&walk), it->expect.enter);
    sp_fs_it_entry_t entry = sp_zero;
    while (sp_fs_it_next(&walk, &entry)) {
      if (produced < MAX_WALK && it->expect.entries[produced].path) {
        const entry_t* want = &it->expect.entries[produced];
        sp_expect_str_eq_c(t, entry.path, want->path);
        sp_expect_eq(t, (u32)entry.kind, (u32)want->kind);
      }
      else {
        sp_test_fail(t, "walker produced unexpected entry {}", sp_fmt_str(entry.path));
      }
      sp_expect_str_eq(t, entry.name, sp_fs_get_name(entry.path));
      produced++;
      if (produced == it->stop_after) break;
    }
    sp_expect_err_eq(t, walk.err, it->expect.walk);
    if (it->expect.enter && !it->enter_first) sp_expect_err_eq(t, sp_fs_it_enter(&walk), it->expect.enter);
    sp_fs_it_deinit(&walk);
    sp_fs_close_dir(root);
  }

  sim_end(&s);

  u32 expected = 0;
  sp_carr_for(it->expect.entries, e) {
    if (!it->expect.entries[e].path) break;
    expected++;
  }
  sp_expect_eq(t, produced, expected);
  sp_expect_eq(t, s.count.opens, it->expect.opens);
  sp_expect_eq(t, s.count.closes, s.count.its);
  sp_expect_eq(t, s.count.fd_closes, s.count.dirs);
  if (s.count.opens) sp_expect_eq(t, s.count.nofollow, s.count.opens - 1);
  return SP_OK;
}
