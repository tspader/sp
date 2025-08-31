Use sp_str_builder_new_line() instead of appending \n for better readability and consistency.

# Good
```c
sp_str_builder_append(&builder, SP_LIT("[[package]]"));
sp_str_builder_new_line(&builder);
sp_str_builder_append_fmt_c8(&builder, "name = {}", SP_FMT_QUOTED_STR(entry->name));
sp_str_builder_new_line(&builder);
sp_str_builder_append_fmt_c8(&builder, "git_url = {}", SP_FMT_QUOTED_STR(entry->git_url));
sp_str_builder_new_line(&builder);
```

# Bad
```c
sp_str_builder_append(&builder, SP_LIT("[[package]]\n"));
sp_str_builder_append_fmt_c8(&builder, "name = {}\n", SP_FMT_QUOTED_STR(entry->name));
sp_str_builder_append_fmt_c8(&builder, "git_url = {}\n", SP_FMT_QUOTED_STR(entry->git_url));
```
