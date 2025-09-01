Prefer switch statements with enumerations over if/else chains for clarity and performance.

## Good
```c
typedef enum {
    CMD_INIT,
    CMD_BUILD,
    CMD_TEST,
    CMD_CLEAN,
    CMD_HELP
} command_t;

command_t parse_command(sp_str_t cmd) {
    if (sp_str_equal_cstr(cmd, "init"))  return CMD_INIT;
    if (sp_str_equal_cstr(cmd, "build")) return CMD_BUILD;
    if (sp_str_equal_cstr(cmd, "test"))  return CMD_TEST;
    if (sp_str_equal_cstr(cmd, "clean")) return CMD_CLEAN;
    return CMD_HELP;
}

void execute_command(command_t cmd) {
    switch (cmd) {
        case CMD_INIT:
            init_project();
            break;
        
        case CMD_BUILD:
            build_project();
            break;
        
        case CMD_TEST:
            run_tests();
            break;
        
        case CMD_CLEAN:
            clean_artifacts();
            break;
        
        case CMD_HELP:
            show_help();
            break;
        
        default: {
            SP_UNREACHABLE_CASE();
        }
    }
}

const c8* command_to_string(command_t cmd) {
    switch (cmd) {
        case CMD_INIT:  return "init";
        case CMD_BUILD: return "build";
        case CMD_TEST:  return "test";
        case CMD_CLEAN: return "clean";
        case CMD_HELP:  return "help";
        default: return "unknown";
    }
}
```

## Bad
```c
void execute_command(const char* cmd) {
    if (strcmp(cmd, "init") == 0) {
        init_project();
    }
    else if (strcmp(cmd, "build") == 0) {
        build_project();
    }
    else if (strcmp(cmd, "test") == 0) {
        run_tests();
    }
    else if (strcmp(cmd, "clean") == 0) {
        clean_artifacts();
    }
    else {
        show_help();
    }
}

const char* get_command_description(const char* cmd) {
    if (strcmp(cmd, "init") == 0) {
        return "Initialize project";
    }
    else if (strcmp(cmd, "build") == 0) {
        return "Build project";
    }
    else if (strcmp(cmd, "test") == 0) {
        return "Run tests";
    }
    return "Unknown command";
}
```

# Tags
- api.strings.sp_str_t.comparison
- usage.general