/*
 * test_sp_libc.c - nostdlib test harness for sp_libc.h
 *
 * Build and run:
 *   gcc -nostdlib -static -o test_sp_libc test_sp_libc.c && ./test_sp_libc && echo "PASS"
 *
 * Exit code = number of failures (0 = all tests passed)
 */

#include "sp_libc.h"

/* Simple output for debugging (writes to stderr) */
static void print(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    sp_syscall3(SP_SYS_write, 2, (int64_t)s, (int64_t)len);
}

static void print_ok(const char *name) {
    print("  [OK] ");
    print(name);
    print("\n");
}

static void print_fail(const char *name) {
    print("  [FAIL] ");
    print(name);
    print("\n");
}

#define TEST(name, cond) do { \
    if (cond) { print_ok(name); } \
    else { print_fail(name); failures++; } \
} while(0)

void _start(void) {
    int failures = 0;

    print("=== sp_libc.h tests ===\n");

    /* memcpy tests */
    {
        char src[64] = "hello world";
        char dst[64] = {0};
        sp_libc_memcpy(dst, src, 12);
        TEST("memcpy basic", sp_libc_memcmp(dst, src, 12) == 0);

        /* Large copy */
        char big_src[256];
        char big_dst[256];
        for (int i = 0; i < 256; i++) big_src[i] = (char)i;
        sp_libc_memcpy(big_dst, big_src, 256);
        TEST("memcpy 256 bytes", sp_libc_memcmp(big_dst, big_src, 256) == 0);
    }

    /* memmove tests */
    {
        char buf[32] = "0123456789";
        sp_libc_memmove(buf + 2, buf, 8);  /* overlapping forward */
        TEST("memmove overlap fwd", sp_libc_memcmp(buf + 2, "01234567", 8) == 0);

        char buf2[32] = "0123456789";
        sp_libc_memmove(buf2, buf2 + 2, 8);  /* overlapping backward */
        TEST("memmove overlap bwd", sp_libc_memcmp(buf2, "23456789", 8) == 0);
    }

    /* memcmp tests */
    {
        TEST("memcmp equal", sp_libc_memcmp("abc", "abc", 3) == 0);
        TEST("memcmp less", sp_libc_memcmp("abc", "abd", 3) < 0);
        TEST("memcmp greater", sp_libc_memcmp("abd", "abc", 3) > 0);
        TEST("memcmp zero len", sp_libc_memcmp("abc", "xyz", 0) == 0);
    }

    /* memset tests */
    {
        char buf[32];
        sp_libc_memset(buf, 'x', 16);
        int ok = 1;
        for (int i = 0; i < 16; i++) if (buf[i] != 'x') ok = 0;
        TEST("memset", ok);
    }

    /* mmap tests */
    {
        void *p = sp_libc_mmap(0, 4096, SP_PROT_READ | SP_PROT_WRITE,
                               SP_MAP_PRIVATE | SP_MAP_ANONYMOUS, -1, 0);
        TEST("mmap", p != SP_MAP_FAILED);

        if (p != SP_MAP_FAILED) {
            /* Verify memory is zeroed and writable */
            char *c = (char*)p;
            int zeroed = (c[0] == 0 && c[4095] == 0);
            c[0] = 'A';
            c[4095] = 'Z';
            int writable = (c[0] == 'A' && c[4095] == 'Z');
            TEST("mmap zeroed", zeroed);
            TEST("mmap writable", writable);

            int ret = sp_libc_munmap(p, 4096);
            TEST("munmap", ret == 0);
        }
    }

    /* allocator tests */
    {
        void *a = sp_libc_alloc(100);
        TEST("alloc", a != 0);

        if (a) {
            sp_libc_memset(a, 0x42, 100);
            TEST("alloc writable", ((char*)a)[0] == 0x42 && ((char*)a)[99] == 0x42);

            void *b = sp_libc_realloc(a, 200);
            TEST("realloc", b != 0);

            if (b) {
                /* First 100 bytes should be preserved */
                TEST("realloc preserves", ((char*)b)[0] == 0x42 && ((char*)b)[99] == 0x42);
                sp_libc_free(b);
            }
        }

        /* alloc_zero test */
        void *z = sp_libc_alloc_zero(64);
        TEST("alloc_zero", z != 0);
        if (z) {
            int all_zero = 1;
            for (int i = 0; i < 64; i++) if (((char*)z)[i] != 0) all_zero = 0;
            TEST("alloc_zero zeroed", all_zero);
            sp_libc_free(z);
        }
    }

    /* TLS tests */
    {
        /* Create a thread block on stack */
        sp_libc_thread_block_t tb;
        tb.self = &tb;
        tb.tls_data = (void*)0xDEADBEEF;

        /* Set TP to point to our thread block */
        int ret = sp_libc_set_tp(&tb);
        TEST("set_tp", ret == 0);

        /* Read back TP */
        void *tp = sp_libc_get_tp();
        TEST("get_tp", tp == &tb);

        /* Test TLS data access */
        void *data = sp_libc_tls_get();
        TEST("tls_get", data == (void*)0xDEADBEEF);

        sp_libc_tls_set((void*)0xCAFEBABE);
        data = sp_libc_tls_get();
        TEST("tls_set", data == (void*)0xCAFEBABE);
    }

    print("=== done ===\n");

    if (failures == 0) {
        print("All tests passed!\n");
    } else {
        print("Some tests failed.\n");
    }

    sp_libc_exit(failures);
}
