#define SP_IMPLEMENTATION
#include "../sp.h"

typedef struct {
  sp_str_t file_name;
} entry_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void use_secure_connection(sp_str_t url) {
  SP_LOG("using https for {}", SP_FMT_STR(url));
}

static void use_standard_connection(sp_str_t url) {
  SP_LOG("using http for {}", SP_FMT_STR(url));
}

int main(void) {
  sp_example_init();

  sp_str_t url = SP_LIT("https://example.com");

  if (sp_str_starts_with(url, SP_LIT("https://"))) {
    use_secure_connection(url);
  }
  else if (sp_str_starts_with(url, SP_LIT("http://"))) {
    use_standard_connection(url);
  }

  sp_dyn_array(entry_t) entries = SP_NULLPTR;
  entry_t hidden = { .file_name = SP_LIT(".hidden") };
  entry_t visible = { .file_name = SP_LIT("visible.txt") };
  sp_dyn_array_push(entries, hidden);
  sp_dyn_array_push(entries, visible);

  sp_dyn_array_for(entries, i) {
    if (sp_str_starts_with(entries[i].file_name, SP_LIT("."))) {
      continue;
    }
  }

  sp_dyn_array_free(entries);

  sp_example_shutdown();
  return 0;
}
