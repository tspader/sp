#define SP_IMPLEMENTATION
#include "../sp.h"

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

int main(void) {
  sp_example_init();

  sp_str_t exe = sp_os_get_executable_path();
  sp_str_t config_path = sp_os_join_path(sp_os_parent_path(exe), SP_LIT("config.toml"));

  sp_str_t filename = sp_os_extract_file_name(config_path);
  sp_str_t stem = sp_os_extract_stem(config_path);
  sp_str_t ext = sp_os_extract_extension(config_path);

  sp_str_t canonical = sp_os_canonicalize_path(SP_LIT("../data/./files"));

  SP_UNUSED(filename);
  SP_UNUSED(stem);
  SP_UNUSED(ext);

  free_str(exe);
  free_str(config_path);
  free_str(canonical);

  sp_example_shutdown();
  return 0;
}
