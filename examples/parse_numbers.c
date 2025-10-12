#define SP_IMPLEMENTATION
#include "../sp.h"

typedef bool sp_parse_result_t;
#define SP_PARSE_OK true

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void server_listen(u32 port) {
  SP_LOG("listening on port {}", SP_FMT_U32(port));
}

static void set_background_color(u32 color) {
  SP_LOG("background color: 0x{:06x}", SP_FMT_U32(color));
}

static void set_target_fps(f32 fps) {
  SP_LOG("target fps: {}", SP_FMT_F32(fps));
}

int main(void) {
  sp_example_init();

  sp_str_t port_str = SP_LIT("8080");
  u32 port = 0;
  sp_parse_result_t result = sp_parse_u32_ex(port_str, &port);
  if (result == SP_PARSE_OK) {
    server_listen(port);
  }

  sp_str_t hex_color = SP_LIT("ff00aa");
  u32 color = 0;
  u64 color64 = 0;
  if (sp_parse_hex_ex(hex_color, &color64)) {
    color = (u32)color64;
    set_background_color(color);
  }

  sp_str_t fps_str = SP_LIT("60.5");
  f32 fps = 0.0f;
  if (sp_parse_f32_ex(fps_str, &fps)) {
    set_target_fps(fps);
  }

  sp_example_shutdown();
  return 0;
}
