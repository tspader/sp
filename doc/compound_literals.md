Always prefer compound literals to initialize when possible.

## Good
```c
sp_os_directory_entry_t entry = {
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
- usage.general
