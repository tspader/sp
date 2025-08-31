Use SP_MACRO_* utilities for metaprogramming and code generation.

## Good
```c
#define MY_FUNC_NAME(type) SP_MACRO_CAT(process_, type)
#define MY_STRUCT_NAME(name) SP_MACRO_CAT(name, _data_t)

MY_FUNC_NAME(int) process_int(int value);
MY_FUNC_NAME(float) process_float(float value);

typedef struct MY_STRUCT_NAME(player) {
    u32 id;
    sp_str_t name;
} player_data_t;

#define LOG_VAR(var) \
    SP_LOG("{} = {}", SP_MACRO_STR(var), var)

u32 count = 42;
LOG_VAR(count);

#define DECLARE_HANDLE(name) \
    typedef struct SP_MACRO_CAT(name, _impl_t)* name##_t

DECLARE_HANDLE(window);
DECLARE_HANDLE(renderer);
```

## Bad
```c
#define PROCESS_INT_FUNC process_int
#define PROCESS_FLOAT_FUNC process_float

void process_int(int value);
void process_float(float value);

typedef struct player_data_t {
    u32 id;
    sp_str_t name;
} player_data_t;

printf("count = %d\n", count);

typedef struct window_impl_t* window_t;
typedef struct renderer_impl_t* renderer_t;
```
