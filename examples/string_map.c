#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void free_str_array(sp_dyn_array(sp_str_t) array) {
  if (!array) return;
  for (u32 i = 0; i < sp_dyn_array_size(array); i++) {
    if (array[i].data) {
      sp_free((void*)array[i].data);
    }
  }
  sp_dyn_array_free(array);
}

int main(void) {
  sp_example_init();

  sp_str_t lines[] = {
      SP_LIT("  hello world  "),
      SP_LIT("\ttest line\n"),
      SP_LIT("  another  "),
  };

  sp_dyn_array(sp_str_t) trimmed = sp_str_map(lines, 3, NULL, sp_str_map_kernel_trim);
  sp_dyn_array(sp_str_t) upper = sp_str_map(lines, 3, NULL, sp_str_map_kernel_to_upper);

  u32 width = 20;
  sp_dyn_array(sp_str_t) padded = sp_str_map(lines, 3, &width, sp_str_map_kernel_pad);

  sp_str_t longest = sp_str_find_longest_n(lines, 3);
  sp_dyn_array(sp_str_t) aligned = sp_str_pad_to_longest(lines, 3);

  SP_LOG("longest line has {} characters", SP_FMT_U32(longest.len));

  free_str_array(trimmed);
  free_str_array(upper);
  free_str_array(padded);
  free_str_array(aligned);

  sp_example_shutdown();
  return 0;
}
