Use sp_str_starts_with() to check if a string begins with a specific prefix

## Good
```c
if (sp_str_starts_with(url, SP_LIT("https://"))) {
  use_secure_connection(url);
}
else if (sp_str_starts_with(url, SP_LIT("http://"))) {
  use_standard_connection(url);
}

sp_dyn_array_for(entries, i) {
  if (sp_str_starts_with(entries[i].file_name, SP_LIT("."))) {
    continue;
  }
}
```

## Bad
```c
if (str.len >= 5 && memcmp(str.data, "hello", 5) == 0) {

}

if (strncmp(sp_str_to_cstr(str), "prefix", 6) == 0) {

}

bool starts_with = true;
for (u32 i = 0; i < prefix.len; i++) {
  if (str.data[i] != prefix.data[i]) {
    starts_with = false;
    break;
  }
}
```

# Tags
- api.strings.sp_str_t.comparison
- api.strings.sp_str_starts_with
- api.strings.sp_str_ends_with
