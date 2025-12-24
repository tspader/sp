extern int external_func(int x);
extern int external_var;

int call_external(int x) {
    return external_func(x) + external_var;
}
