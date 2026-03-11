// #define SP_FREESTANDING
// #define SP_BUILTIN
#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_PROMPT_IMPLEMENTATION
#include "sp/prompt.h"

typedef s32 (*sp_clack_demo_fn_t)(sp_prompt_ctx_t* ctx);

typedef struct {
  sp_str_t name;
  sp_clack_demo_fn_t run;
} sp_clack_demo_t;

// @spader use sp_str_reduce with a custom kernel
static sp_str_t sp_clack_selected_labels(sp_prompt_select_option_t* options, u32 option_count) {
  sp_str_builder_t builder = SP_ZERO_INITIALIZE();
  bool first = true;
  sp_for(it, option_count) {
    if (!options[it].selected) {
      continue;
    }

    if (!first) {
      sp_str_builder_append(&builder, SP_LIT(", "));
    }
    first = false;
    sp_str_builder_append(&builder, sp_str_view(options[it].label));
  }

  return sp_str_builder_to_str(&builder);
}

static s32 sp_clack_demo_intro_note(sp_prompt_ctx_t* ctx) {
  sp_str_t value = SP_LIT("hey");
  sp_prompt_intro(ctx, "prompt harness");
  sp_prompt_note(ctx, sp_str_to_cstr(value), "Read file");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_messages(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "message demo");
  sp_prompt_info(ctx, "blue info line");
  sp_prompt_success(ctx, "green success line");
  sp_prompt_warn(ctx, "yellow warning line");
  sp_prompt_error(ctx, "red error line");
  sp_prompt_cancel(ctx, "red cancel line");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_confirm(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "confirm demo");
  bool confirmed = sp_prompt_confirm(ctx, "Install dependencies?", false);

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, confirmed ? "yes" : "no", "Selected");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_password(sp_prompt_ctx_t* ctx) {
  const c8* secret = sp_prompt_password(ctx, "Password", "sekret");
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, secret, "Captured");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_select(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "select demo");

  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript", .hint = "recommended" },
    { .label = "JavaScript", .selected = true },
    { .label = "Rust", .hint = "fast" },
    { .label = "Go" },
    { .label = "Python" },
    { .label = "C" },
  };

  sp_prompt_select(ctx, (sp_prompt_select_t) {
    .prompt = "Pick language",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 4,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, sp_prompt_get_str(ctx), "Selected language");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_select_filter(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "select filter demo");

  sp_prompt_select_option_t options[] = {
    { .label = "TypeScript" },
    { .label = "JavaScript" },
    { .label = "Rust" },
    { .label = "Go" },
    { .label = "Python" },
    { .label = "C" },
  };

  sp_prompt_select(ctx, (sp_prompt_select_t) {
    .prompt = "Pick language",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 4,
    .filter = true,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, sp_prompt_get_str(ctx), "Selected language");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_multiselect(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "multiselect demo");

  sp_prompt_select_option_t options[] = {
    { .label = "ESLint", .hint = "lint" },
    { .label = "Prettier", .hint = "format" },
    { .label = "TypeScript", .selected = true },
    { .label = "Vitest", .selected = true },
    { .label = "Biome" },
  };

  sp_prompt_multiselect(ctx, (sp_prompt_multiselect_t) {
    .prompt = "Pick tools",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 4,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_str_t selected = sp_clack_selected_labels(options, sp_carr_len(options));
  sp_prompt_note(ctx, sp_str_to_cstr(selected), "Selected tools");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_clack_demo_multiselect_filter(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "multiselect filter demo");

  sp_prompt_select_option_t options[] = {
    { .label = "ESLint" },
    { .label = "Prettier" },
    { .label = "TypeScript" },
    { .label = "Vitest" },
    { .label = "Biome" },
  };

  sp_prompt_multiselect(ctx, (sp_prompt_multiselect_t) {
    .prompt = "Pick tools",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 4,
    .filter = true,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_str_t selected = sp_clack_selected_labels(options, sp_carr_len(options));
  sp_prompt_note(ctx, sp_str_to_cstr(selected), "Selected tools");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static sp_clack_demo_t sp_clack_demos[] = {
  { .name = SP_LIT("intro-note"), .run = sp_clack_demo_intro_note },
  { .name = SP_LIT("messages"), .run = sp_clack_demo_messages },
  { .name = SP_LIT("confirm"), .run = sp_clack_demo_confirm },
  { .name = SP_LIT("password"), .run = sp_clack_demo_password },
  { .name = SP_LIT("select"), .run = sp_clack_demo_select },
  { .name = SP_LIT("select-filter"), .run = sp_clack_demo_select_filter },
  { .name = SP_LIT("multiselect"), .run = sp_clack_demo_multiselect },
  { .name = SP_LIT("multiselect-filter"), .run = sp_clack_demo_multiselect_filter },
};

static sp_clack_demo_fn_t sp_clack_demo_find(sp_str_t name) {
  sp_carr_for(sp_clack_demos, it) {
    if (sp_str_equal(name, sp_clack_demos[it].name)) {
      return sp_clack_demos[it].run;
    }
  }

  return SP_NULLPTR;
}

static s32 sp_clack_demo_picker(sp_prompt_ctx_t* ctx) {
  sp_prompt_select_option_t options[sp_carr_len(sp_clack_demos)] = {0};

  sp_carr_for(sp_clack_demos, it) {
    options[it] = (sp_prompt_select_option_t) {
      .label = sp_clack_demos[it].name.data,
      .selected = it == 0,
    };
  }

  sp_prompt_select(ctx, (sp_prompt_select_t) {
    .prompt = "Pick demo",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 8,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_clack_demo_fn_t run = sp_clack_demo_find(sp_str_view(sp_prompt_get_str(ctx)));
  if (run == SP_NULLPTR) {
    sp_prompt_error(ctx, "unknown demo");
    return 1;
  }

  return run(ctx);
}

static void sp_clack_usage(void) {
  SP_LOG("usage: demo-prompt [program]");
  SP_LOG("programs:");
  sp_carr_for(sp_clack_demos, it) {
    SP_LOG("  {}", SP_FMT_STR(sp_clack_demos[it].name));
  }
}

s32 prompt_main(s32 argc, const c8** argv);
SP_ENTRY(prompt_main)

s32 prompt_main(s32 argc, const c8** argv) {
#ifdef SP_FREESTANDING
  sp_sys_init(); // @spader need to fix SP_ENTRY to do this
#endif
  sp_clack_demo_fn_t run = SP_NULLPTR;

  if (argc >= 2) {
    run = sp_clack_demo_find(sp_str_view(argv[1]));
    if (run == SP_NULLPTR) {
      sp_clack_usage();
      return 1;
    }
  }

  sp_prompt_ctx_t* ctx = sp_prompt_begin();
  if (ctx == SP_NULLPTR) {
    return 1;
  }

  s32 result = 0;
  if (run == SP_NULLPTR) {
    result = sp_clack_demo_picker(ctx);
  } else {
    result = run(ctx);
  }

  sp_prompt_end(ctx);
  return result;
}
