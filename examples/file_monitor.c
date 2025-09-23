#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void trigger_rebuild(void) {
  SP_LOG_STR(SP_LIT("triggering rebuild"));
}

void on_file_change(sp_file_monitor_t* monitor, sp_file_change_t* change, void* userdata) {
  SP_UNUSED(monitor);
  SP_UNUSED(userdata);
  if (change->events & SP_FILE_CHANGE_EVENT_MODIFIED) {
    if (sp_str_ends_with(change->file_name, SP_LIT(".c"))) {
      trigger_rebuild();
    }
  }
}

int main(void) {
  sp_example_init();

  void* userdata = NULL;
  bool running = false;

  sp_file_monitor_t monitor;
  sp_file_monitor_init_debounce(&monitor, on_file_change,
      SP_FILE_CHANGE_EVENT_MODIFIED | SP_FILE_CHANGE_EVENT_ADDED,
      userdata, 100);

  sp_file_monitor_add_directory(&monitor, SP_LIT("src"));
  sp_file_monitor_add_directory(&monitor, SP_LIT("include"));

  while (running) {
    sp_file_monitor_process_changes(&monitor);
    sp_os_sleep_ms(50);
  }

  sp_example_shutdown();
  return 0;
}
