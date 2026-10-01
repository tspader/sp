#include "prompt.h"

sp_test(prompt, spinner_defaults_to_12_fps, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_widget_t widget = sp_prompt_spinner_widget(&prompt_fixture(t)->ctx, sp_zero_s(sp_prompt_spinner_t));
  sp_expect_eq(t, widget.fps, 12u);
  return SP_OK;
}

sp_test(prompt, knight_rider_defaults_to_15_fps, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_widget_t widget = sp_prompt_knight_rider_widget(&prompt_fixture(t)->ctx, sp_zero_s(sp_prompt_knight_rider_t));
  sp_expect_eq(t, widget.fps, 15u);
  return SP_OK;
}
