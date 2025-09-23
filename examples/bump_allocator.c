#define SP_IMPLEMENTATION
#include <stddef.h>
#include "../sp.h"

typedef struct {
  bool ok;
} compile_result_t;

typedef sp_hash_table(sp_str_t, u32) symbol_table_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static sp_dyn_array(sp_str_t) tokenize_file(sp_str_t source) {
  sp_dyn_array(sp_str_t) tokens = SP_NULLPTR;
  SP_UNUSED(source);
  sp_dyn_array_push(tokens, SP_LIT("token1"));
  sp_dyn_array_push(tokens, SP_LIT("token2"));
  return tokens;
}

static compile_result_t compile(symbol_table_t table) {
  SP_UNUSED(table);
  return (compile_result_t){ .ok = true };
}

int main(void) {
  sp_example_init();

  sp_bump_allocator_t temp;
  sp_allocator_t temp_allocator = sp_bump_allocator_init(&temp, 10 * 1024 * 1024);
  sp_context_push_allocator(temp_allocator);

  sp_str_t source = SP_LIT("fn main() {}");
  sp_dyn_array(sp_str_t) tokens = tokenize_file(source);
  symbol_table_t symbol_table = sp_hash_table_new(sp_str_t, u32);
  sp_hash_table_init(symbol_table, sp_str_t, u32);

  for (u32 i = 0; i < sp_dyn_array_size(tokens); i++) {
    sp_str_t symbol = sp_str_copy(tokens[i]);
    sp_hash_table_insert(symbol_table, symbol, i);
  }

  compile_result_t result = compile(symbol_table);
  SP_UNUSED(result);

  sp_hash_table_free(symbol_table);
  sp_dyn_array_free(tokens);
  sp_context_pop();
  sp_bump_allocator_clear(&temp);

  sp_example_shutdown();
  return 0;
}
