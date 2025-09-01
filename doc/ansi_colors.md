Use ANSI color constants and format extensions for colored terminal output.

## Good
```c
sp_str_t colored = sp_format("{:color red}", SP_FMT_CSTR("FAILED"));
sp_str_t test_msg = sp_format("{:color brightcyan} tests failed", SP_FMT_U32(failed_count));

sp_str_builder_t output = SP_ZERO_INITIALIZE();
sp_str_builder_append_fmt(&output, "{:color brightgreen}", SP_FMT_CSTR("All tests passed"));

sp_str_t status = sp_format("{:color brightcyan}", SP_FMT_STR(status_text));

sp_log(SP_LIT("{:color yellow}WARNING{:color reset}: {}"),
    SP_FMT_CSTR(""), SP_FMT_STR(warning_msg));
```

## Bad
```c
printf("\033[31mError: \033[0m");
printf("%s", error_message);

char colored[256];
sprintf(colored, "\033[31mFAILED\033[0m: %d tests", failed_count);

printf("\033[32mAll tests passed\033[0m");

sp_str_builder_t output = SP_ZERO_INITIALIZE();
sp_str_builder_append_cstr(&output, SP_ANSI_FG_GREEN);
sp_str_builder_append(&output, SP_LIT("All tests passed"));
sp_str_builder_append_cstr(&output, SP_ANSI_RESET);

sp_str_t status = sp_format("{}{}{}", 
    SP_FMT_COLOR(SP_ANSI_FG_BRIGHT_CYAN),
    SP_FMT_STR(status_text),
    SP_FMT_COLOR(SP_ANSI_RESET));
```

# Tags
- api.strings.sp_str_builder_t
- api.logging
- api.os.formatting
