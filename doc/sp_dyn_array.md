Use sp_dyn_array to build dynamic arrays.

## GOOD

```c
sp_dyn_array(sp_str_t) args = SP_NULLPTR;
sp_dyn_array_push(args, SP_LIT("clang"));
sp_dyn_array_push(args, SP_LIT("-c"));

sp_dyn_array_for(args, i) {
  sp_str_t arg = args[i];
}
```

# Tags
- api.sp_dynamic_array_t
