#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void release_formatted(sp_str_t str) {
  if (str.data) {
    sp_free((void*)str.data);
  }
}

int main(void) {
  sp_example_init();

  u32 file_count = 42;
  f32 elapsed = 16.7f;
  sp_str_t base_dir = SP_LIT("/tmp");
  sp_str_t filename = SP_LIT("data.bin");
  bool is_running = true;
  u32 port = 8080;
  u32 rgb_value = 0x33cc99;

  sp_str_t msg = sp_format("{} files processed in {}ms",
      SP_FMT_U32(file_count), SP_FMT_F32(elapsed));
  sp_str_t path = sp_format("{}/{}",
      SP_FMT_STR(base_dir), SP_FMT_STR(filename));
  sp_str_t quoted = sp_format("Error in file {}",
      SP_FMT_QUOTED_STR(filename));
  sp_str_t status = sp_format("Server {} on port {}",
      SP_FMT_CSTR(is_running ? "running" : "stopped"),
      SP_FMT_U32(port));
  sp_str_t hex = sp_format("Color: 0x{:06x}",
      SP_FMT_U32(rgb_value));

  release_formatted(msg);
  release_formatted(path);
  release_formatted(quoted);
  release_formatted(status);
  release_formatted(hex);

  sp_example_shutdown();
  return 0;
}
