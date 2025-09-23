#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

void log_error(sp_str_t file, u32 line, sp_str_t msg) {
  sp_log(SP_LIT("{:color red} in {}:{} - {}"),
      SP_FMT_CSTR("ERROR"),
      SP_FMT_STR(file),
      SP_FMT_U32(line),
      SP_FMT_STR(msg));
}

#define LOG_DEBUG(msg) \
    SP_LOG("[DEBUG] {} ({}:{})", \
        SP_FMT_CSTR(msg), \
        SP_FMT_CSTR(__FILE__), \
        SP_FMT_U32(__LINE__))

int main(void) {
  sp_example_init();

  u32 port = 8080;
  SP_LOG("Server started on port {}", SP_FMT_U32(port));

  sp_str_t status = SP_LIT("ready");
  sp_log(SP_LIT("Status: {}"), SP_FMT_STR(status));

  SP_LOG_STR(SP_LIT("Initialization complete"));

  u32 test_count = 12;
  SP_LOG("{:color green} tests passed", SP_FMT_U32(test_count));

  log_error(SP_LIT("main.c"), 42, SP_LIT("failed to initialize"));

  LOG_DEBUG("setup complete");

  sp_example_shutdown();
  return 0;
}
