Always prefer compound literals to initialize when possible. Use SP_RVAL() for compound literal casts that work in both C and C++.

## Good
```c
sp_os_directory_entry_t entry = SP_RVAL(sp_os_directory_entry_t) {
  .file_path = file_path,
  .file_name = sp_str_from_cstr(find_data.cFileName),
  .attributes = sp_os_winapi_attr_to_sp_attr(attrs),
};
```

## Bad
```c
sp_os_directory_entry_t entry;
entry.file_path = file_path;
entry.file_name = sp_str_from_cstr(find_data.cFileName);
entry.attributes = sp_os_winapi_attr_to_sp_attr(attrs);
```

# Tags
- api.strings.sp_str_t.copy
- api.os.time
- api.os.filesystem
- usage.general
