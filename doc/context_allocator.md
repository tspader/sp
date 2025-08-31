Use sp_context_push_allocator() to set the allocator for all subsequent allocations in a scope.

## Good
```c
sp_bump_allocator_t temp_alloc;
sp_bump_allocator_init(&temp_alloc, 1024 * 1024);
sp_context_push_allocator(sp_bump_allocator_init(&temp_alloc, 1024 * 1024));

sp_str_t path = sp_str_from_cstr(filename);
sp_dyn_array(sp_str_t) parts = sp_str_split_c8(path, '/');

sp_context_pop();
sp_bump_allocator_clear(&temp_alloc);
```

## Bad
```c
sp_str_t path = malloc(strlen(filename) + 1);
strcpy(path.data, filename);
path.len = strlen(filename);

char** parts = malloc(sizeof(char*) * 10);
```