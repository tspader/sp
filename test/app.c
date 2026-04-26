#include "sp.h"
#include "test.h"

#include "utest.h"

SP_TEST_MAIN()

typedef struct {
  const c8* name;
  sp_app_config_t config;
  s32 expected_return_code;
} sp_app_test_case_t;

// Helpers to track callback invocations
static bool g_init_called;
static bool g_poll_called;
static bool g_update_called;
static bool g_deinit_called;

static void reset_flags(void) {
  g_init_called = false;
  g_poll_called = false;
  g_update_called = false;
  g_deinit_called = false;
}

// Basic callbacks
static sp_app_result_t on_init_continue(sp_app_t* app) {
  (void)app;
  g_init_called = true;
  return SP_APP_CONTINUE;
}

static sp_app_result_t on_init_quit(sp_app_t* app) {
  (void)app;
  g_init_called = true;
  return SP_APP_QUIT;
}

static sp_app_result_t on_init_err(sp_app_t* app) {
  (void)app;
  g_init_called = true;
  return SP_APP_ERR;
}

static sp_app_result_t on_poll_continue(sp_app_t* app) {
  (void)app;
  g_poll_called = true;
  return SP_APP_CONTINUE;
}

static sp_app_result_t on_update_quit(sp_app_t* app) {
  (void)app;
  g_update_called = true;
  return SP_APP_QUIT;
}

static sp_app_result_t on_update_err(sp_app_t* app) {
  (void)app;
  g_update_called = true;
  return SP_APP_ERR;
}

static sp_app_result_t on_update_with_code(sp_app_t* app) {
  g_update_called = true;
  app->rc = 42;
  return SP_APP_QUIT;
}

static void on_deinit_continue(sp_app_t* app) {
  (void)app;
  g_deinit_called = true;
}


UTEST(app, run) {
  static sp_app_test_case_t test_cases[] = {
    {
      .name = "update_quit",
      .config = { .on_update = on_update_quit, .fps = 1000 },
      .expected_return_code = 0,
    },
    {
      .name = "update_err",
      .config = { .on_update = on_update_err, .fps = 1000 },
      .expected_return_code = 1,
    },
    {
      .name = "update_sets_code",
      .config = { .on_update = on_update_with_code, .fps = 1000 },
      .expected_return_code = 42,
    },
    {
      .name = "init_continue",
      .config = { .on_init = on_init_continue, .on_update = on_update_quit, .fps = 1000 },
      .expected_return_code = 0,
    },
    {
      .name = "init_quit",
      .config = { .on_init = on_init_quit, .on_update = on_update_quit, .fps = 1000 },
      .expected_return_code = 0,
    },
    {
      .name = "init_err",
      .config = { .on_init = on_init_err, .on_update = on_update_quit, .fps = 1000 },
      .expected_return_code = 1,
    },
  };

  sp_carr_for(test_cases, i) {
    sp_app_test_case_t* test = &test_cases[i];
    s32 result = sp_app_run(test->config);
    EXPECT_EQ_MSG(result, test->expected_return_code, test->name);
  }
}

UTEST(app, init_quit_skips_update) {
  reset_flags();
  sp_app_run((sp_app_config_t){
    .on_init = on_init_quit,
    .on_update = on_update_quit,
    .on_deinit = on_deinit_continue,
    .fps = 1000,
  });
  EXPECT_TRUE(g_init_called);
  EXPECT_FALSE(g_update_called);
  EXPECT_TRUE(g_deinit_called);
}

UTEST(app, init_err_skips_update) {
  reset_flags();
  sp_app_run((sp_app_config_t){
    .on_init = on_init_err,
    .on_update = on_update_quit,
    .on_deinit = on_deinit_continue,
    .fps = 1000,
  });
  EXPECT_TRUE(g_init_called);
  EXPECT_FALSE(g_update_called);
  EXPECT_TRUE(g_deinit_called);
}

UTEST(app, poll_called_before_update) {
  reset_flags();
  sp_app_run((sp_app_config_t){
    .on_poll = on_poll_continue,
    .on_update = on_update_quit,
    .fps = 1000,
  });
  EXPECT_TRUE(g_poll_called);
  EXPECT_TRUE(g_update_called);
}

UTEST(app, full_lifecycle) {
  reset_flags();
  sp_app_run((sp_app_config_t){
    .on_init = on_init_continue,
    .on_poll = on_poll_continue,
    .on_update = on_update_quit,
    .on_deinit = on_deinit_continue,
    .fps = 1000,
  });
  EXPECT_TRUE(g_init_called);
  EXPECT_TRUE(g_poll_called);
  EXPECT_TRUE(g_update_called);
  EXPECT_TRUE(g_deinit_called);
}

UTEST(app, free_mode_full_lifecycle) {
  reset_flags();
  s32 rc = sp_app_run((sp_app_config_t){
    .on_init = on_init_continue,
    .on_poll = on_poll_continue,
    .on_update = on_update_quit,
    .on_deinit = on_deinit_continue,
    .fps = 1000,
    .mode = SP_APP_MODE_FREE,
  });
  EXPECT_EQ(rc, 0);
  EXPECT_TRUE(g_init_called);
  EXPECT_TRUE(g_poll_called);
  EXPECT_TRUE(g_update_called);
  EXPECT_TRUE(g_deinit_called);
}

// In free mode, on_poll runs on every tick; on_update is gated by fps.
// Verify poll is called many more times than update over a fixed wall-clock budget.
static u32 g_free_poll_count;
static u32 g_free_update_count;
static sp_tm_timer_t g_free_timer;

static sp_app_result_t on_poll_count(sp_app_t* app) {
  (void)app;
  g_free_poll_count++;
  if (sp_tm_read_timer(&g_free_timer) >= sp_tm_ms_to_ns(50)) {
    return SP_APP_QUIT;
  }
  return SP_APP_CONTINUE;
}

static sp_app_result_t on_update_count(sp_app_t* app) {
  (void)app;
  g_free_update_count++;
  return SP_APP_CONTINUE;
}

UTEST(app, free_mode_poll_runs_hot) {
  g_free_poll_count = 0;
  g_free_update_count = 0;
  g_free_timer = sp_tm_start_timer();

  sp_app_run((sp_app_config_t){
    .on_poll = on_poll_count,
    .on_update = on_update_count,
    .fps = 60,
    .mode = SP_APP_MODE_FREE,
  });

  EXPECT_GT(g_free_poll_count, g_free_update_count);
}
