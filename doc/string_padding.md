Use sp_str_pad() to pad strings to a fixed width for aligned output

## Good
```c
sp_str_t name = SP_LIT("jerry");
sp_str_t status = sp_str_pad(SP_LIT("guitar"), 10);
SP_LOG("{:pad 20} {} {}", SP_FMT_STR(name), SP_FMT_STR(status), SP_FMT_STR(dep->path));

// aligning array of strings to longest
sp_str_t names[] = {
  SP_LIT("alice"),
  SP_LIT("bob"),
  SP_LIT("charlotte")
};
sp_dyn_array(sp_str_t) padded = sp_str_pad_to_longest(names, 3);
// results in ["alice    ", "bob      ", "charlotte"]
```

## Bad
```c
// manual padding with spaces
sp_str_t name_with_spaces = sp_str_concat(name, SP_LIT("     "));

// using printf width specifiers with sp_str_t
printf("%-20s", sp_str_to_cstr(name));

// calculating padding manually
s32 padding = 20 - name.len;
for (s32 i = 0; i < padding; i++) {
    sp_str_builder_append_c8(&builder, ' ');
}
```

# Tags
- api.strings.sp_str_t.manipulation
- api.strings.sp_str_pad
- api.strings.sp_str_pad_to_longest
