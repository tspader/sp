#define SP_IMPLEMENTATION
#include "../sp.h"

typedef enum {
  EVENT_INIT,
  EVENT_UPDATE,
  EVENT_DRAW,
  EVENT_QUIT
} event_type_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

const c8* event_type_to_cstr(event_type_t type) {
  switch (type) {
    SP_SWITCH_ENUM_TO_CSTR(EVENT_INIT);
    SP_SWITCH_ENUM_TO_CSTR(EVENT_UPDATE);
    SP_SWITCH_ENUM_TO_CSTR(EVENT_DRAW);
    SP_SWITCH_ENUM_TO_CSTR(EVENT_QUIT);
    default: return "UNKNOWN";
  }
}

sp_str_t event_type_to_str(event_type_t type) {
  return SP_CSTR(event_type_to_cstr(type));
}

const c8* event_type_to_lower(event_type_t type) {
  switch (type) {
    case EVENT_INIT:   return "init";
    case EVENT_UPDATE: return "update";
    case EVENT_DRAW:   return "draw";
    case EVENT_QUIT:   return "quit";
    default: return "unknown";
  }
}

void log_event(event_type_t type) {
  SP_LOG("Processing event: {}", SP_FMT_CSTR(event_type_to_cstr(type)));
}

int main(void) {
  sp_example_init();

  log_event(EVENT_INIT);
  log_event(EVENT_QUIT);

  sp_example_shutdown();
  return 0;
}
