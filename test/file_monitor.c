#define SP_APP
#include "sp.h"
#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "utest.h"

typedef struct sp_test_file_monitor_data {
  bool change_detected;
  sp_file_change_event_t last_event;
  c8 last_file_path[SP_MAX_PATH_LEN];
} sp_test_file_monitor_data;

sp_str_t sp_test_generate_random_filename() {
  static u32 counter = 0;
  c8* filename = (c8*)sp_alloc(64);
#ifdef _WIN32
  unsigned int rand_val;
  rand_s(&rand_val);
#else
  unsigned int rand_val = counter * 12345 + 67890;  // Simple deterministic value for portability
#endif
  snprintf(filename, 64, "test_file_%u_%u.tmp", rand_val, counter++);
  return sp_str_from_cstr(filename);
}


void sp_test_file_monitor_callback(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
  sp_test_file_monitor_data* data = (sp_test_file_monitor_data*)userdata;
  data->change_detected = true;
  data->last_event = change->events;
  sp_str_copy_to(change->file_path, data->last_file_path, SP_MAX_PATH_LEN);
}

void sp_test_create_file(const c8* filename, const c8* content) {
  FILE* file = fopen(filename, "w");
  if (file) {
  fputs(content, file);
  fclose(file);
  }
}

void sp_test_modify_file(const c8* filename, const c8* new_content) {
  FILE* file = fopen(filename, "w");
  if (file) {
  fputs(new_content, file);
  fclose(file);
  }
}

void sp_test_delete_file(const c8* filename) {
#ifdef _WIN32
  DeleteFileA(filename);
#else
  remove(filename);
#endif
}

#if 0
UTEST(file_monitor, detects_file_modifications) {
  // Create a test file
  c8* test_filename = sp_test_generate_random_filename();
  sp_test_create_file(test_filename, "Initial content");

  // Set up file monitor
  sp_test_file_monitor_data test_data = {};
  sp_file_monitor_t monitor = {};
  sp_file_monitor_init(&monitor, sp_test_file_monitor_callback, SP_FILE_CHANGE_EVENT_MODIFIED, &test_data);

  // Get current directory and add it to monitor
  c8 current_dir[SP_MAX_PATH_LEN] = {};
  GetCurrentDirectoryA(SP_MAX_PATH_LEN, current_dir);
  sp_str_t current_dir_str = SP_CSTR(current_dir);
  sp_file_monitor_add_directory(&monitor, current_dir_str);

  // Process any initial changes
  sp_file_monitor_process_changes(&monitor);
  test_data.change_detected = false;

  // Modify the file
  sp_test_modify_file(test_filename, "Modified content");

  // Wait a bit for the file system to register the change
  Sleep(100);

  // Process changes
  sp_file_monitor_process_changes(&monitor);

  // Check that the change was detected
  ASSERT_TRUE(test_data.change_detected);
  ASSERT_EQ(test_data.last_event, SP_FILE_CHANGE_EVENT_MODIFIED);
  ASSERT_NE(strstr(test_data.last_file_path, test_filename), SP_NULLPTR);

  // Clean up
  sp_test_delete_file(test_filename);
  sp_free(test_filename);
}
#endif

UTEST_MAIN()
