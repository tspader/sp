#define SP_IMPLEMENTATION
#include "../sp.h"

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

typedef struct {
  char cFileName[260];
} fake_find_data_t;

int main(void) {
  sp_example_init();

  sp_str_t file_path = SP_LIT("C:/temp/file.txt");
  fake_find_data_t find_data = { "file.txt" };
  sp_os_file_attr_t attrs = SP_OS_FILE_ATTR_NONE;

  sp_os_directory_entry_t entry = SP_RVAL(sp_os_directory_entry_t) {
      .file_path = file_path,
      .file_name = sp_str_from_cstr(find_data.cFileName),
#ifdef SP_WIN32
      .attributes = sp_os_winapi_attr_to_sp_attr(attrs),
#else
      .attributes = attrs,
#endif
  };

  sp_precise_epoch_time_t time = SP_RVAL(sp_precise_epoch_time_t) {
      .s = 1234,
      .ns = 0
  };

  sp_str_t str = SP_LIT("example");
  u32 start = 1;
  u32 end = 4;
  sp_str_t substr = SP_RVAL(sp_str_t) {
      .len = end - start,
      .data = str.data + start
  };

  sp_free((void*)entry.file_name.data);
  SP_UNUSED(time);
  SP_UNUSED(substr);

  sp_example_shutdown();
  return 0;
}
