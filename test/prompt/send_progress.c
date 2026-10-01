#include "prompt.h"

sp_test(prompt, send_progress_typed_helpers_round_trip, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;
  s32 target = 0;

  sp_prompt_send_progress_f32(ctx, 0.5f);
  sp_expect_eq(t, ctx->progress.value.f, 0.5);

  sp_prompt_send_progress_f64(ctx, 3.14159);
  sp_expect_eq(t, ctx->progress.value.f, 3.14159);

  sp_prompt_send_progress_u32(ctx, 0xDEADBEEF);
  sp_expect_eq(t, ctx->progress.value.u, (u64)0xDEADBEEF);

  sp_prompt_send_progress_u64(ctx, 0x1122334455667788ull);
  sp_expect_eq(t, ctx->progress.value.u, (u64)0x1122334455667788ull);

  sp_prompt_send_progress_s32(ctx, -7);
  sp_expect_eq(t, ctx->progress.value.i, (s64)-7);

  sp_prompt_send_progress_s64(ctx, -1234567890123ll);
  sp_expect_eq(t, ctx->progress.value.i, (s64)-1234567890123ll);

  sp_prompt_send_progress_ptr(ctx, &target);
  sp_expect(t, ctx->progress.value.ptr == &target);

  sp_prompt_send_progress_bool(ctx, true);
  sp_expect(t, ctx->progress.value.b);
  return SP_OK;
}
