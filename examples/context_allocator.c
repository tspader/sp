#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

int main(void) {
  sp_example_init();

  sp_bump_allocator_t bump_allocator;
  sp_allocator_t bump = sp_bump_allocator_init(&bump_allocator, 1024 * 1024);

  sp_context_push_allocator(bump);
  sp_str_t path = sp_str_from_cstr("/tmp/example/file.txt");
  sp_dyn_array(sp_str_t) parts = sp_str_split_c8(path, '/');

  sp_dyn_array_free(parts);
  sp_context_pop();
  sp_bump_allocator_clear(&bump_allocator);

  sp_free((void*)path.data);

  sp_example_shutdown();
  return 0;
}
