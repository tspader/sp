#if defined(BUILD_FREESTANDING_EXAMPLE)
  #define SP_FREESTANDING
  #define SP_DEFINE_BUILTINS
#endif
#define SP_IMPLEMENTATION
#include "sp.h"

#define SP_PROMPT_IMPLEMENTATION
#include "sp/sp_prompt.h"

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

s32 prompt_main(s32 argc, const c8** argv);
SP_ENTRY(prompt_main)

s32 prompt_main(s32 argc, const c8** argv) {
#ifdef SP_FREESTANDING
  sp_sys_init(); // @spader need to fix SP_ENTRY to do this
#endif
  sp_clack_demo_fn_t run = SP_NULLPTR;

  sp_cstr_ht(sp_clack_demo_fn_t) demos = sp_zero();

  sp_cstr_ht_insert(demos, ("intro-note"), sp_clack_demo_intro_note);
  sp_cstr_ht_insert(demos, ("messages"), sp_clack_demo_messages);
  sp_cstr_ht_insert(demos, ("confirm"), sp_clack_demo_confirm);
  sp_cstr_ht_insert(demos, ("password"), sp_clack_demo_password);
  sp_cstr_ht_insert(demos, ("select"), sp_clack_demo_select);
  sp_cstr_ht_insert(demos, ("select-filter"), sp_clack_demo_select_filter);
  sp_cstr_ht_insert(demos, ("multiselect"), sp_clack_demo_multiselect);
  sp_cstr_ht_insert(demos, ("multiselect-filter"), sp_clack_demo_multiselect_filter);

  if (argc >= 2) {
    sp_clack_demo_fn_t* fn = sp_cstr_ht_get(demos, argv[1]);
    if (!fn) {
      sp_log("usage: demo-prompt [program]");
      sp_log("programs:");
      sp_cstr_ht_for_kv(demos, it) {
        sp_log("  {}", SP_FMT_CSTR(*it.key));
      }

      return 1;
    }
    run = *fn;
  }

  sp_prompt_ctx_t* ctx = sp_prompt_begin();
  if (ctx == SP_NULLPTR) {
    return 1;
  }

  s32 result = 0;
  if (run == SP_NULLPTR) {
    sp_da(sp_prompt_select_option_t) options = sp_zero();
    sp_cstr_ht_for_kv(demos, it) {
      sp_prompt_select_option_t option = {
        .label = *it.key,
        .selected = it.idx == 0
      };
      sp_da_push(options, option);
    }

    sp_prompt_select(ctx, (sp_prompt_select_t) {
      .prompt = "Pick demo",
      .options = options,
      .num_options = sp_da_size(options),
      .max_items = 8,
    });

    if (sp_prompt_cancelled(ctx)) {
      sp_prompt_cancel(ctx, "cancelled");
      return 1;
    }

    const c8* selection = sp_prompt_get_str(ctx);
    sp_clack_demo_fn_t* fn = sp_cstr_ht_get(demos, selection);
    if (!fn) {
      sp_prompt_error(ctx, "unknown demo");
      return 1;
    }
    run = *fn;

    result = run(ctx);
  } else {
    result = run(ctx);
  }

  sp_prompt_end(ctx);
  return result;
}
