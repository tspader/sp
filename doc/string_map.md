Use sp_str_map() with kernels for functional string transformations.

## Good
```c
sp_str_t lines[] = {
    SP_LIT("  hello world  "),
    SP_LIT("\ttest line\n"),
    SP_LIT("  another  "),
};

sp_dyn_array(sp_str_t) trimmed = sp_str_map(lines, 3, NULL, sp_str_map_kernel_trim);

sp_dyn_array(sp_str_t) upper = sp_str_map(lines, 3, NULL, sp_str_map_kernel_to_upper);

u32 width = 20;
sp_dyn_array(sp_str_t) padded = sp_str_map(lines, 3, &width, sp_str_map_kernel_pad);

sp_str_t longest = sp_str_find_longest_n(lines, 3);
sp_dyn_array(sp_str_t) aligned = sp_str_pad_to_longest(lines, 3);
```

## Bad
```c
sp_str_t* trimmed = malloc(sizeof(sp_str_t) * 3);
for (int i = 0; i < 3; i++) {
    trimmed[i] = sp_str_trim(lines[i]);
}

sp_str_t* upper = malloc(sizeof(sp_str_t) * 3);
for (int i = 0; i < 3; i++) {
    upper[i] = sp_str_to_upper(lines[i]);
}

sp_str_t* padded = malloc(sizeof(sp_str_t) * 3);
for (int i = 0; i < 3; i++) {
    padded[i] = sp_str_pad(lines[i], 20);
}
```