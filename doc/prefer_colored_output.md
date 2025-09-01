Prefer to color your program output using sp_format(); a little goes a long way.

## GOOD

```c
sp_str_t success = sp_format(
  "{:fg green} {:fg cyan}",
  SP_FMT_CSTR("Built"),
  SP_FMT_STR(target_name)
);
sp_log(success);

sp_str_t error = sp_format(
  "{:fg red} {:fg yellow}",
  SP_FMT_CSTR("Failed"),
  SP_FMT_STR(error_file)
);
sp_log(error);
```

## BAD

```c
printf("Built: %s\n", target);
printf("Error: %s\n", file);
```

# Tags
- api.logging
- api.os.formatting
- usage.general
