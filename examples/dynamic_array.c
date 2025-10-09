#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void compile_file(sp_str_t file) {
  SP_LOG("compiling {}", SP_FMT_STR(file));
}

int main(void) {
  sp_example_init();

  sp_dyn_array(sp_str_t) files = SP_NULLPTR;

  sp_dyn_array_push(files, SP_LIT("main.c"));
  sp_dyn_array_push(files, SP_LIT("utils.c"));
  sp_dyn_array_push(files, SP_LIT("test.c"));

  sp_dyn_array_for(files, i) {
    compile_file(files[i]);
  }

  u32 count = sp_dyn_array_size(files);
  SP_LOG("total files: {}", SP_FMT_U32(count));

  sp_dyn_array_free(files);

  sp_example_shutdown();
  return 0;
}
