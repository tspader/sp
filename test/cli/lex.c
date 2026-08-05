#include "cli.h"

typedef struct {
  bool escape;
  bool is_long;
  bool is_short;
} cli_token_expect_t;

typedef struct {
  const c8* arg;
  cli_token_expect_t expect;
} cli_token_test_t;

UTEST_EMPTY_FIXTURE(cli_token)

static void run_cli_token_test(s32* utest_result, cli_token_test_t t) {
  sp_str_t tok = sp_cstr_as_str(t.arg);
  EXPECT_EQ(t.expect.escape,   sp_cli_token_is_escape(tok));
  EXPECT_EQ(t.expect.is_long,  sp_cli_token_is_long(tok));
  EXPECT_EQ(t.expect.is_short, sp_cli_token_is_short(tok));
}

UTEST_F(cli_token, escape) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "--",
    .expect = { .escape = true },
  });
}

UTEST_F(cli_token, long_flag) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "--verbose",
    .expect = { .is_long = true },
  });
}

UTEST_F(cli_token, long_flag_with_value) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "--jobs=4",
    .expect = { .is_long = true },
  });
}

UTEST_F(cli_token, short_flag) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "-v",
    .expect = { .is_short = true },
  });
}

UTEST_F(cli_token, short_cluster) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "-xvf",
    .expect = { .is_short = true },
  });
}

UTEST_F(cli_token, dash) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "-",
  });
}

UTEST_F(cli_token, plain) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "build",
  });
}

UTEST_F(cli_token, empty) {
  run_cli_token_test(&ur, (cli_token_test_t) {
    .arg = "",
  });
}

typedef struct {
  const c8* name;
  const c8* value;
  bool has_value;
} cli_long_expect_t;

typedef struct {
  const c8* arg;
  cli_long_expect_t expect;
} cli_long_test_t;

UTEST_EMPTY_FIXTURE(cli_long)

static void run_cli_long_test(s32* utest_result, cli_long_test_t t) {
  sp_str_t value = sp_zero_s(sp_str_t);
  bool has_value = false;
  sp_str_t name = sp_cli_token_to_long(sp_cstr_as_str(t.arg), &value, &has_value);
  SP_EXPECT_STR_EQ_CSTR(name, t.expect.name);
  EXPECT_EQ(t.expect.has_value, has_value);
  SP_EXPECT_STR_EQ_CSTR(value, t.expect.value ? t.expect.value : "");
}

UTEST_F(cli_long, bare) {
  run_cli_long_test(&ur, (cli_long_test_t) {
    .arg = "--verbose",
    .expect = { .name = "verbose" },
  });
}

UTEST_F(cli_long, split) {
  run_cli_long_test(&ur, (cli_long_test_t) {
    .arg = "--jobs=4",
    .expect = { .name = "jobs", .value = "4", .has_value = true },
  });
}

UTEST_F(cli_long, empty_value) {
  run_cli_long_test(&ur, (cli_long_test_t) {
    .arg = "--name=",
    .expect = { .name = "name", .has_value = true },
  });
}

UTEST_F(cli_long, first_equals_splits) {
  run_cli_long_test(&ur, (cli_long_test_t) {
    .arg = "--filter=a=b",
    .expect = { .name = "filter", .value = "a=b", .has_value = true },
  });
}

typedef enum {
  CLI_SHORTS_END,
  CLI_SHORTS_FLAG,
  CLI_SHORTS_VALUE,
  CLI_SHORTS_DONE,
} cli_shorts_op_t;

typedef struct {
  cli_shorts_op_t op;
  c8 flag;
  const c8* value;
  bool done;
} cli_shorts_step_t;

#define CLI_SHORTS_MAX_STEPS 8

typedef struct {
  const c8* arg;
  cli_shorts_step_t steps [CLI_SHORTS_MAX_STEPS];
} cli_shorts_test_t;

UTEST_EMPTY_FIXTURE(cli_shorts)

static void run_cli_shorts_test(s32* utest_result, cli_shorts_test_t t) {
  sp_cli_shorts_t shorts = sp_cli_token_to_short(sp_cstr_as_str(t.arg));
  sp_carr_for(t.steps, it) {
    cli_shorts_step_t step = t.steps[it];
    switch (step.op) {
      case CLI_SHORTS_END: {
        return;
      }
      case CLI_SHORTS_FLAG: {
        EXPECT_EQ(step.flag, sp_cli_shorts_next_flag(&shorts));
        break;
      }
      case CLI_SHORTS_VALUE: {
        SP_EXPECT_STR_EQ_CSTR(sp_cli_shorts_next_value(&shorts), step.value ? step.value : "");
        break;
      }
      case CLI_SHORTS_DONE: {
        EXPECT_EQ(step.done, sp_cli_shorts_done(&shorts));
        break;
      }
    }
  }
}

UTEST_F(cli_shorts, cluster) {
  run_cli_shorts_test(&ur, (cli_shorts_test_t) {
    .arg = "-xvf",
    .steps = {
      { .op = CLI_SHORTS_FLAG, .flag = 'x' },
      { .op = CLI_SHORTS_FLAG, .flag = 'v' },
      { .op = CLI_SHORTS_FLAG, .flag = 'f' },
      { .op = CLI_SHORTS_DONE, .done = true },
      { .op = CLI_SHORTS_FLAG, .flag = 0 },
    },
  });
}

UTEST_F(cli_shorts, inline_value) {
  run_cli_shorts_test(&ur, (cli_shorts_test_t) {
    .arg = "-ovalue",
    .steps = {
      { .op = CLI_SHORTS_FLAG, .flag = 'o' },
      { .op = CLI_SHORTS_VALUE, .value = "value" },
      { .op = CLI_SHORTS_DONE, .done = true },
    },
  });
}

UTEST_F(cli_shorts, value_after_flag) {
  run_cli_shorts_test(&ur, (cli_shorts_test_t) {
    .arg = "-vo",
    .steps = {
      { .op = CLI_SHORTS_FLAG, .flag = 'v' },
      { .op = CLI_SHORTS_VALUE, .value = "o" },
    },
  });
}

UTEST_F(cli_shorts, value_at_end_is_empty) {
  run_cli_shorts_test(&ur, (cli_shorts_test_t) {
    .arg = "-o",
    .steps = {
      { .op = CLI_SHORTS_FLAG, .flag = 'o' },
      { .op = CLI_SHORTS_VALUE },
      { .op = CLI_SHORTS_DONE, .done = true },
    },
  });
}
