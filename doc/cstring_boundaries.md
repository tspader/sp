Only use C strings at API boundaries. Convert immediately to sp_str_t for internal use.

## Good
```c
void SDL_Log(const char* fmt, ...);

void my_log_wrapper(sp_str_t message) {
    c8* cstr = sp_str_to_cstr(message);
    SDL_Log("%s", cstr);
    sp_free(cstr);
}

sp_export void plugin_init(const char* config_path) {
    sp_str_t config = sp_str_from_cstr(config_path);
    
    sp_str_t parent = sp_os_parent_path(config);
    sp_str_t stem = sp_os_extract_stem(config);
    
    process_config(config);
}

s32 main(s32 argc, c8** argv) {
    sp_dyn_array(sp_str_t) args = SP_NULLPTR;
    for (s32 i = 1; i < argc; i++) {
        sp_dyn_array_push(args, sp_str_from_cstr(argv[i]));
    }
    
    process_arguments(args);
    sp_dyn_array_free(args);
    return 0;
}
```

## Bad
```c
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