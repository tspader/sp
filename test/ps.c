//#define SP_IMPLEMENTATION
#include "sp.h"
#include <stdlib.h>

#define SP_TEST_IMPLEMENTATION
#include "test.h"

#include "process.h"
#include "utest.h"
#include <unistd.h>

typedef struct {
  sp_str_t expected;
  bool enabled;
} sp_test_proc_io_stream_config_t;

typedef struct {
  sp_ps_io_config_t io;
  sp_str_t input;
  struct {
    sp_test_proc_io_stream_config_t out;
    sp_test_proc_io_stream_config_t err;
  } output;
  test_proc_function_t fn;
} sp_test_proc_io_config_t;

typedef enum {
  SP_TEST_PS_OUTPUT_MATCH,
  SP_TEST_PS_OUTPUT_MISMATCH,
} sp_test_ps_output_result_t;

typedef enum {
  SP_TEST_PROC_READ_EXACT,
  SP_TEST_PROC_READ_UNTIL_DONE,
} sp_test_proc_read_mode_t;

typedef struct {
  sp_io_stream_t* stream;
  sp_byte_buffer_t buffer;
  sp_str_t expected;
  sp_test_proc_read_mode_t mode;
  u32 expected_len;
  u64 bytes_read;
  sp_test_ps_output_result_t result;
} sp_test_proc_stream_context_t;


//////////////
// FIXTURES //
//////////////

typedef struct sp_ps {
  sp_test_file_manager_t file_manager;
  sp_byte_buffer_t buffer;
} sp_ps;

UTEST_F_SETUP(sp_ps) {
  sp_test_file_manager_init(&ut.file_manager);
  ut.buffer = (sp_byte_buffer_t) {
    .len = 1024,
    .data = (u8*)sp_alloc(1024)
  };
}

UTEST_F_TEARDOWN(sp_ps) {
  fflush(stdin);
  fflush(stdout);
  fflush(stderr);
  sp_test_file_manager_cleanup(&ut.file_manager);
}


void sp_test_proc_collect_stream(sp_test_proc_stream_context_t* ctx) {
  u64 total_read = 0;
  u32 attempts = 0;

  u32 target_len = (ctx->mode == SP_TEST_PROC_READ_EXACT) ? ctx->expected_len : ctx->buffer.len;

  while (total_read < target_len && attempts < 10) {
    u8* ptr = ctx->buffer.data + total_read;
    u32 bytes_remaining = ctx->buffer.len - total_read;
    u64 bytes_read = sp_io_read(ctx->stream, ptr, bytes_remaining);
    if (!bytes_read) {
      if (ctx->mode == SP_TEST_PROC_READ_UNTIL_DONE) {
        sp_os_sleep_ms(10);
      } else {
        sp_os_sleep_ms(10);
      }
      attempts++;
      continue;
    }
    total_read += bytes_read;
  }

  ctx->bytes_read = total_read;

  if (ctx->mode == SP_TEST_PROC_READ_EXACT) {
    SP_ASSERT_FMT(
      total_read == ctx->expected_len,
      "expected to read {} bytes but got {}",
      SP_FMT_U32(ctx->expected_len),
      SP_FMT_U64(total_read)
    );
  }
}

void sp_test_proc_check_stream(sp_test_proc_stream_context_t* ctx) {
  sp_test_proc_collect_stream(ctx);

  if (ctx->bytes_read != ctx->expected.len || !sp_os_is_memory_equal(ctx->buffer.data, ctx->expected.data, ctx->expected.len)) {
    ctx->result = SP_TEST_PS_OUTPUT_MISMATCH;
  } else {
    ctx->result = SP_TEST_PS_OUTPUT_MATCH;
  }
}


////////////////
// SP_PS_IO //
////////////////
void sp_test_proc_io(sp_test_proc_io_config_t test) {
  sp_ps_config_t config = {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), SP_CSTR(test_proc_function_to_cstr(test.fn)),
    },
    .io = test.io,
  };

  if (test.output.out.enabled) {
    sp_ps_config_add_arg(&config, sp_str_lit("--stdout"));
  }
  if (test.output.err.enabled) {
    sp_ps_config_add_arg(&config, sp_str_lit("--stderr"));
  }

  sp_ps_t ps = sp_ps_create(config);
  SP_ASSERT(ps.pid != 0);

  sp_io_stream_t* in = sp_ps_io_in(&ps);
  sp_io_stream_t* out = sp_ps_io_out(&ps);
  sp_io_stream_t* err = sp_ps_io_err(&ps);

  if (!sp_str_empty(test.input)) {
    u64 bytes_written = sp_io_write(in, test.input.data, test.input.len);
    SP_ASSERT_FMT(
      bytes_written == test.input.len,
      "stdin: tried to write {} ({}), but {:fg yellow} returned {}",
      SP_FMT_STR(test.input),
      SP_FMT_U32(test.input.len),
      SP_FMT_CSTR("sp_io_write()"),
      SP_FMT_U64(bytes_written)
    );
    sp_io_close(in);
  }

  if (!sp_str_empty(test.output.out.expected)) {
    sp_test_proc_stream_context_t check = {
      .stream = out,
      .expected = test.output.out.expected,
      .buffer = { .len = 1024, .data = (u8*)sp_alloc(1024) },
      .mode = SP_TEST_PROC_READ_EXACT,
      .expected_len = test.output.out.expected.len,
    };
    sp_test_proc_check_stream(&check);

    SP_ASSERT_FMT(
      check.result == SP_TEST_PS_OUTPUT_MATCH,
      "stdout: expected '{}' but got '{}'",
      SP_FMT_STR(check.expected),
      SP_FMT_CSTR((c8*)check.buffer.data)
    );
  }

  if (!sp_str_empty(test.output.err.expected)) {
    sp_test_proc_stream_context_t check = {
      .stream = err,
      .expected = test.output.err.expected,
      .buffer = { .len = 1024, .data = (u8*)sp_alloc(1024) },
      .mode = SP_TEST_PROC_READ_EXACT,
      .expected_len = test.output.err.expected.len,
    };
    sp_test_proc_check_stream(&check);

    SP_ASSERT_FMT(
      check.result == SP_TEST_PS_OUTPUT_MATCH,
      "stderr: expected '{}' but got '{}'",
      SP_FMT_STR(check.expected),
      SP_FMT_CSTR((c8*)check.buffer.data)
    );
  }
}

// SP_PS_IO_MODE_CREATE
UTEST_F(sp_ps, io_create_create_null) {
  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
    .input = sp_test_ps_canary,
    .output.out = { .expected = sp_test_ps_canary, .enabled = true },
    .fn = TEST_PROC_FUNCTION_ECHO,
  });
}

UTEST_F(sp_ps, io_create_null_create) {
  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_CREATE },
    },
    .input = sp_test_ps_canary,
    .output.err = { .expected = sp_test_ps_canary, .enabled = true },
    .fn = TEST_PROC_FUNCTION_ECHO,
  });
}

UTEST_F(sp_ps, io_null_create_null) {
  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
    .output.out = { .expected = sp_test_ps_canary, .enabled = true },
    .fn = TEST_PROC_FUNCTION_PRINT,
  });
}

UTEST_F(sp_ps, io_null_null_create) {
  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_CREATE },
    },
    .output.err = { .expected = sp_test_ps_canary, .enabled = true },
    .fn = TEST_PROC_FUNCTION_PRINT,
  });
}

UTEST_F(sp_ps, io_stdout_stderr) {
  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_CREATE },
    },
    .output.out = { .expected = sp_test_ps_canary, .enabled = true },
    .output.err = { .expected = sp_test_ps_canary, .enabled = true },
    .fn = TEST_PROC_FUNCTION_PRINT,
  });
}

// SP_PS_IO_MODE_EXISTING
UTEST_F(sp_ps, io_create_file_null) {
  sp_str_t file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("stdout.file"));
  sp_io_stream_t io = sp_io_from_file(file_path, SP_IO_MODE_READ | SP_IO_MODE_APPEND);

  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_EXISTING, .stream = io },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
    .input = sp_test_ps_canary,
    .output = {
      .out = {
        .enabled = true
      },
    },
    .fn = TEST_PROC_FUNCTION_ECHO,
  });

  sp_os_sleep_ms(100); // @spader @sp_ps_wait

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  u64 bytes_read = sp_io_read(&io, ut.buffer.data, ut.buffer.len);
  SP_ASSERT(bytes_read == sp_test_ps_canary.len);
  SP_ASSERT(sp_os_is_memory_equal(ut.buffer.data, sp_test_ps_canary.data, sp_test_ps_canary.len));

  sp_io_close(&io);
}

UTEST_F(sp_ps, io_file_create_null) {
  sp_str_t file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("stdin.file"));

  sp_io_stream_t io = sp_io_from_file(file_path, SP_IO_MODE_WRITE);
  sp_io_write_str(&io, sp_test_ps_canary);
  sp_io_close(&io);
  io = sp_io_from_file(file_path, SP_IO_MODE_READ);

  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_EXISTING, .stream = io },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
    .output = {
      .out = {
        .expected = sp_test_ps_canary,
        .enabled = true
      },
    },
    .fn = TEST_PROC_FUNCTION_ECHO,
  });

  sp_io_close(&io);
}

UTEST_F(sp_ps, io_create_null_file) {
  sp_str_t file_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("stderr.file"));
  sp_io_stream_t io = sp_io_from_file(file_path, SP_IO_MODE_READ | SP_IO_MODE_APPEND);

  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_EXISTING, .stream = io },
    },
    .input = sp_test_ps_canary,
    .output = {
      .err = {
        .enabled = true
      },
    },
    .fn = TEST_PROC_FUNCTION_ECHO,
  });

  sp_os_sleep_ms(100); // @spader @sp_ps_wait

  sp_io_seek(&io, 0, SP_IO_SEEK_SET);

  u64 bytes_read = sp_io_read(&io, ut.buffer.data, ut.buffer.len);
  SP_ASSERT(bytes_read == sp_test_ps_canary.len);
  SP_ASSERT(sp_os_is_memory_equal(ut.buffer.data, sp_test_ps_canary.data, sp_test_ps_canary.len));

  sp_io_close(&io);
}

UTEST_F(sp_ps, io_file_null_file) {
  sp_str_t in_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("stdin.file"));

  sp_io_stream_t in = sp_io_from_file(in_path, SP_IO_MODE_WRITE);
  sp_io_write_str(&in, sp_test_ps_canary);
  sp_io_close(&in);
  in = sp_io_from_file(in_path, SP_IO_MODE_READ);

  sp_str_t err_path = sp_test_file_create_empty(&ut.file_manager, sp_str_lit("stderr.file"));
  sp_io_stream_t err = sp_io_from_file(err_path, SP_IO_MODE_READ | SP_IO_MODE_APPEND);

  sp_test_proc_io((sp_test_proc_io_config_t) {
    .io = {
      .in = { .mode = SP_PS_IO_MODE_EXISTING, .stream = in },
      .out = { .mode = SP_PS_IO_MODE_NULL },
      .err = { .mode = SP_PS_IO_MODE_EXISTING, .stream = err },
    },
    .output = {
      .err = {
        .enabled = true
      },
    },
    .fn = TEST_PROC_FUNCTION_ECHO,
  });

  sp_os_sleep_ms(100); // @spader @sp_ps_wait

  sp_io_seek(&err, 0, SP_IO_SEEK_SET);

  u64 bytes_read = sp_io_read(&err, ut.buffer.data, ut.buffer.len);
  SP_ASSERT(bytes_read == sp_test_ps_canary.len);
  SP_ASSERT(sp_os_is_memory_equal(ut.buffer.data, sp_test_ps_canary.data, sp_test_ps_canary.len));

  sp_io_close(&in);
  sp_io_close(&err);
}


/////////////////
// ENVIRONMENT //
/////////////////
typedef struct {
  sp_ps_env_config_t config;
  sp_env_var_t expected [SP_PS_MAX_ENV];
  sp_env_var_t* foo;
} sp_test_proc_env_config_t;

sp_dyn_array(sp_env_var_t) sp_test_parse_env_output(u8* buffer, u32 len) {
  sp_dyn_array(sp_env_var_t) vars = SP_NULLPTR;

  u32 line_start = 0;
  for (u32 i = 0; i <= len; i++) {
    if (i == len || buffer[i] == '\n') {
      if (i > line_start) {
        sp_str_t line = sp_str((c8*)buffer + line_start, i - line_start);

        u32 space_idx = 0;
        bool found_space = false;
        for (u32 j = 0; j < line.len; j++) {
          if (line.data[j] == ' ') {
            space_idx = j;
            found_space = true;
            break;
          }
        }

        sp_env_var_t var = SP_ZERO_INITIALIZE();
        if (found_space) {
          var.key = sp_str(line.data, space_idx);
          var.value = sp_str(line.data + space_idx + 1, line.len - space_idx - 1);
        } else {
          var.key = line;
          var.value = sp_str_lit("");
        }

        sp_dyn_array_push(vars, var);
      }
      line_start = i + 1;
    }
  }

  return vars;
}

void sp_test_proc_env_verify(s32* utest_result, sp_test_proc_env_config_t test) {
  sp_ps_config_t config = {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), SP_CSTR(test_proc_function_to_cstr(TEST_PROC_FUNCTION_PRINT_ENV)),
      sp_str_lit("--stdout")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    },
    .env = test.config
  };

  sp_ps_t ps = sp_ps_create(config);
  SP_ASSERT(ps.pid);

  sp_io_stream_t* in = sp_ps_io_in(&ps);
  sp_io_stream_t* out = sp_ps_io_out(&ps);

  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  for (u32 i = 0; i < 8; i++) {
    if (sp_str_empty(test.expected[i].key)) {
      break;
    }

    sp_str_builder_append(&builder, test.expected[i].key);
    sp_str_builder_append_c8(&builder, '\n');
  }

  sp_io_write(in, builder.buffer.data, builder.buffer.len);
  sp_io_close(in);

  sp_test_proc_stream_context_t ctx = {
    .stream = out,
    .buffer = {
      .data = sp_alloc(1024),
      .len = 1024
    },
    .mode = SP_TEST_PROC_READ_UNTIL_DONE,
  };
  sp_test_proc_collect_stream(&ctx);

  sp_dyn_array(sp_env_var_t) env = sp_test_parse_env_output(ctx.buffer.data, ctx.bytes_read);


  for (u32 i = 0; i < 8; i++) {
    if (sp_str_empty(test.expected[i].key)) break;

    sp_env_var_t expected = test.expected[i];
    bool found = false;

    sp_dyn_array_for(env, j) {
      if (sp_str_equal(env[j].key, expected.key)) {
        found = true;
        SP_EXPECT_STR_EQ(env[j].value, expected.value);
        break;
      }
    }

    EXPECT_TRUE(found);
  }
}

UTEST_F(sp_ps, env_clean) {
  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_CLEAN,
    },
    .expected = {
      { .key = sp_str_lit("jerry"), .value = sp_str_lit("") },
    }
  });

  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_CLEAN,
      .extra = {
        { .key = sp_str_lit("jerry"), .value = sp_str_lit("garcia") },
      }
    },
    .expected = {
      { .key = sp_str_lit("jerry"), .value = sp_str_lit("garcia") },
    }
  });

  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_CLEAN,
      .extra = {
        { .key = sp_str_lit("garcia"), .value = sp_str_lit("john") },
        { .key = sp_str_lit("garcia"), .value = sp_str_lit("jerome") },
      }
    },
    .expected = {
      { .key = sp_str_lit("garcia"), .value = sp_str_lit("jerome") },
    }
  });
}

UTEST_F(sp_ps, env_inherit) {
  setenv("jerry", "garcia", true);

  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_INHERIT,
    },
    .expected = {
      { .key = sp_str_lit("jerry"), .value = sp_str_lit("garcia") },
    }
  });

  unsetenv("jerry");
}

UTEST_F(sp_ps, env_existing) {
  sp_env_t env = SP_ZERO_INITIALIZE();
  sp_env_insert(&env, sp_str_lit("jerry"), sp_str_lit("garcia"));
  sp_env_insert(&env, sp_str_lit("phil"), sp_str_lit("lesh"));
  sp_env_insert(&env, sp_str_lit("bobby"), sp_str_lit("weir"));

  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_EXISTING,
      .env = env
    },
    .expected = {
      { .key = sp_str_lit("jerry"), .value = sp_str_lit("garcia") },
      { .key = sp_str_lit("phil"), .value = sp_str_lit("lesh") },
      { .key = sp_str_lit("bobby"), .value = sp_str_lit("weir") },
    }
  });

  // Anything extra on top of the base env is applied
  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_EXISTING,
      .env = env,
      .extra = {
        { .key = sp_str_lit("billy"), .value = sp_str_lit("kreutzmann") },
      }
    },
    .expected = {
      { .key = sp_str_lit("billy"), .value = sp_str_lit("kreutzmann") },
    }
  });
}

UTEST_F(sp_ps, empty_env_var) {
  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_CLEAN,
      .extra = {
        { .key = sp_str_lit("jerry"), .value = sp_str_lit("") }
      }
    },
    .expected = {
      { .key = sp_str_lit("jerry"), .value = sp_str_lit("") },
    }
  });

  setenv("jerry", "", true);
  EXPECT_STREQ(getenv("jerry"), "");

  sp_test_proc_env_verify(utest_result, (sp_test_proc_env_config_t) {
    .config = {
      .mode = SP_PS_ENV_INHERIT,
    },
    .expected = {
      { .key = sp_str_lit("jerry"), .value = sp_str_lit("") },
    }
  });

  unsetenv("jerry");
}

//////////////////
// SP_PS_WAIT //
//////////////////
UTEST_F(sp_ps, wait_after_process_complete) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("exit_code"),
      sp_str_lit("--exit-code"), sp_str_lit("42")
    },
  });

  sp_os_sleep_ms(100);

  sp_ps_status_t result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 42);
}

UTEST_F(sp_ps, wait_while_process_running) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("wait"),
      sp_str_lit("100")
    },
  });

  sp_ps_status_t result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, sp_test_ps_wait_exit_code);
}

UTEST_F(sp_ps, run) {
  sp_ps_output_t result = sp_ps_run((sp_ps_config_t) {
    .command = SP_LIT("git"),
    .args = {
      SP_LIT("-C"), ut.file_manager.paths.root,
      SP_LIT("rev-parse"),
      SP_LIT("origin/HEAD"),
    }
  });
  EXPECT_NE(result.out.len, 0);
}

UTEST_F(sp_ps, poll_while_process_running) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("wait"),
      sp_str_lit("100")
    },
  });

  sp_ps_status_t result = sp_ps_poll(&ps, 0);
  EXPECT_EQ(result.state, SP_PS_STATE_RUNNING);

  result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
}

UTEST_F(sp_ps, process_complete_during_poll) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("wait"),
      sp_str_lit("100")
    },
  });

  sp_ps_status_t result = sp_ps_poll(&ps, 200);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, sp_test_ps_wait_exit_code);
}

UTEST_F(sp_ps, poll_after_process_complete) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("exit_code"),
      sp_str_lit("--exit-code"), sp_str_lit("72")
    },
  });

  sp_os_sleep_ms(100);

  sp_ps_status_t result = sp_ps_poll(&ps, 0);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 72);
}

UTEST_F(sp_ps, poll_with_timeout_after_process_complete) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("exit_code"),
      sp_str_lit("--exit-code"), sp_str_lit("72")
    },
  });

  sp_os_sleep_ms(100);

  sp_ps_status_t result = sp_ps_poll(&ps, 100);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 72);
}

UTEST_F(sp_ps, wait_twice_while_process_running) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("exit_code"),
      sp_str_lit("--exit-code"), sp_str_lit("72")
    },
  });

  sp_ps_status_t result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 72);

  result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, -1);
}

UTEST_F(sp_ps, poll_then_wait) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("wait"),
      sp_str_lit("100")
    },
  });

  sp_ps_status_t result = sp_ps_poll(&ps, 0);
  EXPECT_EQ(result.state, SP_PS_STATE_RUNNING);

  result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, sp_test_ps_wait_exit_code);
}

UTEST_F(sp_ps, poll_multiple) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("wait"),
      sp_str_lit("300")
    },
  });

  sp_ps_status_t result = SP_ZERO_INITIALIZE();

  result = sp_ps_poll(&ps, 50);
  EXPECT_EQ(result.state, SP_PS_STATE_RUNNING);

  result = sp_ps_poll(&ps, 50);
  EXPECT_EQ(result.state, SP_PS_STATE_RUNNING);

  result = sp_ps_poll(&ps, 300);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, sp_test_ps_wait_exit_code);
}

UTEST_F(sp_ps, wait_with_output) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("print"),
      sp_str_lit("--stdout")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  sp_ps_status_t result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 0);

  u64 bytes_read = sp_io_read(sp_ps_io_out(&ps), ut.buffer.data, ut.buffer.len);
  EXPECT_EQ(bytes_read, sp_test_ps_canary.len);
  EXPECT_TRUE(sp_os_is_memory_equal(ut.buffer.data, sp_test_ps_canary.data, sp_test_ps_canary.len));
}

UTEST_F(sp_ps, poll_with_io) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("wait"),
      sp_str_lit("100")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  sp_ps_status_t r1 = sp_ps_poll(&ps, 10);
  EXPECT_EQ(r1.state, SP_PS_STATE_RUNNING);

  sp_io_stream_t* in = sp_ps_io_in(&ps);
  EXPECT_NE(in, SP_NULLPTR);

  sp_ps_status_t r2 = sp_ps_wait(&ps);
  EXPECT_EQ(r2.state, SP_PS_STATE_DONE);
}

UTEST_F(sp_ps, interleaved_read_write) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("echo_line"),
      sp_str_lit("--stdout")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  sp_io_stream_t* in = sp_ps_io_in(&ps);
  sp_io_stream_t* out = sp_ps_io_out(&ps);

  EXPECT_NE(in, SP_NULLPTR);
  EXPECT_NE(out, SP_NULLPTR);

  for (u32 i = 0; i < 4; i++) {
    sp_str_t input = sp_format("line {}\n", SP_FMT_U32(i));

    u64 written = sp_io_write_str(in, input);
    EXPECT_EQ(written, input.len);

    sp_os_sleep_ms(50);
    u64 bytes_read = sp_io_read(out, ut.buffer.data, ut.buffer.len);
    sp_str_t expected = sp_format("echo: line {}\n", SP_FMT_U32(i));
    EXPECT_EQ(bytes_read, expected.len);
    EXPECT_TRUE(sp_os_is_memory_equal(ut.buffer.data, expected.data, expected.len));
  }

  sp_io_close(in);

  sp_ps_status_t result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 0);
}

UTEST_F(sp_ps, incremental_nonblocking_read) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("slow_write"),
      sp_str_lit("--stdout")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  sp_io_stream_t* out = sp_ps_io_out(&ps);
  EXPECT_NE(out, SP_NULLPTR);

  const u32 expected_size = 100;
  u8 accumulated[expected_size];
  u32 total_read = 0;

  while (total_read < expected_size) {
    u64 n = sp_io_read(out, accumulated + total_read, expected_size - total_read);
    if (n > 0) {
      total_read += n;
    } else {
      sp_ps_status_t result = sp_ps_poll(&ps, 10);
      if (result.state == SP_PS_STATE_DONE) break;
    }
  }

  EXPECT_EQ(total_read, expected_size);

  for (u32 i = 0; i < total_read; i++) {
    u32 chunk_pos = i % 10;
    u8 expected_byte = (u8)('A' + (chunk_pos % 26));
    EXPECT_EQ(accumulated[i], expected_byte);
  }

  sp_ps_status_t result = sp_ps_wait(&ps);
  EXPECT_EQ(result.state, SP_PS_STATE_DONE);
  EXPECT_EQ(result.exit_code, 0);
}

UTEST_F(sp_ps, output) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("print"),
      sp_str_lit("--stdout")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  sp_ps_output_t output = sp_ps_output(&ps);

  EXPECT_EQ(output.status.state, SP_PS_STATE_DONE);
  EXPECT_EQ(output.status.exit_code, 0);
  EXPECT_TRUE(sp_str_equal(output.out, sp_test_ps_canary));
  EXPECT_TRUE(sp_str_empty(output.err));
}

UTEST_F(sp_ps, write_1mb_to_stdin) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("consume"),
      sp_str_lit("--stdout"),
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_CREATE },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_NULL },
    }
  });

  const u32 num_bytes = 1024 * 1024;
  u8* buffer = (u8*)sp_alloc(num_bytes);
  for (u32 i = 0; i < num_bytes; i++) {
    buffer[i] = (u8)('A' + (i % 26));
  }

  sp_io_stream_t* in = sp_ps_io_in(&ps);
  u64 num_written = sp_io_write(in, buffer, num_bytes);
  EXPECT_EQ(num_written, num_bytes);
  sp_io_close(in);

  sp_os_sleep_ms(100);

  sp_ps_output_t output = sp_ps_output(&ps);

  EXPECT_EQ(output.status.state, SP_PS_STATE_DONE);
  EXPECT_EQ(output.status.exit_code, 0);

  sp_str_t expected = sp_format("{}\n", SP_FMT_U64(num_bytes));
  EXPECT_TRUE(sp_str_equal(output.out, expected));
}

UTEST_F(sp_ps, redirect_stderr_to_stdout) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("print"),
      sp_str_lit("--stdout"),
      sp_str_lit("--stderr")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_CREATE },
      .err = { .mode = SP_PS_IO_MODE_REDIRECT },
    }
  });

  sp_ps_output_t output = sp_ps_output(&ps);

  EXPECT_EQ(output.status.state, SP_PS_STATE_DONE);
  EXPECT_EQ(output.status.exit_code, 0);

  sp_str_t expected = sp_format("{}{}", SP_FMT_STR(sp_test_ps_canary), SP_FMT_STR(sp_test_ps_canary));
  EXPECT_TRUE(sp_str_equal(output.out, expected));
  EXPECT_TRUE(sp_str_empty(output.err));

  sp_io_stream_t* err = sp_ps_io_err(&ps);
  EXPECT_EQ(err, SP_NULLPTR);
}

UTEST_F(sp_ps, redirect_stdout_to_stderr) {
  sp_ps_t ps = sp_ps_create((sp_ps_config_t) {
    .command = SP_LIT("./build/debug/process"),
    .args = {
      sp_str_lit("--fn"), sp_str_lit("print"),
      sp_str_lit("--stdout"),
      sp_str_lit("--stderr")
    },
    .io = {
      .in = { .mode = SP_PS_IO_MODE_NULL },
      .out = { .mode = SP_PS_IO_MODE_REDIRECT },
      .err = { .mode = SP_PS_IO_MODE_CREATE },
    }
  });

  sp_ps_output_t output = sp_ps_output(&ps);

  EXPECT_EQ(output.status.state, SP_PS_STATE_DONE);
  EXPECT_EQ(output.status.exit_code, 0);

  sp_str_t expected = sp_format("{}{}", SP_FMT_STR(sp_test_ps_canary), SP_FMT_STR(sp_test_ps_canary));
  EXPECT_TRUE(sp_str_empty(output.out));
  EXPECT_TRUE(sp_str_equal(output.err, expected));

  sp_io_stream_t* out = sp_ps_io_out(&ps);
  EXPECT_EQ(out, SP_NULLPTR);
}

UTEST_MAIN()
