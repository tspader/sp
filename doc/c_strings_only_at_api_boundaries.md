Only use C strings at API boundaries. Convert immediately to sp_str_t for internal use.

## Good
```c
void SDL_Log(const char* fmt, ...);

void my_log_wrapper(sp_str_t message) {
  SDL_Log("%s", sp_str_to_cstr(message));
}
```

## Bad
```c
void SDL_Log(const char* fmt, ...);

void my_log_wrapper(const char* fmt, ...) {
  SDL_Log("%s", sp_str_to_cstr(message));
}
void my_function(const char* path) {
    char parent[256];
    strcpy(parent, path);
    char* last_slash = strrchr(parent, '/');
    if (last_slash) *last_slash = '\0';

    process_path(parent);
}

void process_config(const char* config_path) {
    if (strstr(config_path, ".toml")) {
        load_toml(config_path);
    }
}
```

# Tags
- api.strings.cstr
- api.strings.sp_str_t.copy
- api.os.filesystem
- usage.general
