# SP_QSORT_COMPARE
```c
SP_QSORT_COMPARE(a, b) expands to if (a > b) return a first; ditto for b; nothing if equal
SP_QSORT_COMPARE_MEMORY(a, b)
```

# sp_format_context_t
```c
s32 foo = 69;
s32 bar = 77;

sp_format_context_t fmt = SP_ZERO_INITIALIZE();
SP_FMT_REGISTER_S32(&fmt, foo);
sp_format_context_register(&fmt, SP_FMT_S32(foo));
sp_format("foo is {foo}");
```

# sp_str_builder_t
- always allocate an extra byte so calls to write_cstr() (or perhaps some new API which clearly indicates that you're not copying) can be cheap; you always know you have room to zero out a byte behind the string to create a valid cstr

> Topic Map

  - Implementation gating & multi-TU safety — SP_IMPLEMENTATION, STB_DS_IMPLEMENTATION, STB_IMAGE_IMPLEMENTATION, GS_IMPL, SOKOL_APP_IMPL
  - API linkage & symbol visibility — SP_API, STBIDEF, GS_API_DECL, SOKOL_APP_API_DECL
  - Global state & thread safety — SP_THREAD_LOCAL, stbds_hash_seed, stbi__g_failure_reason, _gs_instance, _sapp
  - Memory strategy & zero-allocation hooks — sp_alloc, STBDS_REALLOC, STBI_MALLOC, gs_malloc, sapp_allocator
  - Configuration surface & feature toggles — SP_OS_BACKEND, STBDS_NO_*, STBI_NO_*, GS_NO_*, SOKOL_*

  Implementation Gating

  - stb_ds.h: Emphasizes single translation-unit inclusion via #define STB_DS_IMPLEMENTATION, with real code guarded under #ifdef STB_DS_IMPLEMENTATION and only extern declarations visible otherwise, preventing duplicate symbols in multi-TU builds
  external/stb/stb_ds.h:9-12,732-503.
  - stb_image.h: Same pattern with STB_IMAGE_IMPLEMENTATION, wrapping heavyweight code in the implementation block while declarations stay outside external/stb/stb_image.h:4-13,547-585.
  - gs.h: Requires GS_IMPL in exactly one TU; implementation lives behind the guard, keeping the header consumable elsewhere external/gunslinger/gs.h:58-68,7268.
  - sokol_app.h: Auto-upgrades SOKOL_IMPL to SOKOL_APP_IMPL, funnelling actual code behind #ifdef SOKOL_APP_IMPL to avoid accidental multiple definitions external/sokol/sokol_app.h:1-14,2104-2119.
  - sp.h comparison: Declares SP_IMPLEMENTATION, but key thread-local globals (sp_context_stack, sp_context) sit outside the implementation guard, so every TU gets an external-definition symbol that will collide at link time sp.h:402-403,1337-1341.

  API Linkage & Visibility

  - stb_ds.h: Internal helpers remain ordinary extern functions with macro front-ends, keeping the public surface purely macro-based and avoiding exported symbols unless needed external/stb/stb_ds.h:494-502.
  - stb_image.h: STBIDEF lets consumers choose static or exported linkage (and DLL decorations) before including the header external/stb/stb_image.h:394-399.
  - gs.h: GS_API_DECL centralizes linkage attributes (extern vs dllexport) and gs_global defaults to static, so users can retarget visibility before inclusion external/gunslinger/gs.h:506-553.
  - sokol_app.h: Similar SOKOL_APP_API_DECL indirection, mapping to extern by default and supporting DLL/export conventions automatically external/sokol/sokol_app.h:1324-1334.
  - sp.h comparison: Leaves SP_API empty, so every declaration defaults to external linkage with no simple switch to static or dllexport, and users must patch the header to change it sp.h:267-269.

  Global State & Thread Safety

  - stb_ds.h: Keeps seeds and statistics static so each TU owns its copy unless the implementation unit exports counters explicitly, sidestepping ODR issues external/stb/stb_ds.h:736-758,849-950.
  - stb_image.h: Failure state is static (optionally thread-local) and only lives in the implementation unit, so helpers remain reentrant-friendly when thread-local storage is enabled external/stb/stb_image.h:966-980.
  - gs.h: Framework singleton _gs_instance is gs_global (default static) so each TU builds cleanly, assuming the library really does run from a single compilation unit external/gunslinger/gs.h:506-528,9867-9888.
  - sokol_app.h: Core runtime data _sapp is static, consolidating state inside the implementation TU while exposing accessors only through API external/sokol/sokol_app.h:3126-3146.
  - sp.h comparison: Thread-local context stack/globals are defined with external linkage in the header; multi-TU builds would emit multiple strong definitions, and even single-TU builds rely on manual sp_init to populate the stack before any allocator-
  using API sp.h:402-407,2683-2706.

  Memory Strategy & Zero-Allocation Hooks

  - stb_ds.h: Lets callers override STBDS_REALLOC/STBDS_FREE, defaults to realloc/free, and grows containers lazily so the “array pointer = NULL” idiom costs nothing until first use external/stb/stb_ds.h:44-52,455-462,785-915.
  - stb_image.h: Requires a heap buffer but allows full allocator substitution via STBI_MALLOC/STBI_REALLOC/STBI_FREE; thread-local toggles also avoid static buffers for multithreaded scenarios external/stb/stb_image.h:673-688.
  - gs.h: Provides default aliases to CRT allocators but can route everything through a user-specified gs_os_api_t so projects can plug custom arenas or zero-allocation strategies external/gunslinger/gs.h:738-778.
  - sokol_app.h: Accepts allocator callbacks in sapp_desc and funnels all internal allocations through _sapp_malloc/_sapp_free, giving users a central hook for arenas or tracking external/sokol/sokol_app.h:1673-1684,3190-3213.
  - sp.h comparison: Context-based allocation is elegant for arenas, but the default init simply pushes malloc onto the context stack and assumes sp_context is valid; without wrapping the thread-local state in guards, any call before sp_init will
  dereference null, and there’s no per-call override macro akin to the other libraries sp.h:402-407,2701-2706,5317-5326.

  Configuration Surface

  - stb_ds.h: Enumerates feature toggles up front (STBDS_NO_SHORT_NAMES, STBDS_SIPHASH_2_4, statistics/unit tests), making opt-in costs and security knobs obvious external/stb/stb_ds.h:24-57.
  - stb_image.h: Offers a rich matrix of STBI_NO_*, STBI_ONLY_*, STBI_THREAD_LOCAL, STB_IMAGE_STATIC, and allocator/assert overrides so consumers can trim code size or change behaviour per TU external/stb/stb_image.h:4-16,547-579,966-980,673-688.
  - gs.h: Documents high-level flags (GS_NO_HIJACK_MAIN, GS_NO_OS_MEMORY_ALLOC_DEFAULT, DLL exports, platform hints) and uses #ifndef blocks so users can predefine alternatives without editing the file external/gunslinger/gs.h:56-118,755-778.
  - sokol_app.h: Lists backend selectors and behavioural toggles (SOKOL_GLCORE, SOKOL_NO_ENTRY, SOKOL_DEBUG, SOKOL_DLL, etc.) right next to the include instructions, encouraging deliberate configuration external/sokol/sokol_app.h:10-52.
  - sp.h comparison: Aside from picking the OS backend or enabling the optional SP_APP subsystem, most subsystems are always compiled in; there’s no way to drop logging, threading, or filesystem pieces from the single translation unit without editing the
  header sp.h:96-152,1225-1289.

  Opportunities for sp.h

  - Move the thread-local context definitions behind SP_IMPLEMENTATION (and emit extern declarations otherwise) to make multi-TU builds link cleanly sp.h:402-407,1337-1341.
  - Give SP_API a meaningful default (for example extern with an opt-in SP_STATIC) and mirror the STBIDEF/SOKOL_APP_API_DECL pattern so consumers can pick static vs exported linkage without touching the file sp.h:267-269.
  - Provide #define switches for major subsystems (logging, threading, file monitoring, SDL backend) to shrink the footprint in projects that only need core facilities, matching the configurability of the reference headers sp.h:96-152,5569-5715.
  - Guard allocator-dependent entry points so they either bootstrap a default context automatically or assert early when sp_init hasn’t been called; document the minimal init sequence for multi-threaded use sp.h:402-407,2701-2709,5317-5326.
  - Consider adding per-module static inline helpers for header-only convenience (e.g., string predicates) while leaving heavyweight routines in the SP_IMPLEMENTATION section, improving inlining flexibility similar to stb_ds’s macro front-ends sp.h:810-
  1180.

  Suggested next steps for you:

  1. Decide whether to support true multi-TU consumers; if so, refactor the global context declarations and add export-control macros.
  2. Sketch a configuration matrix (which subsystems should be optional) and introduce SP_NO_* flags to prune unwanted code paths.
  3. Expand the allocator documentation to show how to run with a preallocated arena or zero-allocation mode, taking cues from sapp_allocator and STBDS_REALLOC usage.
