#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

typedef struct {
  u32 count;
  f32* values;
  sp_mutex_t mutex;
} complex_data_t;

int main(void) {
  sp_example_init();

  sp_file_monitor_t monitor = SP_ZERO_INITIALIZE();
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_ring_buffer_iterator_t it = SP_ZERO_INITIALIZE();
  complex_data_t data = SP_ZERO_INITIALIZE();

  SP_UNUSED(monitor);
  SP_UNUSED(builder);
  SP_UNUSED(it);
  SP_UNUSED(data);

  sp_example_shutdown();
  return 0;
}
