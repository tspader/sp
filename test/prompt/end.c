#include "prompt.h"

sp_test(prompt, end_flushes_pending_log) {
  sp_io_dyn_mem_writer_t writer = sp_zero;
  sp_io_dyn_mem_writer_init(sp_test_arena(t), &writer);

  sp_prompt_ctx_t ctx = sp_zero;
  sp_prompt_ctx_init(&ctx, sp_test_mem(t), PROMPT_COLS, PROMPT_ROWS);
  ctx.tty = (sp_tty_t) { .io = &writer.base };

  sp_prompt_log(&ctx, "A\nB\n");
  sp_prompt_end(&ctx);

  sp_expect_str_eq_c(t, sp_io_dyn_mem_writer_as_str(&writer), "\r\nA\r\nB\r\n");
  return SP_OK;
}
