#include "cli.h"

typedef union {
  bool b;
  const c8* s;
  sp_str_t v;
  s8 s8v;
  s16 s16v;
  s32 s32v;
  s64 s64v;
  u8 u8v;
  u16 u16v;
  u32 u32v;
  u64 u64v;
} cli_assign_slot_t;

typedef struct {
  bool fail;
  const c8* cstr;
  bool cstr_null;
  const c8* str;
  bool str_null;
  s64 num;
  u64 unum;
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

  bool ok = sp_cli_assign(t.kind, &slot, value);
  EXPECT_EQ(!t.expect.fail, ok);

  switch (t.kind) {
    case SP_CLI_OPT_CSTR:
      if (t.expect.cstr_null) EXPECT_EQ(SP_NULLPTR, (void*)slot.s);
      else SP_EXPECT_STR_EQ_CSTR(sp_cstr_as_str(slot.s), t.expect.cstr ? t.expect.cstr : "");
      break;
    case SP_CLI_OPT_STR:
      if (t.expect.str_null) {
        EXPECT_EQ(SP_NULLPTR, (void*)slot.v.data);
        EXPECT_EQ(0u, slot.v.len);
      }
      else SP_EXPECT_STR_EQ_CSTR(slot.v, t.expect.str ? t.expect.str : "");
      break;
    case SP_CLI_OPT_BOOLEAN: EXPECT_EQ(t.expect.flag, slot.b); break;
    case SP_CLI_OPT_S8:  EXPECT_EQ(sp_cast(s8, t.expect.num), slot.s8v); break;
    case SP_CLI_OPT_S16: EXPECT_EQ(sp_cast(s16, t.expect.num), slot.s16v); break;
    case SP_CLI_OPT_S32: EXPECT_EQ(sp_cast(s32, t.expect.num), slot.s32v); break;
    case SP_CLI_OPT_S64: EXPECT_EQ(t.expect.num, slot.s64v); break;
    case SP_CLI_OPT_U8:  EXPECT_EQ(sp_cast(u8, t.expect.unum), slot.u8v); break;
    case SP_CLI_OPT_U16: EXPECT_EQ(sp_cast(u16, t.expect.unum), slot.u16v); break;
    case SP_CLI_OPT_U32: EXPECT_EQ(sp_cast(u32, t.expect.unum), slot.u32v); break;
    case SP_CLI_OPT_U64: EXPECT_EQ(t.expect.unum, slot.u64v); break;
  }
}

UTEST_F(cli_assign, cstr) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_CSTR,
    .value = "hello",
    .expect = { .cstr = "hello" },
  });
}

UTEST_F(cli_assign, cstr_empty) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_CSTR,
    .value = "",
    .expect = { .cstr = "" },
  });
}

UTEST_F(cli_assign, cstr_null_value) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_CSTR,
    .null_value = true,
    .expect = { .cstr_null = true },
  });
}

UTEST_F(cli_assign, str) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_STR,
    .value = "hello",
    .expect = { .str = "hello" },
  });
}

UTEST_F(cli_assign, str_null_value) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_STR,
    .null_value = true,
    .expect = { .str_null = true },
  });
}

UTEST_F(cli_assign, s64) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_S64,
    .value = "42",
    .expect = { .num = 42 },
  });
}

UTEST_F(cli_assign, s64_negative) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_S64,
    .value = "-7",
    .expect = { .num = -7 },
  });
}

UTEST_F(cli_assign, s64_invalid_keeps_preset) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_S64,
    .value = "abc",
    .preset = { .s64v = 999 },
    .expect = {
      .fail = true,
      .num = 999,
    },
  });
}

UTEST_F(cli_assign, s8) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_S8,
    .value = "-128",
    .expect = { .num = -128 },
  });
}

UTEST_F(cli_assign, s8_overflow_keeps_preset) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_S8,
    .value = "128",
    .preset = { .s8v = 7 },
    .expect = {
      .fail = true,
      .num = 7,
    },
  });
}

UTEST_F(cli_assign, s32_overflow) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_S32,
    .value = "2147483648",
    .expect = {
      .fail = true,
    },
  });
}

UTEST_F(cli_assign, u32) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_U32,
    .value = "4294967295",
    .expect = { .unum = 4294967295u },
  });
}

UTEST_F(cli_assign, u32_negative) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_U32,
    .value = "-1",
    .expect = {
      .fail = true,
    },
  });
}

UTEST_F(cli_assign, u8_overflow) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_U8,
    .value = "300",
    .expect = {
      .fail = true,
    },
  });
}

UTEST_F(cli_assign, u64_max) {
  run_cli_assign_test(&ur, (cli_assign_test_t) {
    .kind = SP_CLI_OPT_U64,
    .value = "18446744073709551615",
    .expect = { .unum = SP_LIMIT_U64_MAX },
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
      .fail = true,
      .flag = true,
    },
  });
}
