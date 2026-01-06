/*
 * sp_libc.h - nostdlib implementations of memcpy, memmove, memcmp
 *
 * Derived from musl libc (https://musl.libc.org/)
 * Copyright 2005-2020 Rich Felker, et al.
 * SPDX-License-Identifier: MIT
 *
 * aarch64 memcpy: Copyright 1999-2019 Arm Limited
 * x86_64 memcpy/memmove: Copyright Nicholas J. Kain
 */

#ifndef SP_LIBC_H
#define SP_LIBC_H

#include <stddef.h>
#include <stdint.h>

//////////////////
// ARCHITECTURE //
//////////////////
#if defined(__x86_64__) || defined(_M_X64)
  #define SP_AMD64
  #define SP_AMD
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
  #define SP_ARM64
  #define SP_ARM
#endif

#if !defined(SP_AMD64) && !defined(SP_ARM64)
  #error "sp_libc.h: unsupported architecture (requires x86_64 or aarch64)"
#endif

/////////////
// MEMCPY  //
/////////////

#if defined(SP_AMD64)

/*
 * x86_64 memcpy - uses rep movsq for bulk copy
 * Aligns destination to 8 bytes, then copies 8 bytes at a time
 */
static inline void *sp_libc_memcpy(void *restrict dest, const void *restrict src, size_t n) {
    void *ret = dest;
    __asm__ __volatile__ (
        "cmp $8, %[n]\n\t"
        "jc 2f\n\t"
        "test $7, %%edi\n\t"
        "jz 1f\n"
        "3:\n\t"
        "movsb\n\t"
        "dec %[n]\n\t"
        "test $7, %%edi\n\t"
        "jnz 3b\n"
        "1:\n\t"
        "mov %[n], %%rcx\n\t"
        "shr $3, %%rcx\n\t"
        "rep movsq\n\t"
        "and $7, %[n]\n\t"
        "jz 4f\n"
        "2:\n\t"
        "movsb\n\t"
        "dec %[n]\n\t"
        "jnz 2b\n"
        "4:\n"
        : [n] "+d" (n), "+D" (dest), "+S" (src)
        :
        : "rcx", "memory"
    );
    return ret;
}

#elif defined(SP_ARM64)

/*
 * aarch64 memcpy - uses ldp/stp for bulk copy
 * Handles small/medium/large copies with different strategies
 */
static inline void *sp_libc_memcpy(void *restrict dest, const void *restrict src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (n == 0) return dest;

    /* Small copies: use simple loop for < 16 bytes */
    if (n < 16) {
        while (n--) *d++ = *s++;
        return dest;
    }

    /* Medium/large copies: use ldp/stp pairs */
    uint64_t *d64;
    const uint64_t *s64;

    /* Copy first 16 bytes (may overlap with aligned part) */
    __asm__ __volatile__ (
        "ldp x4, x5, [%[src]]\n\t"
        "stp x4, x5, [%[dst]]"
        : : [src] "r" (s), [dst] "r" (d)
        : "x4", "x5", "memory"
    );

    if (n <= 32) {
        /* Copy last 16 bytes (overlapping is fine) */
        __asm__ __volatile__ (
            "ldp x4, x5, [%[src]]\n\t"
            "stp x4, x5, [%[dst]]"
            : : [src] "r" (s + n - 16), [dst] "r" (d + n - 16)
            : "x4", "x5", "memory"
        );
        return dest;
    }

    /* Align destination to 16 bytes */
    size_t align = 16 - ((uintptr_t)d & 15);
    if (align < 16) {
        d += align;
        s += align;
        n -= align;
    }

    /* Bulk copy 64 bytes at a time */
    d64 = (uint64_t *)d;
    s64 = (const uint64_t *)s;
    while (n >= 64) {
        __asm__ __volatile__ (
            "ldp x4, x5, [%[src]]\n\t"
            "ldp x6, x7, [%[src], #16]\n\t"
            "ldp x8, x9, [%[src], #32]\n\t"
            "ldp x10, x11, [%[src], #48]\n\t"
            "stp x4, x5, [%[dst]]\n\t"
            "stp x6, x7, [%[dst], #16]\n\t"
            "stp x8, x9, [%[dst], #32]\n\t"
            "stp x10, x11, [%[dst], #48]"
            : : [src] "r" (s64), [dst] "r" (d64)
            : "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "memory"
        );
        d64 += 8;
        s64 += 8;
        n -= 64;
    }

    /* Handle remainder */
    d = (unsigned char *)d64;
    s = (const unsigned char *)s64;

    if (n >= 32) {
        __asm__ __volatile__ (
            "ldp x4, x5, [%[src]]\n\t"
            "ldp x6, x7, [%[src], #16]\n\t"
            "stp x4, x5, [%[dst]]\n\t"
            "stp x6, x7, [%[dst], #16]"
            : : [src] "r" (s), [dst] "r" (d)
            : "x4", "x5", "x6", "x7", "memory"
        );
        d += 32; s += 32; n -= 32;
    }
    if (n >= 16) {
        __asm__ __volatile__ (
            "ldp x4, x5, [%[src]]\n\t"
            "stp x4, x5, [%[dst]]"
            : : [src] "r" (s), [dst] "r" (d)
            : "x4", "x5", "memory"
        );
        d += 16; s += 16; n -= 16;
    }

    /* Copy remaining bytes */
    while (n--) *d++ = *s++;

    return dest;
}

#endif /* SP_AMD64 / SP_ARM64 */


//////////////
// MEMMOVE  //
//////////////

#if defined(SP_AMD64)

/*
 * x86_64 memmove - forwards to memcpy if no overlap, otherwise copies backward
 */
static inline void *sp_libc_memmove(void *dest, const void *src, size_t n) {
    void *ret = dest;

    /* Check if we can use forward copy (no overlap or src > dest) */
    if ((uintptr_t)dest - (uintptr_t)src >= n) {
        return sp_libc_memcpy(dest, src, n);
    }

    /* Backward copy using std + rep movsb */
    __asm__ __volatile__ (
        "lea -1(%[dest], %[n]), %%rdi\n\t"
        "lea -1(%[src], %[n]), %%rsi\n\t"
        "mov %[n], %%rcx\n\t"
        "std\n\t"
        "rep movsb\n\t"
        "cld"
        :
        : [dest] "r" (dest), [src] "r" (src), [n] "r" (n)
        : "rdi", "rsi", "rcx", "memory"
    );

    return ret;
}

#elif defined(SP_ARM64)

/*
 * aarch64 memmove - C implementation
 * Copies forward or backward depending on overlap
 */
static inline void *sp_libc_memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d == s || n == 0) return dest;

    /* No overlap - use memcpy */
    if ((uintptr_t)s - (uintptr_t)d - n <= -2*n) {
        return sp_libc_memcpy(dest, src, n);
    }

    if (d < s) {
        /* Forward copy */
        /* Align to 8 bytes if possible */
        if ((uintptr_t)s % 8 == (uintptr_t)d % 8) {
            while ((uintptr_t)d % 8) {
                if (!n--) return dest;
                *d++ = *s++;
            }
            /* Copy 8 bytes at a time */
            while (n >= 8) {
                *(uint64_t *)d = *(const uint64_t *)s;
                d += 8; s += 8; n -= 8;
            }
        }
        while (n--) *d++ = *s++;
    } else {
        /* Backward copy */
        /* Align end to 8 bytes if possible */
        if ((uintptr_t)s % 8 == (uintptr_t)d % 8) {
            while ((uintptr_t)(d + n) % 8) {
                if (!n--) return dest;
                d[n] = s[n];
            }
            while (n >= 8) {
                n -= 8;
                *(uint64_t *)(d + n) = *(const uint64_t *)(s + n);
            }
        }
        while (n--) d[n] = s[n];
    }

    return dest;
}

#endif /* SP_AMD64 / SP_ARM64 */


//////////////
// MEMCMP   //
//////////////

/*
 * memcmp - C implementation (same for all architectures)
 * Returns <0, 0, or >0 based on comparison
 */
static inline int sp_libc_memcmp(const void *vl, const void *vr, size_t n) {
    const unsigned char *l = vl;
    const unsigned char *r = vr;

    /* Fast path: compare 8 bytes at a time while equal */
    while (n >= 8) {
        uint64_t lv = *(const uint64_t *)l;
        uint64_t rv = *(const uint64_t *)r;
        if (lv != rv) break;
        l += 8; r += 8; n -= 8;
    }

    /* Compare remaining bytes */
    for (; n && *l == *r; n--, l++, r++);

    return n ? *l - *r : 0;
}


//////////////
// MEMSET   //
//////////////

static inline void *sp_libc_memset(void *dest, int c, size_t n) {
    unsigned char *d = dest;
    unsigned char v = (unsigned char)c;

#if defined(SP_AMD64)
    if (n >= 8) {
        uint64_t v64 = v;
        v64 |= v64 << 8;
        v64 |= v64 << 16;
        v64 |= v64 << 32;

        /* Align to 8 bytes */
        while ((uintptr_t)d & 7) {
            *d++ = v;
            n--;
        }

        /* Fill 8 bytes at a time */
        uint64_t *d64 = (uint64_t *)d;
        while (n >= 8) {
            *d64++ = v64;
            n -= 8;
        }
        d = (unsigned char *)d64;
    }
#endif

    while (n--) *d++ = v;
    return dest;
}


//////////////
// SYSCALLS //
//////////////

#if defined(SP_AMD64)

/* x86_64 syscall numbers */
#define SP_SYS_read           0
#define SP_SYS_write          1
#define SP_SYS_open           2
#define SP_SYS_close          3
#define SP_SYS_mmap           9
#define SP_SYS_munmap         11
#define SP_SYS_mremap         25
#define SP_SYS_exit           60
#define SP_SYS_arch_prctl     158
#define SP_SYS_exit_group     231

#define SP_ARCH_SET_FS        0x1002

static inline int64_t sp_syscall0(int64_t n) {
    int64_t ret;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t sp_syscall1(int64_t n, int64_t a1) {
    int64_t ret;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t sp_syscall2(int64_t n, int64_t a1, int64_t a2) {
    int64_t ret;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t sp_syscall3(int64_t n, int64_t a1, int64_t a2, int64_t a3) {
    int64_t ret;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t sp_syscall4(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4) {
    int64_t ret;
    register int64_t r10 __asm__("r10") = a4;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t sp_syscall5(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5) {
    int64_t ret;
    register int64_t r10 __asm__("r10") = a4;
    register int64_t r8  __asm__("r8")  = a5;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t sp_syscall6(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6) {
    int64_t ret;
    register int64_t r10 __asm__("r10") = a4;
    register int64_t r8  __asm__("r8")  = a5;
    register int64_t r9  __asm__("r9")  = a6;
    __asm__ __volatile__ ("syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory");
    return ret;
}

#elif defined(SP_ARM64)

/* aarch64 syscall numbers */
#define SP_SYS_read           63
#define SP_SYS_write          64
#define SP_SYS_open           56   /* openat with AT_FDCWD */
#define SP_SYS_close          57
#define SP_SYS_mmap           222
#define SP_SYS_munmap         215
#define SP_SYS_mremap         216
#define SP_SYS_exit           93
#define SP_SYS_exit_group     94

static inline int64_t sp_syscall0(int64_t n) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0");
    __asm__ __volatile__ ("svc 0" : "=r"(x0) : "r"(x8) : "memory");
    return x0;
}

static inline int64_t sp_syscall1(int64_t n, int64_t a1) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0") = a1;
    __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8) : "memory");
    return x0;
}

static inline int64_t sp_syscall2(int64_t n, int64_t a1, int64_t a2) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0") = a1;
    register int64_t x1 __asm__("x1") = a2;
    __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1) : "memory");
    return x0;
}

static inline int64_t sp_syscall3(int64_t n, int64_t a1, int64_t a2, int64_t a3) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0") = a1;
    register int64_t x1 __asm__("x1") = a2;
    register int64_t x2 __asm__("x2") = a3;
    __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");
    return x0;
}

static inline int64_t sp_syscall4(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0") = a1;
    register int64_t x1 __asm__("x1") = a2;
    register int64_t x2 __asm__("x2") = a3;
    register int64_t x3 __asm__("x3") = a4;
    __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3) : "memory");
    return x0;
}

static inline int64_t sp_syscall5(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0") = a1;
    register int64_t x1 __asm__("x1") = a2;
    register int64_t x2 __asm__("x2") = a3;
    register int64_t x3 __asm__("x3") = a4;
    register int64_t x4 __asm__("x4") = a5;
    __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4) : "memory");
    return x0;
}

static inline int64_t sp_syscall6(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6) {
    register int64_t x8 __asm__("x8") = n;
    register int64_t x0 __asm__("x0") = a1;
    register int64_t x1 __asm__("x1") = a2;
    register int64_t x2 __asm__("x2") = a3;
    register int64_t x3 __asm__("x3") = a4;
    register int64_t x4 __asm__("x4") = a5;
    register int64_t x5 __asm__("x5") = a6;
    __asm__ __volatile__ ("svc 0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5) : "memory");
    return x0;
}

#endif /* SP_AMD64 / SP_ARM64 */


//////////////////////
// MMAP / ALLOCATOR //
//////////////////////

/* mmap prot flags */
#define SP_PROT_NONE      0
#define SP_PROT_READ      1
#define SP_PROT_WRITE     2
#define SP_PROT_EXEC      4

/* mmap flags */
#define SP_MAP_SHARED     0x01
#define SP_MAP_PRIVATE    0x02
#define SP_MAP_FIXED      0x10
#define SP_MAP_ANONYMOUS  0x20

#define SP_MAP_FAILED     ((void*)-1)

static inline void *sp_libc_mmap(void *addr, size_t len, int prot, int flags, int fd, int64_t offset) {
    return (void*)sp_syscall6(SP_SYS_mmap, (int64_t)addr, (int64_t)len, prot, flags, fd, offset);
}

static inline int sp_libc_munmap(void *addr, size_t len) {
    return (int)sp_syscall2(SP_SYS_munmap, (int64_t)addr, (int64_t)len);
}

static inline void *sp_libc_mremap(void *old_addr, size_t old_size, size_t new_size, int flags) {
    return (void*)sp_syscall4(SP_SYS_mremap, (int64_t)old_addr, (int64_t)old_size, (int64_t)new_size, flags);
}

/*
 * Simple mmap-based allocator
 * Stores size in header for munmap. Each allocation is its own mmap region.
 * Suitable for arena-style allocation where individual frees are rare.
 */
#define SP_ALLOC_ALIGN    16
#define SP_ALLOC_HEADER   16  /* Must be >= sizeof(size_t) and aligned */

static inline void *sp_libc_alloc(size_t n) {
    if (n == 0) n = 1;
    n = (n + SP_ALLOC_ALIGN - 1) & ~(SP_ALLOC_ALIGN - 1);

    size_t total = n + SP_ALLOC_HEADER;
    void *p = sp_libc_mmap(0, total, SP_PROT_READ | SP_PROT_WRITE,
                           SP_MAP_PRIVATE | SP_MAP_ANONYMOUS, -1, 0);
    if (p == SP_MAP_FAILED) return 0;

    *(size_t*)p = total;
    return (char*)p + SP_ALLOC_HEADER;
}

static inline void *sp_libc_alloc_zero(size_t n) {
    /* mmap already returns zeroed memory */
    return sp_libc_alloc(n);
}

static inline void sp_libc_free(void *ptr) {
    if (!ptr) return;
    void *base = (char*)ptr - SP_ALLOC_HEADER;
    size_t total = *(size_t*)base;
    sp_libc_munmap(base, total);
}

static inline void *sp_libc_realloc(void *ptr, size_t new_size) {
    if (!ptr) return sp_libc_alloc(new_size);
    if (new_size == 0) {
        sp_libc_free(ptr);
        return 0;
    }

    void *base = (char*)ptr - SP_ALLOC_HEADER;
    size_t old_total = *(size_t*)base;
    size_t old_size = old_total - SP_ALLOC_HEADER;

    new_size = (new_size + SP_ALLOC_ALIGN - 1) & ~(SP_ALLOC_ALIGN - 1);
    size_t new_total = new_size + SP_ALLOC_HEADER;

    /* Try mremap first (Linux-specific, may fail) */
    void *new_base = sp_libc_mremap(base, old_total, new_total, 1 /* MREMAP_MAYMOVE */);
    if (new_base != SP_MAP_FAILED) {
        *(size_t*)new_base = new_total;
        return (char*)new_base + SP_ALLOC_HEADER;
    }

    /* Fallback: allocate new, copy, free old */
    void *new_ptr = sp_libc_alloc(new_size);
    if (!new_ptr) return 0;

    size_t copy_size = old_size < new_size ? old_size : new_size;
    sp_libc_memcpy(new_ptr, ptr, copy_size);
    sp_libc_free(ptr);
    return new_ptr;
}


/////////
// TLS //
/////////

/*
 * Thread Pointer access
 * The TP register points to a structure that contains thread-local data.
 * We store sp_tls_rt_t* at offset 8 from the TP base.
 */

#if defined(SP_AMD64)

static inline void *sp_libc_get_tp(void) {
    void *tp;
    __asm__ __volatile__ ("mov %%fs:0, %0" : "=r"(tp));
    return tp;
}

static inline int sp_libc_set_tp(void *tp) {
    return (int)sp_syscall2(SP_SYS_arch_prctl, SP_ARCH_SET_FS, (int64_t)tp);
}

#elif defined(SP_ARM64)

static inline void *sp_libc_get_tp(void) {
    void *tp;
    __asm__ __volatile__ ("mrs %0, tpidr_el0" : "=r"(tp));
    return tp;
}

static inline void sp_libc_set_tp(void *tp) {
    __asm__ __volatile__ ("msr tpidr_el0, %0" : : "r"(tp) : "memory");
}

#endif

/*
 * Thread block structure - lives at the TP address
 * The 'self' pointer at offset 0 is required for x86_64 FS:0 access.
 */
typedef struct sp_libc_thread_block {
    struct sp_libc_thread_block *self;  /* Points to itself, at FS:0 / TP */
    void *tls_data;                      /* User TLS data pointer */
} sp_libc_thread_block_t;

static inline void *sp_libc_tls_get(void) {
    sp_libc_thread_block_t *tb = (sp_libc_thread_block_t*)sp_libc_get_tp();
    return tb ? tb->tls_data : 0;
}

static inline void sp_libc_tls_set(void *data) {
    sp_libc_thread_block_t *tb = (sp_libc_thread_block_t*)sp_libc_get_tp();
    if (tb) tb->tls_data = data;
}


////////////
// EXIT   //
////////////

static inline void sp_libc_exit(int code) {
    sp_syscall1(SP_SYS_exit_group, code);
    __builtin_unreachable();
}


/////////////////////
// LIBC WRAPPERS   //
/////////////////////

/* Define these to replace libc functions */
#ifdef SP_LIBC_REPLACE

#define memcpy  sp_libc_memcpy
#define memmove sp_libc_memmove
#define memcmp  sp_libc_memcmp
#define memset  sp_libc_memset

#endif /* SP_LIBC_REPLACE */

#endif /* SP_LIBC_H */
