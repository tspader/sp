When you have a singleton application state, embrace it instead of passing context parameters.

# Good
```c
void spn_tui_update_noninteractive(spn_tui_t* tui) {
  sp_dyn_array_for(app.build.deps, i) {
    spn_dep_context_t* dep = &app.build.deps[i];
    // Access build context through global app
  }
}

// Called from build function:
spn_tui_update_noninteractive(&app.tui);
```

# Bad
```c
void spn_tui_update_noninteractive(spn_tui_t* tui, spn_build_context_t* context) {
  sp_dyn_array_for(context->deps, i) {
    spn_dep_context_t* dep = &context->deps[i];
    // Process deps from passed context parameter
  }
}

// Called from build function:
spn_tui_update_noninteractive(&tui, &context);
```
