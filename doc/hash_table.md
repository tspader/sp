Use sp_hash_table() macro for type-safe hash tables with automatic hashing.

## Good
```c
sp_hash_table(sp_str_t, u32) file_sizes;
sp_hash_table_init(&file_sizes, 128);

sp_hash_table_insert(&file_sizes, SP_LIT("main.c"), 1024);
sp_hash_table_insert(&file_sizes, SP_LIT("test.c"), 512);

u32* size = sp_hash_table_get(&file_sizes, SP_LIT("main.c"));
if (size) {
    process_file_size(*size);
}

sp_hash_table_iter_t it = sp_hash_table_iter(&file_sizes);
while (sp_hash_table_iter_advance(&it)) {
    sp_str_t* key = sp_hash_table_iter_key(&it);
    u32* value = sp_hash_table_iter_value(&it);
}

sp_hash_table_destroy(&file_sizes);
```

## Bad
```c
typedef struct {
    char* key;
    u32 value;
} entry_t;

entry_t* table = malloc(sizeof(entry_t) * 128);
int table_size = 0;

for (int i = 0; i < table_size; i++) {
    if (strcmp(table[i].key, "main.c") == 0) {
        return table[i].value;
    }
}
```

# Tags
- api.containers
- api.hashing