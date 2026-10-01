#include "prompt.h"

#define LOG_MAX_PENDING 2

typedef struct {
  const c8* pending [LOG_MAX_PENDING + 1];
  const c8* text;
} expect_t;

typedef struct {
  const c8* name;
  prompt_act_t acts [PROMPT_MAX_ACTS];
  expect_t expect;
} test_t;

static const test_t tests [] = {
  {
    .name = "starts_empty",
  },
  {
    .name = "log_queues_copies_in_order",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .text = "A" },
      { .kind = PROMPT_ACT_LOG, .text = "B" },
    },
    .expect = {
      .pending = { "A", "B" },
    },
  },
  {
    .name = "tick_flushes_lines_with_crlf",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .text = "A" },
      { .kind = PROMPT_ACT_LOG, .text = "B" },
      { .kind = PROMPT_ACT_TICK, .ticks = 2 },
    },
    .expect = {
      .text = "A\r\nB\r\n",
    },
  },
  {
    .name = "log_while_running_flushes_without_events",
    .acts = {
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
      { .kind = PROMPT_ACT_LOG, .text = "A" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "A\r\n",
    },
  },
  {
    .name = "queue_is_reused_across_flushes",
    .acts = {
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
      { .kind = PROMPT_ACT_LOG, .text = "A" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
      { .kind = PROMPT_ACT_LOG, .text = "B" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
      { .kind = PROMPT_ACT_LOG, .text = "C" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "A\r\nB\r\nC\r\n",
    },
  },
  {
    .name = "embedded_newline_splits_rows",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .text = "A\nB" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "A\r\nB\r\n",
    },
  },
  {
    .name = "carriage_return_before_newline_is_stripped",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .text = "A\r\nB" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "A\r\nB\r\n",
    },
  },
  {
    .name = "single_trailing_newline_is_swallowed",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .text = "A\n" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "A\r\n",
    },
  },
  {
    .name = "empty_text_is_a_blank_row",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .text = "" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "\r\n",
    },
  },
  {
    .name = "log_from_thread_is_flushed",
    .acts = {
      { .kind = PROMPT_ACT_LOG, .threaded = true, .text = "A" },
      { .kind = PROMPT_ACT_TICK, .ticks = 1 },
    },
    .expect = {
      .text = "A\r\n",
    },
  },
};

sp_test_each(prompt, log, test_t, tests, .setup = prompt_setup, .teardown = prompt_teardown) {
  sp_prompt_ctx_t* ctx = &prompt_fixture(t)->ctx;
  sp_try(prompt_act(t, it->acts));

  u32 active = ctx->channel.log.active;
  sp_da(sp_str_t) pending = ctx->channel.log.pending[active];
  sp_expect_strs_eq(t, pending, sp_da_size(pending), it->expect.pending);
  sp_expect(t, sp_da_empty(ctx->channel.log.pending[1 - active]));
  sp_expect_eq(t, sp_mem_arena_bytes_used(ctx->channel.log.arenas[1 - active]), 0u);
  if (sp_da_empty(pending)) {
    sp_expect_eq(t, sp_mem_arena_bytes_used(ctx->channel.log.arenas[active]), 0u);
  }

  sp_expect_eq(t, sp_atomic_s32_load(&ctx->wake.pending, SP_ATOMIC_SEQ_CST), SP_PROMPT_WAKE_NOT_PENDING);
  sp_expect_str_eq_c(t, prompt_text(t), it->expect.text);
  return SP_OK;
}
