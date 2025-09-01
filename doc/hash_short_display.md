Converting hashes to short build IDs for display.

## GOOD

```c
sp_str_t format_build_id(u64 hash) {
  sp_str_t full_hash = sp_format("{:016x}", SP_FMT_U64(hash));
  sp_str_t short_hash = sp_str_sub(full_hash, 0, 8);

  return sp_format(
    "{:fg brightblack}",
    SP_FMT_STR(short_hash)
  );
}

sp_str_t version_str = sp_format(
  "{} {:fg brightblack}",
  SP_FMT_STR(version),
  SP_FMT_STR(build_id)
);
```

# Tags
- api.hashing
- api.strings.sp_str_t.common_operations
- api.os.formatting
