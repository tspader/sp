#define SP_IMPLEMENTATION
#include "../sp.h"

typedef enum {
  KEY_PRESS,
  KEY_RELEASE,
} event_type_t;

typedef struct {
  event_type_t type;
  c8 key;
} event_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void process_event(event_t* event) {
  SP_LOG("event type {}", SP_FMT_U32((u32)event->type));
}

int main(void) {
  sp_example_init();

  sp_ring_buffer_t events;
  sp_ring_buffer_init(&events, 100, sizeof(event_t));

  event_t evt = { .type = KEY_PRESS, .key = 'A' };
  sp_ring_buffer_push_overwrite(&events, &evt);

  sp_ring_buffer_for(events, it) {
    event_t* e = sp_rb_it(it, event_t);
    process_event(e);
  }

  sp_ring_buffer_rfor(events, it) {
    event_t* e = sp_rb_it(it, event_t);
    if (e->type == KEY_PRESS) break;
  }

  sp_ring_buffer_destroy(&events);

  sp_example_shutdown();
  return 0;
}
