Use sort kernels which wrap the underlying operation, to allow it to be used outside of sorting. Use SP_QSORT_* constants in kernels.

## Good
```c
s32 sp_str_sort_kernel_alphabetical(const void* a, const void* b) {
    return sp_str_compare_alphabetical(*(sp_str_t*)a, *(sp_str_t*)b);
}

s32 sp_str_compare_alphabetical(sp_str_t a, sp_str_t b) {
    u32 i = 0;
    while (true) {
        if (i >= a.len && i >= b.len) return SP_QSORT_EQUAL;
        if (i >= a.len)               return SP_QSORT_A_FIRST;
        if (i >= b.len)               return SP_QSORT_B_FIRST;

        if (a.data[i] == b.data[i]) {
            i++;
            continue;
        }
        if (a.data[i] > b.data[i]) return SP_QSORT_B_FIRST;
        if (b.data[i] > a.data[i]) return SP_QSORT_A_FIRST;
    }
}

sp_dyn_array(sp_str_t) files = get_file_list();
qsort(files, sp_dyn_array_size(files), sizeof(sp_str_t),
      sp_str_sort_kernel_alphabetical);
```

## Bad
```c
int compare_strings(const void* a, const void* b) {
    char* str_a = *(char**)a;
    char* str_b = *(char**)b;

    int result = strcmp(str_a, str_b);
    if (result < 0) return -1;
    if (result > 0) return 1;
    return 0;
}
qsort(files, file_count, sizeof(char*), compare_strings);
```

# Tags
- api.strings.sp_str_t.comparison
- api.sp_dynamic_array_t
