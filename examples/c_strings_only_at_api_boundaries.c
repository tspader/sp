#define SP_IMPLEMENTATION
#include "../sp.h"

#define sp_export

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void SDL_Log(const char* fmt, ...) {
  SP_UNUSED(fmt);
}

static void process_config(sp_str_t config) {
  SP_UNUSED(config);
}

static void process_arguments(sp_dyn_array(sp_str_t) args) {
  SP_UNUSED(args);
}

void my_log_wrapper(sp_str_t message) {
  c8* cstr = sp_str_to_cstr(message);
  SDL_Log("%s", cstr);
  sp_free(cstr);
}

sp_export void plugin_init(const char* config_path) {
  sp_str_t config = sp_str_from_cstr(config_path);

  sp_str_t parent = sp_os_parent_path(config);
  sp_str_t stem = sp_os_extract_stem(config);

  process_config(config);

  sp_free((void*)config.data);
  sp_free((void*)parent.data);
  SP_UNUSED(stem);
}

int main(int argc, char** argv) {
  sp_example_init();

  sp_dyn_array(sp_str_t) args = SP_NULLPTR;
  for (s32 i = 1; i < argc; i++) {
    sp_dyn_array_push(args, sp_str_from_cstr(argv[i]));
  }

  process_arguments(args);
  if (args) {
    for (u32 i = 0; i < sp_dyn_array_size(args); i++) {
      sp_free((void*)args[i].data);
    }
  }
  sp_dyn_array_free(args);

  sp_example_shutdown();
  return 0;
}
