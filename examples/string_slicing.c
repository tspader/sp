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

  sp_str_t path = SP_LIT("/home/user/documents/file.txt");

  sp_str_t filename = sp_os_extract_file_name(path);
  sp_str_t extension = sp_os_extract_extension(path);
  sp_str_t stem = sp_os_extract_stem(path);

  sp_str_t first_10 = sp_str_sub(path, 0, 10);
  sp_str_t last_4 = sp_str_sub_reverse(path, 0, 4);

  if (sp_str_ends_with(path, SP_LIT(".txt"))) {
    sp_str_t without_ext = sp_str_sub(path, 0, path.len - 4);
    SP_UNUSED(without_ext);
  }

  sp_dyn_array(sp_str_t) parts = sp_str_split_c8(path, '/');

  SP_UNUSED(filename);
  SP_UNUSED(extension);
  SP_UNUSED(stem);
  SP_UNUSED(first_10);
  SP_UNUSED(last_4);
  SP_UNUSED(parts);

  sp_example_shutdown();
  return 0;
}
