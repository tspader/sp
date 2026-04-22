#include "sp.h"
#include "test.h"
#include "utest.h"

#if defined(SP_FREESTANDING)
SP_TEST_MAIN()
#else

typedef struct linkage {
  sp_test_file_manager_t files;
  sp_str_t root;
  sp_str_t source;
  sp_opt(bool) has_compiler;
} linkage;

#if defined(SP_WIN32)
bool check_path(sp_str_t program) {
  return SearchPathA(SP_NULLPTR, sp_str_to_cstr(program), SP_NULLPTR, 0, SP_NULLPTR, SP_NULLPTR) > 0;
}
#else
bool check_path(sp_str_t program) {
  return true;
}
#endif

UTEST_F_SETUP(linkage) {
  sp_test_file_manager_init(&ut.files);

  ut.root = ut.files.paths.root;
  ut.source = sp_fs_join_path(ut.root, sp_str_lit("test/tools/linkage"));

  if (!ut.has_compiler.some) {
    sp_opt_set(ut.has_compiler, check_path(sp_str_lit("cl")));
  }

  if (!ut.has_compiler.value) {
    UTEST_SKIP("cl.exe is not on path");
  }
}

UTEST_F_TEARDOWN(linkage) {
  sp_test_file_manager_cleanup(&ut.files);
}

sp_str_t linkage_exe(sp_str_t stem) {
  #if defined(SP_WIN32)
    return sp_fmt("{}.exe", sp_fmt_str(stem));
  #else
    return stem;
  #endif
}

sp_str_t linkage_obj(sp_str_t stem) {
  #if defined(SP_WIN32)
    return sp_fmt("{}.obj", sp_fmt_str(stem));
  #else
    return sp_fmt("{}.o", sp_fmt_str(stem));
  #endif
}

sp_str_t linkage_shared(sp_str_t stem) {
  #if defined(SP_WIN32)
    return sp_fmt("{}.dll", sp_fmt_str(stem));
  #else
    return sp_fmt("{}.so", sp_fmt_str(stem));
  #endif
}

sp_str_t linkage_static(sp_str_t stem) {
  #if defined(SP_WIN32)
    return sp_fmt("{}.lib", sp_fmt_str(stem));
  #else
    return sp_fmt("{}.a", sp_fmt_str(stem));
  #endif
}

void linkage_add_win32_link_libs(sp_ps_config_t* cfg) {
  #if defined(SP_WIN32)
    sp_ps_config_add_arg(cfg, sp_str_lit("/link"));
    sp_ps_config_add_arg(cfg, sp_str_lit("shell32.lib"));
  #else
    SP_UNUSED(cfg);
  #endif
}

bool compile_to_exe(linkage* ctx, const c8* file, sp_str_t output) {
  #if defined(SP_WIN32)
    sp_ps_config_t cfg = {
      .command = sp_str_lit("cl"),
      .args = {
        sp_str_lit("/nologo"),
        sp_str_lit("/TC"),
        sp_fmt("/I{}", sp_fmt_str(ctx->root)),
        sp_fs_join_path(ctx->source, SP_CSTR(file)),
        sp_fmt("/Fe:{}", sp_fmt_str(output)),
      },
    };

    linkage_add_win32_link_libs(&cfg);
    sp_ps_output_t out = sp_ps_run(cfg);

    return !out.status.exit_code;
  #else
  sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
    .command = sp_str_lit("cc"),
    .args = {
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_fmt("-I{}", sp_fmt_str(ctx->root)),
      sp_str_lit("-o"), output,
      sp_str_lit("-g"),
    },
  });

  return !out.status.exit_code;
  #endif
}

bool compile_to_object(linkage* ctx, const c8* file, sp_str_t output) {
  #if defined(SP_WIN32)
    sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
      .command = sp_str_lit("cl"),
      .args = {
        sp_str_lit("/nologo"),
        sp_str_lit("/TC"),
        sp_str_lit("/c"),
        sp_fmt("/I{}", sp_fmt_str(ctx->root)),
        sp_fs_join_path(ctx->source, SP_CSTR(file)),
        sp_fmt("/Fo:{}", sp_fmt_str(output)),
      },
    });

    return !out.status.exit_code;
  #else
  sp_ps_output_t out = sp_ps_run((sp_ps_config_t) {
    .command = sp_str_lit("cc"),
    .args = {
      sp_str_lit("-c"),
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_fmt("-I{}", sp_fmt_str(ctx->root)),
      sp_str_lit("-o"), output,
      sp_str_lit("-g"),
    },
  });

  return !out.status.exit_code;
  #endif
}

bool compile_objects_to_exe(sp_str_t output, sp_str_t* objs, u32 len) {
  #if defined(SP_WIN32)
    sp_ps_config_t cfg = {
      .command = sp_str_lit("cl"),
      .args = { sp_str_lit("/nologo"), sp_fmt("/Fe:{}", sp_fmt_str(output)) },
    };

    sp_for(it, len) {
      sp_ps_config_add_arg(&cfg, objs[it]);
    }

    linkage_add_win32_link_libs(&cfg);

    sp_ps_output_t out = sp_ps_run(cfg);
    return !out.status.exit_code;
  #else
  sp_ps_config_t cfg = {
    .command = sp_str_lit("cc"),
    .args = { sp_str_lit("-o"), output }
  };

  for (u32 it = 0; it < len; it++) {
    sp_ps_config_add_arg(&cfg, objs[it]);
  }

  sp_ps_output_t out = sp_ps_run(cfg);
  return !out.status.exit_code;
  #endif
}

bool compile_to_linked_exe(linkage* ctx, const c8* file, sp_str_t output, sp_str_t* libs, u32 len) {
  #if defined(SP_WIN32)
    sp_ps_config_t cfg = {
      .command = sp_str_lit("cl"),
      .args = {
        sp_str_lit("/nologo"),
        sp_str_lit("/TC"),
        sp_fs_join_path(ctx->source, SP_CSTR(file)),
        sp_fmt("/I{}", sp_fmt_str(ctx->root)),
        sp_fmt("/Fe:{}", sp_fmt_str(output)),
      },
    };

    sp_ps_config_add_arg(&cfg, sp_str_lit("/link"));

    sp_for(it, len) {
      sp_ps_config_add_arg(&cfg, libs[it]);
    }

    sp_ps_config_add_arg(&cfg, sp_str_lit("shell32.lib"));

    sp_ps_output_t out = sp_ps_run(cfg);
    return !out.status.exit_code;
  #else
  sp_ps_config_t cfg = {
    .command = sp_str_lit("cc"),
    .args = {
      sp_fs_join_path(ctx->source, SP_CSTR(file)),
      sp_fmt("-I{}", sp_fmt_str(ctx->root)),
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
  #endif
}

bool create_archive(linkage* ctx, sp_str_t archive, sp_str_t* objs, u32 len) {
  #if defined(SP_WIN32)
    SP_UNUSED(ctx);
    sp_ps_config_t cfg = {
      .command = sp_str_lit("lib"),
      .args = {
        sp_str_lit("/nologo"),
        sp_fmt("/OUT:{}", sp_fmt_str(archive)),
      },
    };

    sp_for(it, len) {
      sp_ps_config_add_arg(&cfg, objs[it]);
    }

    sp_ps_output_t out = sp_ps_run(cfg);
    return !out.status.exit_code;
  #else
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
  #endif
}

bool is_symbol_in_binary(sp_str_t binary, sp_str_t symbol) {
  #if defined(SP_WIN32)
    HMODULE module = LoadLibraryA(sp_str_to_cstr(binary));
    if (!module) {
      return false;
    }

    bool found = GetProcAddress(module, sp_str_to_cstr(symbol)) != SP_NULLPTR;
    FreeLibrary(module);
    return found;
  #else
    sp_ps_output_t nm = sp_ps_run((sp_ps_config_t){
      .command = sp_str_lit("nm"),
    .args = { sp_str_lit("-g"), binary },
  });

  return !nm.status.exit_code && sp_str_contains(nm.out, symbol);
  #endif
}

UTEST_F(linkage, single_tu) {
  sp_str_t bin = linkage_exe(sp_test_file_path(&ut.files, sp_str_lit("header-single")));
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
    { "lib-decl.c", linkage_obj(sp_test_file_path(&ut.files, sp_str_lit("multi-decl"))) },
    { "main-impl.c", linkage_obj(sp_test_file_path(&ut.files, sp_str_lit("multi-impl"))) },
  };
  sp_carr_for(targets, it) {
    EXPECT_TRUE(compile_to_object(&ut, targets[it].file, targets[it].output));
  }

  sp_str_t objs[] = { targets[0].output, targets[1].output };
  sp_str_t bin = linkage_exe(sp_test_file_path(&ut.files, sp_str_lit("header-multi")));
  EXPECT_TRUE(compile_objects_to_exe(bin, objs, sp_carr_len(objs)));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  EXPECT_EQ(run.status.exit_code, 0);
}

UTEST_F(linkage, shared_lib) {
  sp_str_t so = linkage_shared(sp_test_file_path(&ut.files, sp_str_lit("shared")));

  #if defined(SP_WIN32)
    sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
      .command = sp_str_lit("cl"),
      .args = {
        sp_str_lit("/nologo"),
        sp_str_lit("/LD"),
        sp_str_lit("/TC"),
        sp_str_lit("/DSP_SHARED_LIB"),
        sp_fs_join_path(ut.source, sp_str_lit("lib-impl.c")),
        sp_fmt("/I{}", sp_fmt_str(ut.root)),
        sp_fmt("/Fe:{}", sp_fmt_str(so)),
        sp_str_lit("/link"),
        sp_str_lit("shell32.lib"),
      },
    });
  #else
    sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
      .command = sp_str_lit("cc"),
      .args = {
        sp_str_lit("-shared"), sp_str_lit("-fPIC"),
        sp_fs_join_path(ut.source, sp_str_lit("lib-impl.c")),
        sp_fmt("-I{}", sp_fmt_str(ut.root)),
        sp_str_lit("-o"), so,
        sp_str_lit("-lpthread"), sp_str_lit("-lm")
      },
    });
  #endif
  ASSERT_EQ(out.status.exit_code, 0);
  EXPECT_TRUE(is_symbol_in_binary(so, sp_str_lit("sp_alloc")));
}

UTEST_F(linkage, static_lib) {
  sp_str_t obj = linkage_obj(sp_test_file_path(&ut.files, sp_str_lit("sp")));
  EXPECT_TRUE(compile_to_object(&ut, "lib-impl.c", obj));

  sp_str_t archive = linkage_static(sp_test_file_path(&ut.files, sp_str_lit("static")));
  EXPECT_TRUE(create_archive(&ut, archive, &obj, 1));

  sp_str_t bin = linkage_exe(sp_test_file_path(&ut.files, sp_str_lit("static-single")));
  EXPECT_TRUE(compile_to_linked_exe(&ut, "main-decl.c", bin, &archive, 1));

  sp_ps_output_t run = sp_ps_run((sp_ps_config_t){ .command = bin });
  ASSERT_EQ(run.status.exit_code, 0);
}

UTEST_F(linkage, cpp_compat) {
  sp_str_t obj = linkage_obj(sp_test_file_path(&ut.files, sp_str_lit("cpp-main")));

  #if defined(SP_WIN32)
    sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
      .command = sp_str_lit("cl"),
      .args = {
        sp_str_lit("/nologo"),
        sp_str_lit("/TP"),
        sp_str_lit("/std:c++20"),
        sp_str_lit("/c"),
        sp_fs_join_path(ut.source, sp_str_lit("cpp-compat.c")),
        sp_fmt("/I{}", sp_fmt_str(ut.root)),
        sp_fmt("/Fo:{}", sp_fmt_str(obj)),
        sp_str_lit("/WX"),
      },
    });
  #else
    sp_ps_output_t out = sp_ps_run((sp_ps_config_t){
      .command = sp_str_lit("c++"),
      .args = {
        sp_str_lit("-c"),
        sp_str_lit("-x"), sp_str_lit("c++"),
        sp_fs_join_path(ut.source, sp_str_lit("cpp-compat.c")),
        sp_fmt("-I{}", sp_fmt_str(ut.root)),
        sp_str_lit("-o"), obj,
        sp_str_lit("-g"),
        sp_str_lit("-Werror"),
      },
    });
  #endif

  EXPECT_EQ(out.status.exit_code, 0);
}

UTEST_F(linkage, format_bare_args_should_fail) {
  // sp_str_t bin = sp_test_file_path(&ut.files, sp_str_lit("format-bare-args"));
  // bool compiled = compile_to_exe(&ut, "format-bare-args.c", bin);
  // EXPECT_FALSE(compiled);
}

SP_TEST_MAIN()
#endif /* !SP_FREESTANDING */
