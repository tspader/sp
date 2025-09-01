Declare a global struct called `app`, and place anything whose lifetime is the duration of the program inside this struct. Do not pass the struct to functions; reference it from the global scope.

# Good
```c
typedef struct {
  spn_project_t project;
  spn_config_t config;
  spn_build_context_t build;  // Build context as app member
  spn_tui_t tui;
} spn_app_t;

void spn_cli_command_build(spn_cli_t* cli) {
  app.build = spn_build_context_from_default_profile();
  spn_build_context_prepare(&app.build);

  // No need to pass around
  spn_tui_init(&app.tui, state);
  spn_tui_update(&app.tui);
  spn_lock_file_generate(&app.build);
}
```

# Bad
```c
void spn_project_build(spn_project_t* project) {
  // Build context as local variable
  spn_build_context_t context = spn_build_context_from_default_profile();
  spn_build_context_prepare(&context);

  // Pass it around
  spn_tui_init_noninteractive(&tui, &context);
  spn_tui_update(&tui, &context);
  spn_lock_file_generate(&context);
}
```

# Tags
- usage.general
