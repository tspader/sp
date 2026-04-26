#define SP_IMPLEMENTATION
#include "sp.h"

typedef struct {
  sp_tm_timer_t timer;
} state_t;

sp_app_result_t on_init(sp_app_t* app) {
  state_t* s = (state_t*)app->user_data;
  sp_log("on_init()");
  s->timer = sp_tm_start_timer();
  return SP_APP_CONTINUE;
}

sp_app_result_t on_update(sp_app_t* app) {
  state_t* s = (state_t*)app->user_data;
  u64 elapsed = sp_tm_read_timer(&s->timer);
  sp_log("elapsed: {.gray .duration}", sp_fmt_uint(elapsed));
  if (elapsed >= sp_tm_s_to_ns(1)) {
    return SP_APP_QUIT;
  };
  return SP_APP_CONTINUE;
}

void on_deinit(sp_app_t* app) {
  sp_log("on_deinit()");
}

sp_app_config_t app_main(s32 num_args, const c8** args) {
  sp_da(u64) arr = sp_zero();
  sp_da_push(arr, 69);
  sp_da_for(arr, it) {
    sp_log("arr[{.gray}] -> {}", sp_fmt_uint(it), sp_fmt_uint(arr[it]));
  }
  return (sp_app_config_t) {
    .fps = 30,
    .on_init = on_init,
    .on_update = on_update,
    .on_deinit = on_deinit,
    .user_data = sp_alloc_type(state_t)
  };
}
SP_APP_MAIN(app_main)
