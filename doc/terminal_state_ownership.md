Encapsulate common data in an inner struct

# Good
```c
typedef struct {
  spn_tui_state_t state;
  bool* terminal_reported;
  u32 num_deps;
  u32 width;

  struct {
    struct termios ios;
    bool modified;
  } terminal;
} spn_tui_t;
```

# Bad
```c
typedef struct {
  spn_tui_state_t state;
  bool* terminal_reported;
  u32 num_deps;
  u32 width;

  struct termios orig_termios = {0};
  bool terminal_modified = false;
} spn_tui_t;
```

# Tags
- usage.general
