#define SP_IMPLEMENTATION
#include "../sp.h"
#include <stdlib.h>

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

sp_dyn_array(sp_str_t) get_file_list(void) {
  sp_dyn_array(sp_str_t) files = SP_NULLPTR;
  sp_dyn_array_push(files, SP_LIT("src/main.c"));
  sp_dyn_array_push(files, SP_LIT("src/app.c"));
  sp_dyn_array_push(files, SP_LIT("src/zeta.c"));
  sp_dyn_array_push(files, SP_LIT("src/core.c"));
  return files;
}

int main(void) {
  sp_example_init();

  sp_dyn_array(sp_str_t) files = get_file_list();
  qsort(files, sp_dyn_array_size(files), sizeof(sp_str_t), sp_str_sort_kernel_alphabetical);

  sp_dyn_array_free(files);

  sp_example_shutdown();
  return 0;
}
