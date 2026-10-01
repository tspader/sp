#include "prompt.h"

sp_test(prompt, config_unset_fds_resolve_to_stdin_and_stderr) {
  sp_prompt_config_t config = sp_prompt_config_resolve(sp_zero_s(sp_prompt_config_t));
  sp_expect_eq(t, config.fds.in, sp_sys_stdin);
  sp_expect_eq(t, config.fds.out, sp_sys_stderr);
  return SP_OK;
}

sp_test(prompt, config_explicit_fds_are_kept) {
  sp_prompt_config_t config = sp_prompt_config_resolve((sp_prompt_config_t) {
    .fds = { .in = 68, .out = 69 },
  });
  sp_expect_eq(t, config.fds.in, (sp_sys_fd_t)68);
  sp_expect_eq(t, config.fds.out, (sp_sys_fd_t)69);
  return SP_OK;
}
