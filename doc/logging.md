Use sp_log and SP_LOG macros for formatted logging output.

## Good
```c
SP_LOG("Server started on port {}", SP_FMT_U32(port));

sp_str_t status = SP_LIT("ready");
sp_log(SP_LIT("Status: {}"), SP_FMT_STR(status));

SP_LOG_STR(SP_LIT("Initialization complete"));

SP_LOG("{:color green} tests passed", SP_FMT_U32(test_count));

void log_error(sp_str_t file, u32 line, sp_str_t msg) {
    sp_log(SP_LIT("{:color red} in {}:{} - {}"),
        SP_FMT_CSTR("ERROR"),
        SP_FMT_STR(file),
        SP_FMT_U32(line),
        SP_FMT_STR(msg));
}

#define LOG_DEBUG(msg) \
    SP_LOG("[DEBUG] {} ({}:{})", \
        SP_FMT_CSTR(msg), \
        SP_FMT_CSTR(__FILE__), \
        SP_FMT_U32(__LINE__))
```

## Bad
```c
printf("Server started on port %d\n", port);

fprintf(stderr, "Status: %s\n", status);

printf("\033[32mSUCCESS\033[0m: %d tests passed\n", test_count);

void log_error(const char* file, int line, const char* msg) {
    fprintf(stderr, "\033[31mERROR\033[0m in %s:%d - %s\n",
            file, line, msg);
}

#define LOG_DEBUG(msg) \
    printf("[DEBUG] %s (%s:%d)\n", msg, __FILE__, __LINE__)
```

# Tags
- api.logging
- api.os.formatting
