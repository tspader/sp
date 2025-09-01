Use sp_dyn_array() macro and related functions for type-safe dynamic arrays in the gunslinger style.

## Good
```c
sp_dyn_array(sp_str_t) files = SP_NULLPTR;

sp_dyn_array_push(files, SP_LIT("main.c"));
sp_dyn_array_push(files, SP_LIT("utils.c"));
sp_dyn_array_push(files, SP_LIT("test.c"));

sp_dyn_array_for(files, i) {
    compile_file(files[i]);
}

u32 count = sp_dyn_array_size(files);
sp_dyn_array_free(files);
```

## Bad  
```c
sp_str_t* files = malloc(sizeof(sp_str_t) * 10);
int file_count = 0;
int file_capacity = 10;

files[file_count++] = SP_LIT("main.c");
if (file_count >= file_capacity) {
    file_capacity *= 2;
    files = realloc(files, sizeof(sp_str_t) * file_capacity);
}
files[file_count++] = SP_LIT("utils.c");

for (int i = 0; i < file_count; i++) {
    compile_file(files[i]);
}
```

# Tags
- api.sp_dynamic_array_t