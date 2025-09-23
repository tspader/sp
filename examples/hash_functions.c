#define SP_IMPLEMENTATION
#include "../sp.h"

typedef struct {
  sp_str_t data;
  u32 len;
} file_data_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

int main(void) {
  sp_example_init();

  const c8* filename = "example.txt";
  u8 buffer[] = { 1, 2, 3, 4, 5 };
  u32 buffer_size = (u32)(sizeof(buffer) / sizeof(buffer[0]));
  sp_hash_t hashes[] = { 10, 20, 30 };
  u32 hash_count = (u32)(sizeof(hashes) / sizeof(hashes[0]));

  file_data_t file_data = {
    .data = SP_LIT("contents"),
    .len = 8,
  };

  sp_hash_t file_hash = sp_hash_cstr(filename);
  sp_hash_t data_hash = sp_hash_bytes(buffer, buffer_size, 0);
  sp_hash_t combined = sp_hash_combine(hashes, hash_count);
  sp_hash_t content_hash = sp_hash_bytes((const u8*)file_data.data.data, file_data.len, 42);

  SP_UNUSED(file_hash);
  SP_UNUSED(data_hash);
  SP_UNUSED(combined);
  SP_UNUSED(content_hash);

  sp_example_shutdown();
  return 0;
}
