Using SP_FATAL for unrecoverable errors. It is better to fail quickly than try to recover, and SP_FATAL is the best way to do so

## GOOD

```c
if (!sp_os_does_path_exist(config_file)) {
  SP_FATAL("Configuration file not found: {}", SP_FMT_STR(config_file));
}

if (version_major > 99) {
  SP_FATAL("Version major {} exceeds maximum (99)", SP_FMT_U32(version_major));
}
```

## BAD

```c
if (!file_exists(path)) {
  printf("Error: file not found\n");
  exit(1);
}

assert(config_file != NULL);
```
