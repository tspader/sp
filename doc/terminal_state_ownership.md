Encapsulate common data in a struct, even if it's only used in one place.

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

void spn_tui_init(spn_tui_t* tui, spn_tui_state_t state) {
  tcgetattr(STDIN_FILENO, &tui->terminal.ios);
  struct termios ios = tui->terminal.ios;
  ios.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &ios);
  tui->terminal.modified = true;
}
```

# Bad
```c
void spn_project_build(spn_project_t* project) {
  spn_tui_state_t state;
  bool* terminal_reported;
  u32 num_deps;
  u32 width;

  struct termios orig_termios = {0};
  bool terminal_modified = false;

  // Terminal setup scattered in build logic
  tcgetattr(STDIN_FILENO, &orig_termios);
  struct termios new_termios = orig_termios;
  new_termios.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}
```
