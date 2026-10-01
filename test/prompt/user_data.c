#include "prompt.h"

static void on_event(sp_prompt_ctx_t* ctx, sp_prompt_event_t event) {
}

sp_test(prompt, user_data_is_the_running_widgets, .setup = prompt_setup, .teardown = prompt_teardown) {
  s32 data = 0;

  prompt_start(t, (sp_prompt_widget_t) {
    .user_data = &data,
    .on_event = on_event,
  });

  sp_expect(t, sp_prompt_get_user_data(&prompt_fixture(t)->ctx) == &data);
  return SP_OK;
}
