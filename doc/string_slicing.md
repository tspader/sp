Use string slicing macros and functions for efficient substring operations without allocation.

## Good
```c
sp_str_t path = SP_LIT("/home/user/documents/file.txt");

sp_str_t filename = sp_os_extract_file_name(path);
sp_str_t extension = sp_os_extract_extension(path);
sp_str_t stem = sp_os_extract_stem(path);

sp_str_t first_10 = sp_str_sub(path, 0, 10);
sp_str_t last_4 = sp_str_sub_reverse(path, 0, 4);

if (sp_str_ends_with(path, SP_LIT(".txt"))) {
    sp_str_t without_ext = sp_str_sub(path, 0, path.len - 4);
}

sp_dyn_array(sp_str_t) parts = sp_str_split_c8(path, '/');
```

## Bad
```c
char path[] = "/home/user/documents/file.txt";

char filename[256];
char* last_slash = strrchr(path, '/');
strcpy(filename, last_slash + 1);

char first_10[11];
strncpy(first_10, path, 10);
first_10[10] = '\0';

if (strlen(path) > 4 && strcmp(path + strlen(path) - 4, ".txt") == 0) {
    char without_ext[256];
    strncpy(without_ext, path, strlen(path) - 4);
    without_ext[strlen(path) - 4] = '\0';
}
```

# Tags
- api.strings.sp_str_t.copy
- api.strings.sp_str_t.common_operations
- api.os.filesystem
- api.sp_dynamic_array_t