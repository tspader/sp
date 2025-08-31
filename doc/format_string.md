Use sp_format() with SP_FMT_* macros for type-safe string formatting without printf.

## Good
```c
sp_str_t msg = sp_format("{} files processed in {}ms", 
    SP_FMT_U32(file_count), SP_FMT_F32(elapsed));

sp_str_t path = sp_format("{}/{}", 
    SP_FMT_STR(base_dir), SP_FMT_STR(filename));

sp_str_t quoted = sp_format("Error in file {}", 
    SP_FMT_QUOTED_STR(filename));

sp_str_t status = sp_format("Server {} on port {}", 
    SP_FMT_CSTR(is_running ? "running" : "stopped"), 
    SP_FMT_U32(port));

sp_str_t hex = sp_format("Color: 0x{:06x}", 
    SP_FMT_U32(rgb_value));
```

## Bad
```c
char msg[256];
sprintf(msg, "%d files processed in %.2fms", file_count, elapsed);

char path[PATH_MAX];
snprintf(path, PATH_MAX, "%s/%s", base_dir, filename);

char quoted[512];
sprintf(quoted, "Error in file \"%s\"", filename);
```