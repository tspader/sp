Use path operation functions for cross-platform file path manipulation.

## Good
```c
sp_str_t exe = sp_os_get_executable_path();
sp_str_t config_path = sp_os_join_path(sp_os_parent_path(exe), SP_LIT("config.toml"));

sp_str_t filename = sp_os_extract_file_name(config_path);
sp_str_t stem = sp_os_extract_stem(config_path);
sp_str_t ext = sp_os_extract_extension(config_path);

sp_str_t canonical = sp_os_canonicalize_path(SP_LIT("../data/./files"));
```

## Bad
```c
char exe[PATH_MAX];
readlink("/proc/self/exe", exe, PATH_MAX);
char* last_slash = strrchr(exe, '/');
*last_slash = '\0';
char config_path[PATH_MAX];
sprintf(config_path, "%s/config.toml", exe);

char* filename = strrchr(config_path, '/');
if (filename) filename++;

char* ext = strrchr(filename, '.');
```

# Tags
- api.os.filesystem
