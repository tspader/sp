#include "sp.h"
#include "test.h"
#include "utest.h"

typedef struct linkage {
  sp_test_file_manager_t files;
  sp_str_t root;
  sp_str_t source;
} linkage;

UTEST_F_SETUP(linkage) {
  sp_test_file_manager_init(&ut.files);

  ut.root = sp_fs_get_exe_path();
  while (!sp_str_equal(sp_fs_get_name(ut.root), sp_str_lit("build"))) {
    ut.root = sp_fs_parent_path(ut.root);
  }

  ut.root = sp_fs_parent_path(ut.root);
  ut.source = sp_fs_join_path(ut.root, sp_str_lit("test/tools/linkage"));
}

UTEST_F_TEARDOWN(linkage) {
  sp_test_file_manager_cleanup(&ut.files);
}

bool compile_to_exe(linkage* ctx, const c8* file, sp_str_t output) {
  sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
    .command = sp_str_lit("cc"),
    .args = {
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_format("-I{}", SP_FMT_STR(ctx->root)),
      sp_str_lit("-o"), output,
      sp_str_lit("-g"),
      sp_str_lit("-lpthread"), sp_str_lit("-lm")
    },
  });

  return !out.status.exit_code;
}

bool compile_to_object(linkage* ctx, const c8* file, sp_str_t output) {
  sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
    .command = sp_str_lit("cc"),
    .args = {
      sp_str_lit("-c"),
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_format("-I{}", SP_FMT_STR(ctx->root)),
      sp_str_lit("-o"), output,
      sp_str_lit("-g"),
      sp_str_lit("-lpthread"), sp_str_lit("-lm")
    },
  });

  return !out.status.exit_code;
}

bool compile_objects_to_exe(sp_str_t output, sp_str_t* objs, u32 len) {
  sp_ps_config_t cfg = {
    .command = sp_str_lit("cc"),
    .args = { sp_str_lit("-o"), output, sp_str_lit("-lpthread"), sp_str_lit("-lm") },
  };

  for (u32 it = 0; it < len; it++) {
    sp_ps_config_add_arg(&cfg, objs[it]);
  }

  sp_ps_output_t out = sp_ps_run(cfg);
  return !out.status.exit_code;
}

bool compile_to_linked_exe(linkage* ctx, const c8* file, sp_str_t output, sp_str_t* libs, u32 len) {
  sp_ps_config_t cfg = {
    .command = sp_str_lit("cc"),
    .args = {
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_format("-I{}", SP_FMT_STR(ctx->root)),
      sp_str_lit("-o"), output,
      sp_str_lit("-g"),
      sp_str_lit("-lpthread"), sp_str_lit("-lm")
    },
  };

  for (u32 it = 0; it < len; it++) {
    sp_ps_config_add_arg(&cfg, libs[it]);
  }

  sp_ps_output_t out = sp_ps_run(cfg);
  return !out.status.exit_code;
}

bool create_archive(linkage* ctx, sp_str_t archive, sp_str_t* objs, u32 len) {
  sp_ps_config_t cfg = {
    .command = sp_str_lit("ar"),
    .args = {
      sp_str_lit("rcs")
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
    .command = sp_str_lit("nm"),
    .args = { sp_str_lit("-g"), binary },
  });

  return !nm.status.exit_code && sp_str_contains(nm.out, symbol);
}

UTEST_F(linkage, single_tu) {
  sp_str_t bin = sp_test_file_path(&ut.files, sp_str_lit("header-single"));
  EXPECT_TRUE(compile_to_exe(&ut, "main.c", bin));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  ASSERT_EQ(run.status.exit_code, 0);
}

UTEST_F(linkage, multi_tu) {
  typedef struct {
    const c8* file;
    sp_str_t output;
  } target_t;

  target_t targets[] = {
    { "lib-decl.c", sp_test_file_path(&ut.files, sp_str_lit("multi-decl.o")) },
    { "main-impl.c", sp_test_file_path(&ut.files, sp_str_lit("multi-impl.o")) },
  };
  sp_carr_for(targets, it) {
    EXPECT_TRUE(compile_to_object(&ut, targets[it].file, targets[it].output));
  }

  sp_str_t objs[] = { targets[0].output, targets[1].output };
  sp_str_t bin = sp_test_file_path(&ut.files, sp_str_lit("header-multi"));
  EXPECT_TRUE(compile_objects_to_exe(bin, objs, sp_carr_len(objs)));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  EXPECT_EQ(run.status.exit_code, 0);
}

UTEST_F(linkage, shared_lib) {
  sp_str_t so = sp_test_file_path(&ut.files, sp_str_lit("shared.so"));

  sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("cc"),
    .args = {
      sp_str_lit("-shared"), sp_str_lit("-fPIC"),
      sp_fs_join_path(ut.source, sp_str_lit("lib-impl.c")),
      sp_format("-I{}", SP_FMT_STR(ut.root)),
      sp_str_lit("-o"), so,
      sp_str_lit("-lpthread"), sp_str_lit("-lm")
    },
  });
  ASSERT_EQ(out.status.exit_code, 0);
  EXPECT_TRUE(is_symbol_in_binary(so, sp_str_lit("sp_alloc")));
}

UTEST_F(linkage, static_lib) {
  sp_str_t obj = sp_test_file_path(&ut.files, sp_str_lit("sp.o"));
  EXPECT_TRUE(compile_to_object(&ut, "lib-impl.c", obj));

  sp_str_t archive = sp_test_file_path(&ut.files, sp_str_lit("static.a"));
  EXPECT_TRUE(create_archive(&ut, archive, &obj, 1));

  sp_str_t bin = sp_test_file_path(&ut.files, sp_str_lit("static-single"));
  EXPECT_TRUE(compile_to_linked_exe(&ut, "main-decl.c", bin, &archive, 1));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  ASSERT_EQ(run.status.exit_code, 0);
}

UTEST_F(linkage, cpp_compat) {
  sp_str_t obj = sp_test_file_path(&ut.files, sp_str_lit("cpp-main.o"));

  sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
    .command = sp_str_lit("c++"),
    .args = {
      sp_str_lit("-c"),
      sp_str_lit("-x"), sp_str_lit("c++"),
      sp_fs_join_path(ut.source, sp_str_lit("main.c")),
      sp_format("-I{}", SP_FMT_STR(ut.root)),
      sp_str_lit("-o"), obj,
      sp_str_lit("-g"),
      sp_str_lit("-Werror"),
    },
  });

  EXPECT_EQ(out.status.exit_code, 0);
}

UTEST_F(linkage, format_bare_args_should_fail) {
  // sp_str_t bin = sp_test_file_path(&ut.files, sp_str_lit("format-bare-args"));
  // bool compiled = compile_to_exe(&ut, "format-bare-args.c", bin);
  // EXPECT_FALSE(compiled);
}

SP_TEST_MAIN()
