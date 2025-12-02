#include "sp.h"

#define SP_TEST_IMPLEMENTATION
#include "test.h"
#include "utest.h"

typedef struct sp_compile {
  sp_test_file_manager_t files;
  sp_str_t root;
  sp_str_t source;
} sp_compile;

UTEST_F_SETUP(sp_compile) {
  sp_test_file_manager_init(&ut.files);

  ut.root = sp_fs_get_exe_path();
  for (s32 i = 0; i < 2; i++) {
    ut.root = sp_fs_parent_path(ut.root);
  }

  ut.source = sp_fs_join_path(ut.root, SP_LIT("test/tools/linkage"));
}

UTEST_F_TEARDOWN(sp_compile) {
  sp_test_file_manager_cleanup(&ut.files);
}

bool compile_to_exe(sp_compile* ctx, const c8* file, sp_str_t output) {
  sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
    .command = SP_LIT("cc"),
    .args = {
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_format("-I{}", SP_FMT_STR(ctx->root)),
      SP_LIT("-o"), output,
      sp_str_lit("-g"),
      SP_LIT("-lpthread")
    },
  });

  return !out.status.exit_code;
}

bool compile_to_object(sp_compile* ctx, const c8* file, sp_str_t output) {
  sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
    .command = SP_LIT("cc"),
    .args = {
      sp_str_lit("-c"),
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_format("-I{}", SP_FMT_STR(ctx->root)),
      sp_str_lit("-o"), output,
      sp_str_lit("-g"),
      sp_str_lit("-lpthread")
    },
  });

  return !out.status.exit_code;
}

bool compile_objects_to_exe(sp_str_t output, sp_str_t* objs, u32 len) {
  sp_ps_config_t cfg = {
    .command = SP_LIT("cc"),
    .args = { SP_LIT("-o"), output, SP_LIT("-lpthread") },
  };

  for (u32 it = 0; it < len; it++) {
    sp_ps_config_add_arg(&cfg, objs[it]);
  }

  sp_ps_output_t out = sp_ps_run(cfg);
  return !out.status.exit_code;
}

bool compile_to_linked_exe(sp_compile* ctx, const c8* file, sp_str_t output, sp_str_t* libs, u32 len) {
  sp_ps_config_t cfg = {
    .command = SP_LIT("cc"),
    .args = {
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_format("-I{}", SP_FMT_STR(ctx->root)),
      SP_LIT("-o"), output,
      SP_LIT("-g"), SP_LIT("-lpthread")
    },
  };

  for (u32 it = 0; it < len; it++) {
    sp_ps_config_add_arg(&cfg, libs[it]);
  }

  sp_ps_output_t out = sp_ps_run(cfg);
  return !out.status.exit_code;
}

bool create_archive(sp_compile* ctx, sp_str_t archive, sp_str_t* objs, u32 len) {
  sp_ps_config_t cfg = {
    .command = SP_LIT("ar"),
    .args = {
      SP_LIT("rcs")
    },
  };

  sp_ps_config_add_arg(&cfg, archive);
  for (u32 it = 0; it < len; it++) {
    sp_ps_config_add_arg(&cfg, objs[it]);
  }

  sp_ps_output_t out = sp_ps_run(cfg);
  return !out.status.exit_code;
}

bool is_symbol_in_binary(sp_str_t binary, sp_str_t symbol) {
  sp_ps_output_t nm = sp_ps_run((sp_ps_config_t){
    .command = SP_LIT("nm"),
    .args = { SP_LIT("-g"), binary },
  });

  return !nm.status.exit_code && sp_str_contains(nm.out, symbol);
}

UTEST_F(sp_compile, single_tu) {
  sp_str_t bin = sp_test_file_path(&ut.files, SP_LIT("header-single"));
  EXPECT_TRUE(compile_to_exe(&ut, "main.c", bin));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  ASSERT_EQ(run.status.exit_code, 0);
}

UTEST_F(sp_compile, multi_tu) {
  typedef struct {
    const c8* file;
    sp_str_t output;
  } target_t;

  target_t targets[] = {
    { "lib-decl.c", sp_test_file_path(&ut.files, SP_LIT("multi-decl.o")) },
    { "main-impl.c", sp_test_file_path(&ut.files, SP_LIT("multi-impl.o")) },
  };
  sp_carr_for(targets, it) {
    EXPECT_TRUE(compile_to_object(&ut, targets[it].file, targets[it].output));
  }

  sp_str_t objs[] = { targets[0].output, targets[1].output };
  sp_str_t bin = sp_test_file_path(&ut.files, SP_LIT("header-multi"));
  EXPECT_TRUE(compile_objects_to_exe(bin, objs, sp_carr_len(objs)));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  EXPECT_EQ(run.status.exit_code, 0);
}

UTEST_F(sp_compile, shared_lib) {
  sp_str_t so = sp_test_file_path(&ut.files, SP_LIT("shared.so"));

  sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
    .command = SP_LIT("cc"),
    .args = {
      SP_LIT("-shared"), SP_LIT("-fPIC"),
      sp_fs_join_path(ut.source, SP_LIT("lib-impl.c")),
      sp_format("-I{}", SP_FMT_STR(ut.root)),
      SP_LIT("-o"), so,
      SP_LIT("-lpthread")
    },
  });
  ASSERT_EQ(out.status.exit_code, 0);
  EXPECT_TRUE(is_symbol_in_binary(so, SP_LIT("sp_alloc")));
}

UTEST_F(sp_compile, static_lib) {
  sp_str_t obj = sp_test_file_path(&ut.files, SP_LIT("sp.o"));
  EXPECT_TRUE(compile_to_object(&ut, "lib-impl.c", obj));

  sp_str_t archive = sp_test_file_path(&ut.files, SP_LIT("static.a"));
  EXPECT_TRUE(create_archive(&ut, archive, &obj, 1));

  sp_str_t bin = sp_test_file_path(&ut.files, SP_LIT("static-single"));
  EXPECT_TRUE(compile_to_linked_exe(&ut, "main-decl.c", bin, &archive, 1));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  ASSERT_EQ(run.status.exit_code, 0);
}

SP_TEST_MAIN()
