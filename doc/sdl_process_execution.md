Running external commands using SDL3 process APIs with sp.h idioms. SDL is available when a project uses SP_OS_BACKEND_SDL before including `sp.h`.

## GOOD

```c
SDL_Process* process = SDL_CreateProcess(
  (const c8*[]) { "git", "status", "--short", SP_NULLPTR },
  SP_SDL_PIPE_STDIO
);

if (!process) {
  SP_LOG("Failed to run git status");
  return false;
}

sp_size_t len = 0;
s32 return_code;
void* output = SDL_ReadProcess(process, &len, &return_code);
SDL_DestroyProcess(process);

if (return_code != 0) {
  SP_LOG("git status failed with code {}", SP_FMT_S32(return_code));
  SDL_free(output);
  return false;
}

sp_str_t result = sp_str_from_cstr_sized((c8*)output, len);
SDL_free(output);
```

## BAD

```c
FILE* fp = popen("git status --short", "r");  // unsafe, shell injection risk
char buffer[1024];
fgets(buffer, sizeof(buffer), fp);
pclose(fp);
```
