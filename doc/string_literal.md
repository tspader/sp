Use SP_LIT() to wrap C string literals in a sp_str_t

## Good
```c
sp_str_t path = SP_LIT("/home/user");
sp_str_t delimiter = SP_LIT(", ");
sp_str_builder_append(&builder, SP_LIT("Error: "));

if (sp_str_equal(ext, SP_LIT(".txt"))) {
    process_text_file(file);
}
```

## Bad
```c
sp_str_t path = sp_str_from_cstr("/home/user");

const char* delim = ", ";
sp_str_t delimiter = sp_str(delim, strlen(delim));

sp_str_builder_append_cstr(&builder, "Error: ");
```
