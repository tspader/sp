Use sp_context_push_allocator() to set the allocator for all subsequent allocations in a scope.

## Good
```c
// Once, at initialization
sp_bump_allocator_t bump_allocator;
sp_allocator_t bump = sp_bump_allocator_init(&bump_allocator, 1024 * 1024);

// In some function, push the bump allocator to have all allocations go through it
sp_context_push_allocator(bump);
sp_str_t path = sp_str_from_cstr(filename);
sp_dyn_array(sp_str_t) parts = sp_str_split_c8(path, '/');

// Switch back to the standard allocator, implicitly freeing all memory allocated above
sp_context_pop()
```

## Bad
```c
sp_str_t path = malloc(strlen(filename) + 1);
strcpy(path.data, filename);
path.len = strlen(filename);

char** parts = malloc(sizeof(char*) * 10);
```

# Tags
- api.memory
- api.sp_context_t
- api.strings.sp_str_t.copy
- api.sp_dynamic_array_t
