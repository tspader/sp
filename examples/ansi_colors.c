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

static const c8* get_error_from_some_external_library(void) {
  return "an error occurred";
}

int main(void) {
  sp_example_init();

  sp_str_t formatted1 = sp_format("{:fg brightred}", SP_FMT_CSTR("FAILED"));
  free_str(formatted1);

  sp_str_t formatted2 = sp_format("{:fg brightcyan} tests failed", SP_FMT_U32(3));
  free_str(formatted2);

  sp_str_builder_t output = SP_ZERO_INITIALIZE();
  sp_str_builder_append_fmt(&output, "{:fg brightgreen}", SP_FMT_CSTR("All tests passed"));
  sp_str_t built = sp_str_builder_write(&output);
  free_str(built);

  sp_str_t status = SP_LIT("Some status message produced by your code");
  sp_str_t formatted3 = sp_format("{:fg brightcyan}", SP_FMT_STR(status));
  free_str(formatted3);

  SP_LOG(
    "{:fg brightyellow}: {:fg brightblack}",
    SP_FMT_CSTR("warning"),
    SP_FMT_CSTR(get_error_from_some_external_library())
  );

  sp_example_shutdown();
  return 0;
}
