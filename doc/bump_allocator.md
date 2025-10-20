Use sp_bump_allocator for fast temporary allocations with bulk deallocation.

## Good
```c
sp_bump_allocator_t temp;
sp_allocator_t temp_allocator = sp_bump_allocator_init(&temp, 10 * 1024 * 1024);
sp_context_push_allocator(temp_allocator);

sp_dyn_array(sp_str_t) tokens = tokenize_file(source);
sp_ht(sp_str_t, u32) symbol_table;
sp_ht_init(&symbol_table, 512);

for (u32 i = 0; i < sp_dyn_array_size(tokens); i++) {
    sp_str_t symbol = sp_str_copy(tokens[i]);
    sp_ht_insert(&symbol_table, symbol, i);
}

compile_result_t result = compile(&symbol_table);

sp_context_pop();
sp_bump_allocator_clear(&temp);
```

## Bad
```c
char** tokens = malloc(sizeof(char*) * 1000);
for (int i = 0; i < token_count; i++) {
    tokens[i] = malloc(strlen(source_tokens[i]) + 1);
    strcpy(tokens[i], source_tokens[i]);
}

for (int i = 0; i < token_count; i++) {
    free(tokens[i]);
}
free(tokens);
```

# Tags
- api.memory
- api.sp_context_t
- api.sp_dynamic_array_t
- api.containers
- api.strings.sp_str_t.copy
