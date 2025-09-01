Returning bool or error codes for recoverable errors.

## GOOD

```c
bool load_optional_config(sp_str_t path) {
  if (!sp_os_does_path_exist(path)) {
    SP_LOG("Optional config not found, using defaults");
    return false;
  }

  if (!parse_config(path)) {
    SP_LOG("Config parse failed, using defaults");
    return false;
  }

  return true;
}
```
