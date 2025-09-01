Always provide symmetric init/cleanup functions for clear resource management.

# Good
```c
void spn_tui_init(spn_tui_t* tui, spn_tui_state_t state) {
  tui->state = state;
  // All initialization in one place
  switch (tui->state) {
    case SPN_TUI_STATE_INTERACTIVE:
      // Interactive init
      break;
    case SPN_TUI_STATE_NONINTERACTIVE:
      // Non-interactive init
      break;
  }
}

void spn_tui_cleanup(spn_tui_t* tui) {
  switch (tui->state) {
    case SPN_TUI_STATE_INTERACTIVE:
      tcsetattr(STDIN_FILENO, TCSANOW, &tui->terminal.ios);
      sp_tui_show_cursor();
      sp_tui_home();
      sp_tui_flush();
      break;
    default:
      break;
  }
}

// Usage:
spn_tui_init(&app.tui, state);
// ... do work ...
spn_tui_cleanup(&app.tui);
```

# Bad
```c
void spn_project_build(spn_project_t* project) {
  // Initialization scattered throughout
  spn_tui_t tui = SP_ZERO_INITIALIZE();

  if (app.cli.no_interactive) {
    tui.state = SPN_TUI_STATE_NONINTERACTIVE;
    tui.num_deps = sp_dyn_array_size(context.deps);
    tui.terminal_reported = (bool*)sp_alloc(tui.num_deps * sizeof(bool));
    // More init...
  } else {
    tui.state = SPN_TUI_STATE_INTERACTIVE;
    // Different init...
  }

  // ... build logic ...

  // Cleanup mixed with logic
  if (terminal_modified) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
  }
  if (!app.cli.no_interactive) {
    sp_tui_show_cursor();
    sp_tui_home();
    sp_tui_flush();
  }
}
```

# Tags
- usage.general
