#ifndef CLI_TEST_H
#define CLI_TEST_H

#include "sp.h"
#include "sp/sp_cli.h"
#include "test.h"
#include "utest.h"

#define CLI_TEST_MAX_ARGS 8
#define CLI_TEST_MAX_BINDS 4

typedef struct {
  bool flags [CLI_TEST_MAX_BINDS];
  const c8* strs [CLI_TEST_MAX_BINDS];
  s64 nums [CLI_TEST_MAX_BINDS];
} cli_binds_t;

static cli_binds_t cli_binds;
static sp_str_t cli_dispatched;

static sp_cli_result_t cli_handler_ok(sp_cli_t* cli) {
  cli_dispatched = sp_str_view(cli->cmd->name);
  return SP_CLI_OK;
}

static sp_cli_result_t cli_handler_err(sp_cli_t* cli) {
  cli_dispatched = sp_str_view(cli->cmd->name);
  return SP_CLI_ERR;
}

static u32 cli_count_args(const c8** args) {
  u32 num = 0;
  while (num < CLI_TEST_MAX_ARGS && args[num]) {
    num++;
  }
  return num;
}

#define CLI_TEST_FIXTURE(SUITE) \
  struct SUITE { \
    sp_mem_tracking_t tracker; \
    sp_mem_arena_t* arena; \
    struct { sp_mem_t tracking; sp_mem_t arena; } mem; \
  }; \
  UTEST_F_SETUP(SUITE) { \
    sp_mem_tracking_init(&ut.tracker); \
    ut.mem.tracking = sp_mem_tracking_as_allocator(&ut.tracker); \
    ut.arena = sp_mem_arena_new(ut.mem.tracking); \
    ut.mem.arena = sp_mem_arena_as_allocator(ut.arena); \
  } \
  UTEST_F_TEARDOWN(SUITE) { \
    sp_mem_arena_destroy(ut.arena); \
    EXPECT_TRUE(sp_mem_tracking_ok(&ut.tracker)); \
    sp_mem_tracking_deinit(&ut.tracker); \
  }

#endif
