#define SP_IMPLEMENTATION
#include "../sp.h"

typedef struct SDL_Process {
  int dummy;
} SDL_Process;

typedef struct SDL_Environment {
  int dummy;
} SDL_Environment;

static SDL_Process* SDL_CreateProcess(char* const* args, bool inherit_handles, int pipes) {
  SP_UNUSED(args);
  SP_UNUSED(inherit_handles);
  SP_UNUSED(pipes);
  static SDL_Process process;
  return &process;
}

static u8* SDL_ReadProcess(SDL_Process* process, size_t* output_size, void* error) {
  SP_UNUSED(process);
  SP_UNUSED(error);
  static u8 buffer[] = "ok";
  *output_size = sizeof(buffer) - 1;
  return buffer;
}

static s32 SDL_WaitProcess(SDL_Process* process, bool block) {
  SP_UNUSED(process);
  SP_UNUSED(block);
  return 0;
}

static void SDL_free(void* ptr) {
  SP_UNUSED(ptr);
}

static SDL_Environment* SDL_GetEnvironment(void) {
  static SDL_Environment env;
  return &env;
}

static const char* SDL_GetEnvironmentVariable(SDL_Environment* env, const char* name) {
  SP_UNUSED(env);
  SP_UNUSED(name);
  return "PATH";
}

static void SDL_SetEnvironmentVariable(SDL_Environment* env, const char* name, const char* value, bool overwrite) {
  SP_UNUSED(env);
  SP_UNUSED(name);
  SP_UNUSED(value);
  SP_UNUSED(overwrite);
}

static const char* SDL_GetBasePath(void) {
  return "/tmp";
}

static void sp_example_init(void) {
  sp_config_t config = { .allocator = sp_malloc_allocator_init() };
  sp_init(config);
}

static void sp_example_shutdown(void) {
  sp_context_pop();
}

static char** sp_str_to_cstr_array(sp_str_t command) {
  char** args = (char**)sp_alloc(sizeof(char*) * 2);
  args[0] = sp_str_to_cstr(command);
  args[1] = NULL;
  return args;
}

static void sp_free_cstr_array(char** args) {
  if (!args) return;
  if (args[0]) {
    sp_free(args[0]);
  }
  sp_free(args);
}

int main(void) {
  sp_example_init();

  sp_str_t project_dir = SP_LIT("/tmp/project");
  sp_str_t command = sp_format("make -C {} build", SP_FMT_STR(project_dir));
  char** args = sp_str_to_cstr_array(command);

  SDL_Process* process = SDL_CreateProcess(args, true, SP_SDL_PIPE_STDIO);

  size_t output_size = 0;
  u8* output = SDL_ReadProcess(process, &output_size, NULL);
  s32 exit_code = SDL_WaitProcess(process, true);

  if (exit_code == 0) {
    sp_str_t result = sp_str((c8*)output, (u32)output_size);
    SP_LOG("process output: {}", SP_FMT_STR(result));
  }
  SDL_free(output);

  sp_str_t env_path = sp_str_from_cstr(SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "PATH"));
  SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "CUSTOM_VAR", "value", true);

  sp_str_t base_path = sp_str_from_cstr(SDL_GetBasePath());

  sp_free_cstr_array(args);
  if (command.data) {
    sp_free((void*)command.data);
  }
  sp_free((void*)env_path.data);
  sp_free((void*)base_path.data);

  sp_example_shutdown();
  return 0;
}
