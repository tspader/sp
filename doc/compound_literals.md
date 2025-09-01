Always prefer compound literals to initialize when possible. Use SP_RVAL() for compound literal casts that work in both C and C++.

## Good
```c
sp_os_directory_entry_t entry = SP_RVAL(sp_os_directory_entry_t) {
    .file_path = file_path,
    .file_name = sp_str_from_cstr(find_data.cFileName),
    .attributes = sp_os_winapi_attr_to_sp_attr(attrs),
};

return SP_RVAL(sp_precise_epoch_time_t) {
    .s = (u64)st.st_mtime,
    .ns = 0
};

sp_str_t substr = SP_RVAL(sp_str_t) {
    .len = end - start,
    .data = str.data + start
};
```

## Bad
```c
sp_os_directory_entry_t entry;
entry.file_path = file_path;
entry.file_name = sp_str_from_cstr(find_data.cFileName);
entry.attributes = sp_os_winapi_attr_to_sp_attr(attrs);

sp_precise_epoch_time_t result;
result.s = (u64)st.st_mtime;
result.ns = 0;
return result;

sp_str_t substr = { end - start, str.data + start };
```

# Tags
- api.strings.sp_str_t.copy
- api.os.time
- api.os.filesystem
- usage.general
