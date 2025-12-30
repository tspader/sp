#define SP_APP
#include "sp.h"

#include "test.h"

#include "utest.h"


#define SP_FILE_CHANGE_EVENT_ALL (SP_FILE_CHANGE_EVENT_ADDED | SP_FILE_CHANGE_EVENT_MODIFIED | SP_FILE_CHANGE_EVENT_REMOVED)
#define SP_TEST_POLL_ITERATIONS 50
#define SP_TEST_POLL_SLEEP_MS 20
#define sp_for_n(N) for (u32 _i = 0; _i < (N); _i++)

static bool paths_equal(sp_str_t a, sp_str_t b) {
  return sp_str_equal(sp_fs_canonicalize_path(a), sp_fs_canonicalize_path(b));
}

typedef struct sp_test_file_monitor {
  sp_test_file_manager_t file_manager;
  sp_test_env_manager_t env_manager;
  sp_fmon_t monitor;
  bool change_detected;
  sp_fmon_event_kind_t last_event;
  sp_str_t last_file_path;
} sp_test_file_monitor;

UTEST_F_SETUP(sp_test_file_monitor) {
  sp_test_file_manager_init(&ut.file_manager);
  sp_test_env_manager_init(&ut.env_manager);

  ut.change_detected = false;
  ut.last_event = SP_FILE_CHANGE_EVENT_NONE;
  ut.last_file_path = SP_LIT("");
}

UTEST_F_TEARDOWN(sp_test_file_monitor) {
  sp_test_env_manager_cleanup(&ut.env_manager);
  sp_test_file_manager_cleanup(&ut.file_manager);
}

void fmon_callback(sp_fmon_t* monitor, sp_fmon_event_t* change, void* userdata) {
  sp_test_file_monitor* fixture = (sp_test_file_monitor*)userdata;
  fixture->change_detected = true;
  fixture->last_event = change->events;
  fixture->last_file_path = sp_str_copy(change->file_path);
}

UTEST_F(sp_test_file_monitor, init_and_cleanup) {
  sp_fmon_init(&ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  EXPECT_NE(ut.monitor.os, SP_NULLPTR);
}

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
UTEST_F(sp_test_file_monitor, detects_file_creation) {
  sp_fmon_init(&ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ADDED, &ut);

  sp_str_t dir = sp_test_file_path(&ut.file_manager, sp_str_lit("watched.dir"));
  sp_fs_create_dir(dir);
  sp_fmon_add_dir(&ut.monitor, dir);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_str_t file = sp_fs_join_path(dir, sp_str_lit("new.file"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = file,
    .content = SP_LIT("spum"),
  });

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_ADDED);
  EXPECT_TRUE(paths_equal(ut.last_file_path, file));
}
#endif

UTEST_F(sp_test_file_monitor, detects_file_modification) {
  sp_fmon_init(&ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_MODIFIED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir(test_dir);

  sp_str_t test_file = sp_fs_join_path(test_dir, sp_str_lit("modify_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("initial content"),
  });

  sp_fmon_add_file(&ut.monitor, test_file);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

#if defined(SP_MACOS)
  sp_io_t s = sp_io_from_file(test_file, SP_IO_MODE_WRITE);
  sp_io_write_str(&s, sp_str_lit("modified content"));
  sp_io_close(&s);

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
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
}

UTEST_F(sp_test_file_monitor, detects_file_deletion) {
  sp_fmon_init(&ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_REMOVED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir(test_dir);

  sp_str_t test_file = sp_fs_join_path(test_dir, sp_str_lit("delete_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("to be deleted"),
  });

  sp_fmon_add_file(&ut.monitor, test_file);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_fs_remove_file(test_file);

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_os_sleep_ms(SP_TEST_POLL_SLEEP_MS);
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE((ut.last_event & SP_FILE_CHANGE_EVENT_REMOVED) != 0);
  EXPECT_TRUE(paths_equal(ut.last_file_path, test_file));
}

#if !defined(SP_MACOS) || defined(SP_FMON_MACOS_USE_FSEVENTS)
UTEST_F(sp_test_file_monitor, multiple_events_same_file) {
  sp_fmon_init(&ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir(test_dir);
  sp_fmon_add_dir(&ut.monitor, test_dir);

  sp_str_t test_file = sp_fs_join_path(test_dir, sp_str_lit("lifecycle.txt"));

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("created"),
  });

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
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
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
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
  sp_fs_remove_file(test_file);

  timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
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

UTEST_F(sp_test_file_monitor, no_events_without_changes) {
  sp_fmon_init(&ut.monitor, fmon_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_fs_create_dir(test_dir);
  sp_fmon_add_dir(&ut.monitor, test_dir);

  sp_fmon_process_changes(&ut.monitor);
  ut.change_detected = false;

  bool spurious_event = false;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_fmon_process_changes(&ut.monitor);
    if (ut.change_detected) {
      spurious_event = true;
      break;
    }
  }

  EXPECT_FALSE(spurious_event);
}

SP_TEST_MAIN()
