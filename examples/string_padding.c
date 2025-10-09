#define SP_IMPLEMENTATION
#include "../sp.h"

typedef struct {
  sp_str_t name;
  sp_str_t status;
  sp_str_t path;
} dependency_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void free_str(sp_str_t str) {
  if (str.data) {
    sp_free((void*)str.data);
  }
}

static void free_str_array(sp_dyn_array(sp_str_t) array) {
  if (!array) return;
  for (u32 i = 0; i < sp_dyn_array_size(array); i++) {
    free_str(array[i]);
  }
  sp_dyn_array_free(array);
}

int main(void) {
  sp_example_init();

  dependency_t dep_instance = {
    .name = SP_LIT("spn"),
    .status = SP_LIT("ready"),
    .path = SP_LIT("/tmp/project"),
  };
  dependency_t* dep = &dep_instance;

  sp_str_t name = sp_str_pad(dep->name, 20);
  sp_str_t status = sp_str_pad(dep->status, 10);
  SP_LOG("{} {} {}", SP_FMT_STR(name), SP_FMT_STR(status), SP_FMT_STR(dep->path));

  sp_str_t names[] = {
    SP_LIT("alice"),
    SP_LIT("bob"),
    SP_LIT("charlotte"),
  };
  sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(names, 3);

  free_str(name);
  free_str(status);
  free_str_array(padded);

  sp_example_shutdown();
  return 0;
}
