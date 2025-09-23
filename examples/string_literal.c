#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void process_text_file(sp_str_t file) {
  SP_LOG("processing text file: {}", SP_FMT_STR(file));
}

int main(void) {
  sp_example_init();

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_str_t path = SP_LIT("/home/user");
  sp_str_t delimiter = SP_LIT(", ");
  sp_str_builder_append(&builder, SP_LIT("Error: "));

  sp_str_t ext = SP_LIT(".txt");
  sp_str_t file = SP_LIT("report.txt");
  if (sp_str_equal(ext, SP_LIT(".txt"))) {
    process_text_file(file);
  }

  SP_UNUSED(path);
  SP_UNUSED(delimiter);

  sp_example_shutdown();
  return 0;
}
