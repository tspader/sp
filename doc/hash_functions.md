Use sp_hash_* functions for hashing strings and data.

## Good
```c
sp_hash_t file_hash = sp_hash_cstr(filename);
sp_hash_t data_hash = sp_hash_bytes(buffer, buffer_size, 0);
sp_hash_t combined = sp_hash_combine(hashes, hash_count);
sp_hash_t content_hash = sp_hash_bytes(file_data.data, file_data.len, 42);
```

## Bad
```c
unsigned long hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

unsigned long file_hash = hash_string(filename);

int find_in_cache(const char* path) {
    for (int i = 0; i < cache_size; i++) {
        if (strcmp(cache[i].path, path) == 0) {
            return i;
        }
    }
    return -1;
}
```
