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
  sp_prompt_intro(ctx, "Demo: Note");
  sp_prompt_note(ctx, "Wow. We got someone with a nose for *exciting* widgets", "Congratulations");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_indicators(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Indicators");
  sp_prompt_info(ctx, "info");
  sp_prompt_success(ctx, "success");
  sp_prompt_warn(ctx, "warning");
  sp_prompt_error(ctx, "error");
  sp_prompt_cancel(ctx, "cancel");
  sp_prompt_outro(ctx, "outro");
  return 0;
}

static s32 sp_prompt_demo_confirm(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Confirm");
  bool confirmed = sp_prompt_confirm(ctx, "spum?", false);

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, confirmed ? "yes" : "no", "Selected");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_password(sp_prompt_ctx_t* ctx) {
  const c8* secret = sp_prompt_password(ctx, "Demo: Password", "sekret");
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, secret, "Captured");
  sp_prompt_outro(ctx, "done");
  return 0;
}

static s32 sp_prompt_demo_select(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Select");

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
  sp_prompt_intro(ctx, "Demo: Select + Filter");

  sp_prompt_select_option_t options[] = {
    { .label = "foo" },
    { .label = "foobar" },
    { .label = "baz" },
    { .label = "kram" },
    { .label = "spum" },
    { .label = "foospum" },
    { .label = "foobazspum" },
    { .label = "kramspum" },
  };

  sp_prompt_select(ctx, (sp_prompt_select_t) {
    .prompt = "Pick one!",
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
  sp_prompt_intro(ctx, "Demo: Multiselect");

  sp_prompt_select_option_t options[] = {
    { .label = "mimi", .hint = "the original" },
    { .label = "ohno", .hint = "mountain rat", .selected = true },
    { .label = "pigpen", .hint = "peeper", .selected = true },
    { .label = "pearl" },
    { .label = "lina", .hint = "stumpy", .selected = true },
    { .label = "dot", .hint = "princess" },
  };

  sp_prompt_multiselect(ctx, (sp_prompt_multiselect_t) {
    .prompt = "Pick your cats",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 6,
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
  sp_prompt_intro(ctx, "Multiselect + Filter");

  sp_prompt_select_option_t options[] = {
    { .label = "peeper" },
    { .label = "mister p" },
    { .label = "pigpen" },
    { .label = "peepo" },
    { .label = "herbert edward sogood" },
    { .label = "danny" },
    { .label = "pig" },
  };

  sp_prompt_multiselect(ctx, (sp_prompt_multiselect_t) {
    .prompt = "Name the orange cat",
    .options = options,
    .num_options = sp_carr_len(options),
    .max_items = 6,
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

typedef struct {
  const c8* name;
  sp_prompt_demo_fn_t fn;
} demo_t;

s32 prompt_main(s32 argc, const c8** argv) {
  sp_prompt_demo_fn_t run = SP_NULLPTR;

  demo_t ordered [] = {
    { "note", sp_prompt_demo_note },
    { "indicators", sp_prompt_demo_indicators },
    { "confirm", sp_prompt_demo_confirm },
    { "password", sp_prompt_demo_password },
    { "select", sp_prompt_demo_select },
    { "select + filter", sp_prompt_demo_select_filter },
    { "multiselect", sp_prompt_demo_multiselect },
    { "multiselect + filter", sp_prompt_demo_multiselect_filter },
  };

  sp_cstr_ht(sp_prompt_demo_fn_t) demos = sp_zero();
  sp_carr_for(ordered, it) {
    sp_cstr_ht_insert(demos, ordered[it].name, ordered[it].fn);
  }

  if (argc >= 2) {
    sp_prompt_demo_fn_t* fn = sp_cstr_ht_get(demos, argv[1]);
    if (!fn) {
      sp_log("usage: prompt [program]");
      sp_log("programs:");
      sp_carr_for(ordered, it) {
        sp_log("  {}", sp_fmt_cstr(ordered[it].name));
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
    sp_carr_for(ordered, it) {
      sp_prompt_select_option_t option = {
        .label = ordered[it].name,
        .selected = it == 0
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
    sp_assert(fn);
    result = (*fn)(ctx);
  } else {
    result = run(ctx);
  }

  sp_prompt_end(ctx);
  return result;
}
SP_MAIN(prompt_main)
