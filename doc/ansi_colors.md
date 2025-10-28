Use ANSI color constants and format extensions for colored terminal output.

## Good
```c
sp_format("{:fg brightcyan} tests failed", SP_FMT_U32(failed_count));

sp_str_builder_t output = SP_ZERO_INITIALIZE();
sp_str_builder_append_fmt(&output, "{:fg brightgreen}", SP_FMT_CSTR("All tests passed"));

sp_str_t status = SP_LIT("Some status message produced by your code");
sp_format("{:fg brightcyan}", SP_FMT_STR(status));

SP_LOG(
  "{:fg brightyellow}: {:fg brightblack}",
  SP_FMT_CSTR("warning"),
  SP_FMT_CSTR(get_error_from_some_external_library())
);
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
