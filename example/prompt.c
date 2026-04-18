#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_prompt.h"

typedef s32 (*sp_prompt_demo_fn_t)(sp_prompt_ctx_t* ctx);

typedef struct {
  sp_str_t name;
  sp_prompt_demo_fn_t run;
} sp_prompt_demo_t;

static sp_str_t concat_selected(sp_prompt_select_option_t* options, u32 option_count) {
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

static s32 sp_prompt_demo_note(sp_prompt_ctx_t* ctx) {
  sp_str_t value = SP_LIT("hey");
  sp_prompt_intro(ctx, "prompt harness");
  sp_prompt_note(ctx, sp_str_to_cstr(value), "Read file");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_indicators(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "message demo");
  sp_prompt_info(ctx, "blue info line");
  sp_prompt_success(ctx, "green success line");
  sp_prompt_warn(ctx, "yellow warning line");
  sp_prompt_error(ctx, "red error line");
  sp_prompt_cancel(ctx, "red cancel line");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_confirm(sp_prompt_ctx_t* ctx) {
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

static s32 sp_prompt_demo_password(sp_prompt_ctx_t* ctx) {
  const c8* secret = sp_prompt_password(ctx, "Password", "sekret");
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, secret, "Captured");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_select(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "sp_prompt.h widget: select");

  sp_prompt_select_option_t options[] = {
    { .label = "hey", .hint = "recommended" },
    { .label = "hello", .selected = true },
    { .label = "howdy", .hint = "ropers only" },
    { .label = "hi" },
    { .label = "hullo", .hint = "questionable" },
    { .label = "whatup" },
  };

  sp_prompt_select(ctx, (sp_prompt_select_t) {
    .prompt = "Pick a greeting",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 4,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "You say nothing at all");
    return 1;
  }

  sp_prompt_note(ctx, sp_prompt_get_str(ctx), "Greeting");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_select_filter(sp_prompt_ctx_t* ctx) {
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

static s32 sp_prompt_demo_multiselect(sp_prompt_ctx_t* ctx) {
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

  sp_str_t selected = concat_selected(options, sp_carr_len(options));
  sp_prompt_note(ctx, sp_str_to_cstr(selected), "Selected tools");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_multiselect_filter(sp_prompt_ctx_t* ctx) {
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

  sp_str_t selected = concat_selected(options, sp_carr_len(options));
  sp_prompt_note(ctx, sp_str_to_cstr(selected), "Selected tools");
  sp_prompt_outro(ctx, "done");
  return 0;
}

s32 prompt_main(s32 argc, const c8** argv) {
  sp_prompt_demo_fn_t run = SP_NULLPTR;

  sp_cstr_ht(sp_prompt_demo_fn_t) demos = sp_zero();
  sp_cstr_ht_insert(demos, "intro-note", sp_prompt_demo_note);
  sp_cstr_ht_insert(demos, "messages", sp_prompt_demo_indicators);
  sp_cstr_ht_insert(demos, "confirm", sp_prompt_demo_confirm);
  sp_cstr_ht_insert(demos, "password", sp_prompt_demo_password);
  sp_cstr_ht_insert(demos, "select", sp_prompt_demo_select);
  sp_cstr_ht_insert(demos, "select-filter", sp_prompt_demo_select_filter);
  sp_cstr_ht_insert(demos, "multiselect", sp_prompt_demo_multiselect);
  sp_cstr_ht_insert(demos, "multiselect-filter", sp_prompt_demo_multiselect_filter);

  if (argc >= 2) {
    sp_prompt_demo_fn_t* fn = sp_cstr_ht_get(demos, argv[1]);
    if (!fn) {
      sp_log("usage: prompt [program]");
      sp_log("programs:");
      sp_cstr_ht_for_kv(demos, it) {
        sp_log("  {}", sp_fmt_cstr(*it.key));
      }

      return SP_PROMPT_ERROR;
    }
    run = *fn;
  }

  sp_prompt_ctx_t* ctx = sp_prompt_begin();
  if (!ctx) {
    return SP_PROMPT_ERROR;
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

    if (!sp_prompt_select(ctx, (sp_prompt_select_t) {
      .prompt = "Pick demo",
      .options = options,
      .num_options = sp_da_size(options),
    })) {
      sp_prompt_cancel(ctx, "cancelled");
      sp_prompt_end(ctx);
      return SP_PROMPT_ERROR;
    }

    sp_prompt_demo_fn_t* fn = sp_cstr_ht_get(demos, sp_prompt_get_str(ctx));
    result = (*fn)(ctx);
  } else {
    result = run(ctx);
  }

done:
  sp_prompt_end(ctx);
  return result;
}
SP_MAIN(prompt_main)
