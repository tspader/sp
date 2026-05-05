#define SP_APP
#include "sp.h"

#include "test.h"

#include "utest.h"


#define SP_FILE_CHANGE_EVENT_ALL (SP_FILE_CHANGE_EVENT_ADDED | SP_FILE_CHANGE_EVENT_MODIFIED | SP_FILE_CHANGE_EVENT_REMOVED)
#define FMON_POLL_ITERATIONS 10
#define SP_TEST_POLL_SLEEP_MS 20
#define sp_for_n(N) for (u32 _i = 0; _i < (N); _i++)

static bool paths_equal(sp_mem_t mem, sp_str_t a, sp_str_t b) {
  return sp_str_equal(sp_fs_normalize_path_a(mem, a), sp_fs_normalize_path_a(mem, b));
}

typedef struct sp_test_file_monitor {
  sp_test_file_manager_t file_manager;
  sp_fmon_t monitor;
  bool change_detected;
  sp_fmon_event_kind_t last_event;
  sp_str_t last_file_path;
  sp_mem_arena_t* arena;
  sp_mem_t mem;
} sp_test_file_monitor;

UTEST_F_SETUP(sp_test_file_monitor) {
  ut.arena = sp_mem_arena_new();
  ut.mem = sp_mem_arena_as_allocator(ut.arena);
  sp_test_file_manager_init(&ut.file_manager);

  ut.change_detected = false;
  ut.last_event = SP_FILE_CHANGE_EVENT_NONE;
  ut.last_file_path = SP_LIT("");
}

UTEST_F_TEARDOWN(sp_test_file_monitor) {
  sp_test_file_manager_cleanup(&ut.file_manager);
  sp_fmon_deinit(&ut.monitor);
  sp_mem_arena_destroy(ut.arena);
}

typedef struct {
  sp_fmon_event_kind_t events;
  sp_str_t file_path;
} sp_test_fmon_record_t;

#define SP_TEST_FMON_MAX_RECORDS 16

typedef struct {
  sp_test_fmon_record_t records[SP_TEST_FMON_MAX_RECORDS];
  u32 count;
} sp_test_fmon_history_t;

static sp_test_fmon_history_t fmon_history;

void fmon_callback(sp_fmon_t* monitor, sp_fmon_event_t* change, void* userdata) {
  sp_test_file_monitor* fixture = (sp_test_file_monitor*)userdata;
  fixture->change_detected = true;
  fixture->last_event = change->events;
  fixture->last_file_path = sp_str_copy_a(sp_mem_scratch_allocator_a(), change->file_path);

  if (fmon_history.count < SP_TEST_FMON_MAX_RECORDS) {
    sp_test_fmon_record_t* r = &fmon_history.records[fmon_history.count++];
    r->events = change->events;
    r->file_path = sp_str_copy_a(sp_mem_scratch_allocator_a(), change->file_path);
  }
}

UTEST_F(sp_test_file_monitor, init_and_cleanup) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  EXPECT_NE(ut.monitor.os, SP_NULLPTR);
}

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
UTEST_F(sp_test_file_monitor, detects_file_creation) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ADDED, &ut);

  sp_str_t dir = sp_test_file_path(&ut.file_manager, sp_str_lit("watched.dir"));
  sp_fs_create_dir_a(dir);
  sp_fmon_add_dir(&ut.monitor, dir);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_str_t file = sp_fs_join_path_a(ut.mem, dir, sp_str_lit("new.file"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = file,
    .content = SP_LIT("spum"),
  });

  bool timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_ADDED);
  EXPECT_TRUE(paths_equal(ut.mem, ut.last_file_path, file));
}
#endif

UTEST_F(sp_test_file_monitor, detects_file_modification) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_MODIFIED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir_a(test_dir);

  sp_str_t test_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("modify_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("initial content"),
  });

  sp_fmon_add_file(&ut.monitor, test_file);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

#if defined(SP_MACOS)
  sp_io_writer_t s = SP_ZERO_INITIALIZE();
  sp_io_writer_from_file(&s, test_file, SP_IO_WRITE_MODE_OVERWRITE);
  sp_io_write_str(&s, sp_str_lit("modified content"), SP_NULLPTR);
  sp_io_writer_close(&s);

  bool timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_MODIFIED);
  EXPECT_TRUE(paths_equal(ut.last_file_path, test_file));
#endif
}

UTEST_F(sp_test_file_monitor, detects_file_deletion) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_REMOVED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir_a(test_dir);

  sp_str_t test_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("delete_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("to be deleted"),
  });

  sp_fmon_add_file(&ut.monitor, test_file);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_fs_remove_file_a(test_file);

  bool timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE((ut.last_event & SP_FILE_CHANGE_EVENT_REMOVED) != 0);
  EXPECT_TRUE(paths_equal(ut.mem, ut.last_file_path, test_file));
}

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
UTEST_F(sp_test_file_monitor, multiple_events_same_file) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir_a(test_dir);
  sp_fmon_add_dir(&ut.monitor, test_dir);

  sp_str_t test_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("lifecycle.txt"));

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("created"),
  });

  bool timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE((ut.last_event & (SP_FILE_CHANGE_EVENT_ADDED | SP_FILE_CHANGE_EVENT_MODIFIED)) != 0);

  ut.change_detected = false;
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("modified"),
  });

  timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE((ut.last_event & SP_FILE_CHANGE_EVENT_MODIFIED) != 0);

  ut.change_detected = false;
  sp_fs_remove_file_a(test_file);

  timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE((ut.last_event & SP_FILE_CHANGE_EVENT_REMOVED) != 0);
}
#endif

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
UTEST_F(sp_test_file_monitor, event_filtering) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_REMOVED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("filter_test"));
  sp_fs_create_dir_a(test_dir);

  sp_str_t test_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("filter.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("hello"),
  });

  sp_fmon_add_dir(&ut.monitor, test_dir);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  // Modify the file — should NOT fire since we only watch REMOVED
  {
    sp_io_writer_t w = SP_ZERO_INITIALIZE();
    sp_io_writer_from_file(&w, test_file, SP_IO_WRITE_MODE_OVERWRITE);
    sp_io_write_str(&w, sp_str_lit("modified"), SP_NULLPTR);
    sp_io_writer_close(&w);
  }

  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
  }

  EXPECT_FALSE(ut.change_detected);

  // Delete it — should fire
  sp_fs_remove_file_a(test_file);

  bool timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE((ut.last_event & SP_FILE_CHANGE_EVENT_REMOVED) != 0);
}
#endif

UTEST_F(sp_test_file_monitor, add_file_filtering) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("file_filter_test"));
  sp_fs_create_dir_a(test_dir);

  sp_str_t watched_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("watched.txt"));
  sp_str_t ignored_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("ignored.txt"));

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = watched_file,
    .content = SP_LIT("initial"),
  });
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = ignored_file,
    .content = SP_LIT("initial"),
  });

  sp_fmon_add_file(&ut.monitor, watched_file);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  // Modify the ignored file — should NOT fire
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = ignored_file,
    .content = SP_LIT("changed"),
  });

  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
  }

  EXPECT_FALSE(ut.change_detected);

  // Modify the watched file — should fire
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = watched_file,
    .content = SP_LIT("changed"),
  });

  bool timed_out = true;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }


  EXPECT_FALSE(timed_out);
  EXPECT_TRUE(paths_equal(ut.mem, ut.last_file_path, watched_file));
}

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
UTEST_F(sp_test_file_monitor, rename_file) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("rename_test"));
  sp_fs_create_dir_a(test_dir);

  sp_str_t old_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("before.txt"));
  sp_str_t new_file = sp_fs_join_path_a(ut.mem, test_dir, sp_str_lit("after.txt"));

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = old_file,
    .content = SP_LIT("rename me"),
  });

  sp_fmon_add_dir(&ut.monitor, test_dir);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;
  fmon_history.count = 0;

  sp_sys_rename(sp_str_to_cstr_a(sp_mem_scratch_allocator_a(), old_file), sp_str_to_cstr_a(sp_mem_scratch_allocator_a(), new_file));

  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (fmon_history.count >= 2) break;
  }

  bool got_removed = false;
  bool got_added = false;

  for (u32 i = 0; i < fmon_history.count; i++) {
    sp_test_fmon_record_t* r = &fmon_history.records[i];
    if ((r->events & SP_FILE_CHANGE_EVENT_REMOVED) && paths_equal(ut.mem, r->file_path, old_file)) {
      got_removed = true;
    }
    if ((r->events & SP_FILE_CHANGE_EVENT_ADDED) && paths_equal(ut.mem, r->file_path, new_file)) {
      got_added = true;
    }
  }

  EXPECT_TRUE(got_removed);
  EXPECT_TRUE(got_added);
}
#endif

UTEST_F(sp_test_file_monitor, no_events_without_changes) {
  sp_fmon_init_a(ut.mem, &ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir_a(test_dir);
  sp_fmon_add_dir(&ut.monitor, test_dir);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  bool spurious_event = false;
  sp_for_n(FMON_POLL_ITERATIONS) {
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      spurious_event = true;
      break;
    }
  }

  EXPECT_FALSE(spurious_event);
}

SP_TEST_MAIN()
