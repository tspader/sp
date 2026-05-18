#include "fs.h"

UTEST_F(fs, iterate_trailing_slash_no_double_separator) {
  SKIP_ON_WASM()
  sp_str_t sandbox = ut.file_manager.paths.test;

  fs_setup_t setup [] = {
    { "iter_slash/a.txt",       FS_SETUP_FILE },
    { "iter_slash/sub",         FS_SETUP_DIR },
    { "iter_slash/sub/b.txt",   FS_SETUP_FILE },
    sp_zero,
  };
  fs_apply_setup(&ur, &ut.file_manager, sandbox, setup);

  sp_str_t base = sp_fs_join_path(ut.file_manager.mem, sandbox, sp_str_lit("iter_slash"));
  sp_str_t base_slash = sp_str_concat(ut.file_manager.mem, base, sp_str_lit("/"));

  sp_da(sp_fs_entry_t) entries = sp_fs_collect_recursive(ut.file_manager.mem, base_slash);

  EXPECT_EQ(sp_da_size(entries), 3u);
  sp_da_for(entries, i) {
    EXPECT_FALSE(sp_str_contains(entries[i].path, sp_str_lit("//")));
  }
}

UTEST_F(fs, iterate_name_matches_path_tail) {
  SKIP_ON_WASM()
  sp_str_t sandbox = ut.file_manager.paths.test;

  fs_setup_t setup [] = {
    { "iter_tail/alpha.txt", FS_SETUP_FILE },
    { "iter_tail/beta",      FS_SETUP_DIR },
    sp_zero,
  };
  fs_apply_setup(&ur, &ut.file_manager, sandbox, setup);

  sp_str_t base = sp_fs_join_path(ut.file_manager.mem, sandbox, sp_str_lit("iter_tail"));
  sp_da(sp_fs_entry_t) entries = sp_fs_collect(ut.file_manager.mem, base);

  EXPECT_EQ(sp_da_size(entries), 2u);
  sp_da_for(entries, i) {
    sp_fs_entry_t e = entries[i];
    sp_str_t tail = sp_str_sub(e.path, e.path.len - e.name.len, e.name.len);
    EXPECT_TRUE(sp_str_equal(tail, e.name));
    EXPECT_TRUE(sp_str_equal(e.path, sp_fs_join_path(ut.file_manager.mem, base, e.name)));
  }
}
