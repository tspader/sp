int initialized = 100;
int uninitialized;
static int static_var;

void set_values(int a, int b) {
    uninitialized = a;
    static_var = b;
}

int get_sum(void) {
    return initialized + uninitialized + static_var;
}
