#define SP_IMPLEMENTATION
#include "../sp.h"

typedef enum {
    CMD_INIT,
    CMD_BUILD,
    CMD_TEST,
    CMD_CLEAN,
    CMD_HELP
} command_t;

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static void init_project(void) { SP_LOG_STR(SP_LIT("init")); }
static void build_project(void) { SP_LOG_STR(SP_LIT("build")); }
static void run_tests(void) { SP_LOG_STR(SP_LIT("test")); }
static void clean_artifacts(void) { SP_LOG_STR(SP_LIT("clean")); }
static void show_help(void) { SP_LOG_STR(SP_LIT("help")); }

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

int main(void) {
  sp_example_init();
  execute_command(parse_command(SP_LIT("build")));
  SP_UNUSED(command_to_string(CMD_TEST));
  sp_example_shutdown();
  return 0;
}
