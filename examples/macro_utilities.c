#define SP_IMPLEMENTATION
#include "../sp.h"

#define MY_FUNC_NAME(type) SP_MACRO_CAT(process_, type)
#define MY_STRUCT_NAME(name) SP_MACRO_CAT(name, _data_t)

static void MY_FUNC_NAME(int)(int value) { SP_UNUSED(value); }
static void MY_FUNC_NAME(float)(float value) { SP_UNUSED(value); }

typedef struct MY_STRUCT_NAME(player) {
    u32 id;
    sp_str_t name;
} player_data_t;

#define LOG_VAR(var) \
    SP_LOG("{} = {}", SP_FMT_CSTR(SP_MACRO_STR(var)), SP_FMT_U32(var))

#define DECLARE_HANDLE(name) \
    typedef struct SP_MACRO_CAT(name, _impl_t)* name##_t

DECLARE_HANDLE(window);
DECLARE_HANDLE(renderer);

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

int main(void) {
  sp_example_init();

  u32 count = 42;
  LOG_VAR(count);

  MY_FUNC_NAME(int)(5);
  MY_FUNC_NAME(float)(1.0f);

  player_data_t player = { .id = 1, .name = SP_LIT("player1") };
  SP_UNUSED(player);

  sp_example_shutdown();
  return 0;
}
