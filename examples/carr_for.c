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

  sp_formatter_t formatters[] = {
      { .id = SP_FMT_ID(str), .fn = sp_fmt_format_str },
      { .id = SP_FMT_ID(u32), .fn = sp_fmt_format_u32 },
      { .id = SP_FMT_ID(f32), .fn = sp_fmt_format_f32 },
  };

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  sp_format_arg_t arg = SP_FMT_ARG(str, SP_LIT("value"));

  SP_CARR_FOR(formatters, i) {
      if (arg.id == formatters[i].id) {
          formatters[i].fn(&builder, &arg);
          break;
      }
  }

  sp_str_t output = sp_str_builder_write(&builder);
  sp_free((void*)output.data);

  sp_example_shutdown();
  return 0;
}
