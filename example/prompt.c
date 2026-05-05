/*
                                                                                        █████       █████
                                                                                      ▒▒███       ▒▒███
    █████  ████████            ████████  ████████   ██████  █████████████   ████████  ███████      ▒███████
   ███▒▒  ▒▒███▒▒███          ▒▒███▒▒███▒▒███▒▒███ ███▒▒███▒▒███▒▒███▒▒███ ▒▒███▒▒███▒▒▒███▒       ▒███▒▒███
  ▒▒█████  ▒███ ▒███           ▒███ ▒███ ▒███ ▒▒▒ ▒███ ▒███ ▒███ ▒███ ▒███  ▒███ ▒███  ▒███        ▒███ ▒███
   ▒▒▒▒███ ▒███ ▒███           ▒███ ▒███ ▒███     ▒███ ▒███ ▒███ ▒███ ▒███  ▒███ ▒███  ▒███ ███    ▒███ ▒███
   ██████  ▒███████  █████████ ▒███████  █████    ▒▒██████  █████▒███ █████ ▒███████   ▒▒█████  ██ ████ █████
  ▒▒▒▒▒▒   ▒███▒▒▒  ▒▒▒▒▒▒▒▒▒  ▒███▒▒▒  ▒▒▒▒▒      ▒▒▒▒▒▒  ▒▒▒▒▒ ▒▒▒ ▒▒▒▒▒  ▒███▒▒▒     ▒▒▒▒▒  ▒▒ ▒▒▒▒ ▒▒▒▒▒
           ▒███                ▒███                                         ▒███
           █████               █████                                        █████
          ▒▒▒▒▒               ▒▒▒▒▒                                        ▒▒▒▒▒

  >> sp_prompt.h
  beautiful, interactive, zero-dependency utf-8 prompts for native CLIs

  In the interest of consolidation, explanations and documentation live at the top of sp_prompt.h. This file
  is mostly code, not prose. If something confuses you, check the header.
*/

#define SP_UNIMPLEMENTED() ((void)0);
#define SP_IMPLEMENTATION
#include "sp.h"
#include "sp/sp_prompt.h"

typedef s32 (*sp_prompt_demo_fn_t)(sp_prompt_ctx_t* ctx);

s32 demo_note(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Note");
  sp_prompt_note(ctx, "Wow. We got someone with a nose for *exciting* widgets", "Congratulations");
  sp_prompt_outro(ctx, "Bye!");
  return 0;
}

s32 demo_indicators(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Indicators");
  sp_prompt_info(ctx, "info");
  sp_prompt_success(ctx, "success");
  sp_prompt_warn(ctx, "warning");
  sp_prompt_error(ctx, "error");
  sp_prompt_cancel(ctx, "cancel");
  sp_prompt_outro(ctx, "outro");
  return 0;
}

s32 demo_password(sp_prompt_ctx_t* ctx) {
  const c8* secret = sp_prompt_password(ctx, "Demo: Password", "sekret");
  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
    return 1;
  }

  sp_prompt_note(ctx, secret, "Captured");
  sp_prompt_outro(ctx, "Password successfully sent to Mossad liaison");
  return 0;
}

s32 demo_text(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Text");
  const c8* text = sp_prompt_text(ctx, "Type something", "");

  if (sp_prompt_submitted(ctx)) {
    sp_prompt_note(ctx, text, "Text");
  }
  else if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "Oh...");
  }

  sp_prompt_outro(ctx, "Tastefully done");
  return 0;
}

/////////////
// CONFIRM //
/////////////
s32 demo_confirm(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Confirm");
  bool confirmed = sp_prompt_confirm(ctx, "spum?", false);

  if (sp_prompt_submitted(ctx)) {
    sp_prompt_note(ctx, confirmed ? "yes" : "no", "Selected");
  }
  else if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "cancelled");
  }

  sp_prompt_outro(ctx, "Good job!");

  return 0;
}


////////////
// SELECT //
////////////
s32 demo_select(sp_prompt_ctx_t* ctx) {
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
    .max_visible = 4,
  });

  if (sp_prompt_submitted(ctx)) {
    sp_prompt_note(ctx, sp_prompt_get_str(ctx), "Greeting");
  }
  else if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "You say nothing at all");
  }

  sp_prompt_outro(ctx, "Later");

  return 0;
}

s32 demo_select_filter(sp_prompt_ctx_t* ctx) {
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
    .max_visible = 4,
    .filter = true,
  });

  if (sp_prompt_submitted(ctx)) {
    const c8* selection = sp_prompt_get_str(ctx);
    sp_str_t reaction = sp_fmt_a(sp_context_get_allocator(), "{.quote}, eh? A childish response...", sp_fmt_cstr(selection)).value;
    sp_prompt_note(ctx, sp_str_to_cstr_a(sp_context_get_allocator(), reaction), "Selection");
  }
  else if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "You got cold feet...");
  }

  sp_prompt_outro(ctx, "Bye!");
  return 0;
}

/////////////////
// MULTISELECT //
/////////////////
s32 demo_multiselect(sp_prompt_ctx_t* ctx) {
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
    .max_visible = 6,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "You picked no cats");
    return 1;
  }

  const c8* selected = sp_prompt_join_selection(options, sp_carr_len(options));
  sp_prompt_note(ctx, selected, "Cats");
  sp_prompt_outro(ctx, "Bye!");
  return 0;
}

s32 demo_multiselect_filter(sp_prompt_ctx_t* ctx) {
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
    .max_visible = 6,
    .filter = true,
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "The orange cat lies despondent, nameless...");
    return 1;
  }

  sp_prompt_note(ctx, sp_prompt_join_selection(options, sp_carr_len(options)), "Cat names");
  sp_prompt_outro(ctx, "Bye!");
  return 0;
}


/////////////
// SPINNER //
/////////////
s32 spinner_thread(void* userdata) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)userdata;
  sp_sleep_ns(sp_tm_s_to_ns(2));
  sp_prompt_complete(ctx);
  return 0;
}

s32 demo_spinner(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Spinner");

  sp_thread_t worker = sp_zero();
  sp_thread_init(&worker, spinner_thread, ctx);

  sp_prompt_spinner(ctx, (sp_prompt_spinner_t) {
    .prompt = "Spinning for 2 seconds...",
    .fps = 12,
    .frames = SP_PROMPT_SPINNER_PACMAN_MUNCHER,
    .color.rgb = { .r = 0x55, .g = 0xAA, .b = 0xFF },
  });

  if (sp_prompt_submitted(ctx)) {
    sp_prompt_success(ctx, "You did it!");
  }
  else if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "You got dizzy and decided against spinning");
  }

  return 0;
}


//////////////////
// KNIGHT RIDER //
//////////////////
s32 demo_knight_rider(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Knight Rider");

  sp_prompt_knight_rider(ctx, (sp_prompt_knight_rider_t) {
    .prompt = "Press Enter to stop",
    .fps = 200,
    .color = { .r = 0x55, .g = 0xAA, .b = 0xFF },
  });

  if (sp_prompt_cancelled(ctx)) {
    sp_prompt_cancel(ctx, "Knight riding unsuccessful");
    return 1;
  }

  sp_prompt_success(ctx, "Knight ridden!");
  return 0;
}


//////////////
// PROGRESS //
//////////////
f32 ease_in_out_3_f32(f32 t) {
  if (t < 0.5) {
    return 4.0 * t * t * t;
  }
  else {
    f32 p = 2.0 * t - 2.0;
    return 0.5 * p * p * p + 1.0;
  }
}

const c8* progress_to_status(f32 progress) {
  if (progress < 0.25f) return "Cramming";
  else if (progress < 0.50f) return "Spraying";
  else if (progress < 0.75f) return "Grunting";
  else return "Screaming";
}

s32 demo_progress_worker(void* userdata) {
  sp_prompt_ctx_t* ctx = (sp_prompt_ctx_t*)userdata;

  sp_for(it, 233) { // Arbitrary
    f32 progress = ease_in_out_3_f32((f32)it / 233);
    sp_prompt_send_progress_f32(ctx, progress);
    sp_prompt_send_status(ctx, progress_to_status(progress));

    // Sleep a little while so we can see the progress
    sp_sleep_ms(10);

    // If you want your prompt to be able to tell a worker to
    // cancel, then the worker should poll like this at whatever
    // its cancellation points are.
    if (sp_prompt_is_aborted(ctx)) {
      return 0;
    }

    // If it's the other way around, and you want the worker to
    // tell the prompt to cancel, you can call this from any
    // thread at any time:
    //
    //   sp_prompt_abort(ctx);
  }

  // Tell the prompt that it's done. This isn't hardcoded in the
  // progress widget; it works for any widget.
  sp_prompt_complete(ctx);
  return 0;
}

s32 demo_progress(sp_prompt_ctx_t* ctx) {
  sp_prompt_intro(ctx, "Demo: Progress");

  sp_thread_t worker = SP_ZERO_INITIALIZE();
  sp_thread_init(&worker, demo_progress_worker, ctx);

  sp_prompt_progress(ctx, (sp_prompt_progress_t) {
    .prompt = "Working...",
    .color.rgb = { .r = 0x55, .g = 0xAA, .b = 0xFF },
  });

  sp_thread_join(&worker);

  if (sp_prompt_submitted(ctx)) {
    sp_prompt_success(ctx, "Installed!");
  }
  else {
    sp_prompt_cancel(ctx, "Cancelled");
  }
  return 0;
}

typedef struct {
  const c8* name;
  sp_prompt_demo_fn_t fn;
} demo_t;

s32 prompt_main(s32 argc, const c8** argv) {
  sp_prompt_demo_fn_t run = SP_NULLPTR;

  demo_t ordered [] = {
    { "Spinner", demo_spinner },
    { "Progress", demo_progress },
    { "Text", demo_text },
    { "Select", demo_select },
    { "Select + Filter", demo_select_filter },
    { "Multiselect", demo_multiselect },
    { "Multiselect + Filter", demo_multiselect_filter },
    { "Note", demo_note },
    { "Indicators", demo_indicators },
    { "Confirm", demo_confirm },
    { "Password", demo_password },
    { "Knight Rider", demo_knight_rider },
  };

  sp_ht_a(const c8*, sp_prompt_demo_fn_t) demos = sp_zero();
  sp_cstr_ht_init_a(sp_context_get()->allocator, demos);
  sp_carr_for(ordered, it) {
    sp_cstr_ht_insert(demos, ordered[it].name, ordered[it].fn);
  }

  if (argc >= 2) {
    sp_prompt_demo_fn_t* fn = sp_cstr_ht_get(demos, argv[1]);
    if (!fn) {
      sp_log_a("usage: prompt [program]");
      sp_log_a("programs:");
      sp_carr_for(ordered, it) {
        sp_log_a("  {}", sp_fmt_cstr(ordered[it].name));
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
    sp_da_init(sp_context_get()->allocator, options);
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
      sp_prompt_cancel(ctx, "The idea terrifies you. You back away slowly.");
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
