Organize CLI commands with a dispatch table.

## GOOD
```c
typedef struct {
  sp_str_t name;
  sp_str_t description;
  spn_cmd_func_t func;
} command_t;

static const command_t commands[] = {
  { SP_LIT("init"),  SP_LIT("Initialize a new project"),  cmd_init },
  { SP_LIT("build"), SP_LIT("Build the project"),         cmd_build },
  { SP_LIT("clean"), SP_LIT("Clean build artifacts"),     cmd_clean },
};

bool dispatch_command(sp_str_t cmd_name, app_t* app) {
  for (u32 i = 0; i < SP_CARR_SIZE(commands); i++) {
    if (sp_str_equal(commands[i].name, cmd_name)) {
      return commands[i].func(app);
    }
  }

  SP_LOG("{:fg red}: Unknown command {}", SP_FMT_CSTR("ERROR"), SP_FMT_QUOTED_STR(cmd_name));
  return false;
}
```
