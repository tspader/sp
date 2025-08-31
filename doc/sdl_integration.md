Use SDL integration patterns for cross-platform OS operations. Always check which backend is selected (SP_OS_BACKEND_NATIVE by default, or SP_OS_BACKEND_SDL) and use native APIs only where the API does not exist in sp.h

## Good
```c
sp_str_t command = sp_format("make -C {} build", SP_FMT_STR(project_dir));

SDL_Process* process = SDL_CreateProcess(
    sp_str_to_cstr_array(command),
    true,
    SP_SDL_PIPE_STDIO);

size_t output_size;
u8* output = SDL_ReadProcess(process, &output_size, NULL);
s32 exit_code = SDL_WaitProcess(process, true);

if (exit_code == 0) {
    sp_str_t result = sp_str((c8*)output, output_size);
    process_build_output(result);
}
SDL_free(output);

sp_str_t env_path = sp_str_from_cstr(SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "PATH"));
SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "CUSTOM_VAR", "value", true);

sp_str_t base_path = sp_str_from_cstr(SDL_GetBasePath());
```

## Bad
```c
#ifdef _WIN32
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    CreateProcess(NULL, "make -C project build", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
#else
    FILE* pipe = popen("make -C project build", "r");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        process_line(buffer);
    }
    pclose(pipe);
#endif

char* path = getenv("PATH");
setenv("CUSTOM_VAR", "value", 1);
```
