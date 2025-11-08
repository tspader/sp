#define SP_APP
#include "sp.h"

#include "utest.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#define SP_FILE_CHANGE_EVENT_ALL (SP_FILE_CHANGE_EVENT_ADDED | SP_FILE_CHANGE_EVENT_MODIFIED | SP_FILE_CHANGE_EVENT_REMOVED)
#define SP_TEST_POLL_ITERATIONS 50
#define sp_for_n(N) for (u32 _i = 0; _i < (N); _i++)

typedef struct sp_test_file_monitor {
  sp_test_file_manager_t file_manager;
  sp_test_env_manager_t env_manager;
  sp_file_monitor_t monitor;
  bool change_detected;
  sp_file_change_event_t last_event;
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

void sp_test_file_monitor_callback(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
  sp_test_file_monitor* fixture = (sp_test_file_monitor*)userdata;
  fixture->change_detected = true;
  fixture->last_event = change->events;
  fixture->last_file_path = change->file_path;
}

UTEST_F(sp_test_file_monitor, init_and_cleanup) {
  sp_file_monitor_init(&ut.monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  EXPECT_NE(ut.monitor.os, SP_NULLPTR);
}

UTEST_F(sp_test_file_monitor, detects_file_creation) {
  sp_file_monitor_init(&ut.monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_ADDED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_os_create_directory(test_dir);
  sp_file_monitor_add_directory(&ut.monitor, test_dir);

  sp_file_monitor_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_str_t test_file = sp_os_join_path(test_dir, sp_str_lit("new_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("test content"),
  });

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_ADDED);
  EXPECT_TRUE(sp_str_equal(ut.last_file_path, test_file));
}

UTEST_F(sp_test_file_monitor, detects_file_modification) {
  sp_file_monitor_init(&ut.monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_MODIFIED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_os_create_directory(test_dir);
  sp_file_monitor_add_directory(&ut.monitor, test_dir);

  sp_str_t test_file = sp_os_join_path(test_dir, sp_str_lit("modify_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("initial content"),
  });

  sp_file_monitor_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("modified content"),
  });

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_MODIFIED);
  EXPECT_TRUE(sp_str_equal(ut.last_file_path, test_file));
}

UTEST_F(sp_test_file_monitor, detects_file_deletion) {
  sp_file_monitor_init(&ut.monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_REMOVED, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_os_create_directory(test_dir);
  sp_file_monitor_add_directory(&ut.monitor, test_dir);

  sp_str_t test_file = sp_os_join_path(test_dir, sp_str_lit("delete_file.txt"));
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("to be deleted"),
  });

  sp_file_monitor_process_changes(&ut.monitor);
  ut.change_detected = false;

  sp_os_remove_file(test_file);

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_REMOVED);
  EXPECT_TRUE(sp_str_equal(ut.last_file_path, test_file));
}

UTEST_F(sp_test_file_monitor, multiple_events_same_file) {
  sp_file_monitor_init(&ut.monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_os_create_directory(test_dir);
  sp_file_monitor_add_directory(&ut.monitor, test_dir);

  sp_str_t test_file = sp_os_join_path(test_dir, sp_str_lit("lifecycle.txt"));

  sp_file_monitor_process_changes(&ut.monitor);
  ut.change_detected = false;

  // Create file - may be reported as ADDED or MODIFIED depending on platform
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("created"),
  });

  bool timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_TRUE(ut.last_event == SP_FILE_CHANGE_EVENT_ADDED || ut.last_event == SP_FILE_CHANGE_EVENT_MODIFIED);

  // Modify file
  ut.change_detected = false;
  sp_test_file_create_ex((sp_test_file_config_t) {
    .path = test_file,
    .content = SP_LIT("modified"),
  });

  timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_MODIFIED);

  // Delete file
  ut.change_detected = false;
  sp_os_remove_file(test_file);

  timed_out = true;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = false;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
  EXPECT_EQ(ut.last_event, SP_FILE_CHANGE_EVENT_REMOVED);
}

UTEST_F(sp_test_file_monitor, no_events_without_changes) {
  sp_file_monitor_init(&ut.monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_ALL, &ut);

  sp_str_t test_dir = sp_test_file_path(&ut.file_manager, sp_str_lit("monitor_test"));
  sp_os_create_directory(test_dir);
  sp_file_monitor_add_directory(&ut.monitor, test_dir);

  sp_file_monitor_process_changes(&ut.monitor);
  ut.change_detected = false;

  bool timed_out = false;
  sp_for_n(SP_TEST_POLL_ITERATIONS) {
    sp_file_monitor_process_changes(&ut.monitor);
    if (ut.change_detected) {
      timed_out = true;
      break;
    }
  }

  EXPECT_FALSE(timed_out);
}

UTEST_MAIN()
