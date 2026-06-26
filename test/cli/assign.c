#include "cli.h"

typedef union {
  bool b;
  s64 i;
  const c8* s;
} cli_assign_slot_t;

typedef struct {
  sp_cli_err_kind_t err;
  const c8* err_value;
  const c8* str;
  bool str_null;
  s64 num;
  bool flag;
} cli_assign_expect_t;

typedef struct {
  sp_cli_value_kind_t kind;
  const c8* value;
  bool null_value;
  cli_assign_slot_t preset;
  cli_assign_expect_t expect;
} cli_assign_test_t;

CLI_TEST_FIXTURE(cli_assign)

static void run_cli_assign_test(s32* utest_result, cli_assign_test_t t) {
  cli_assign_slot_t slot = t.preset;
  sp_str_t value = t.null_value ? sp_zero_s(sp_str_t) : sp_str_view(t.value);

  sp_cli_err_t err = sp_cli_assign(t.kind, &slot, value);

  EXPECT_EQ(t.expect.err, err.kind);
  if (t.expect.err_value) {
    SP_EXPECT_STR_EQ_CSTR(err.value, t.expect.err_value);
  }

  switch (t.kind) {
    case SP_CLI_OPT_STRING:
      if (t.expect.str_null) EXPECT_EQ(SP_NULLPTR, (void*)slot.s);
      else SP_EXPECT_STR_EQ_CSTR(sp_cstr_as_str(slot.s), t.expect.str ? t.expect.str : "");
      break;
    case SP_CLI_OPT_INTEGER: EXPECT_EQ(t.expect.num, slot.i); break;
    case SP_CLI_OPT_BOOLEAN: EXPECT_EQ(t.expect.flag, slot.b); break;
  }
}

UTEST_F(cli_assign, string) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_STRING,
    .value = "hello",
    .expect = { .str = "hello" },
  });
}

UTEST_F(cli_assign, string_empty) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_STRING,
    .value = "",
    .expect = { .str = "" },
  });
}

UTEST_F(cli_assign, string_null_value) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_STRING,
    .null_value = true,
    .expect = { .str_null = true },
  });
}

UTEST_F(cli_assign, integer) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_INTEGER,
    .value = "42",
    .expect = { .num = 42 },
  });
}

UTEST_F(cli_assign, integer_negative) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_INTEGER,
    .value = "-7",
    .expect = { .num = -7 },
  });
}

UTEST_F(cli_assign, integer_invalid_keeps_preset) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_INTEGER,
    .value = "abc",
    .preset = { .i = 999 },
    .expect = {
      .err = SP_CLI_ERR_INVALID_VALUE,
      .err_value = "abc",
      .num = 999,
    },
  });
}

UTEST_F(cli_assign, boolean_true) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_BOOLEAN,
    .value = "true",
    .expect = { .flag = true },
  });
}

UTEST_F(cli_assign, boolean_false) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_BOOLEAN,
    .value = "false",
    .preset = { .b = true },
    .expect = { .flag = false },
  });
}

UTEST_F(cli_assign, boolean_empty_defaults_true) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_BOOLEAN,
    .value = "",
    .expect = { .flag = true },
  });
}

UTEST_F(cli_assign, boolean_invalid_keeps_preset) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_BOOLEAN,
    .value = "banana",
    .preset = { .b = true },
    .expect = {
      .err = SP_CLI_ERR_INVALID_VALUE,
      .err_value = "banana",
      .flag = true,
    },
  });
}
