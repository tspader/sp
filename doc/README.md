@spader: apologies, this is very much slop

# sp.h Project Overview

This directory contains the 'SP' project, a C/C++ library providing a collection of cross-platform utilities and data structures. Key features include custom memory allocators, string handling, dynamic arrays, hash tables, ring buffers, file monitoring, and formatting utilities. It also includes a testing framework based on `utest.h`. The project supports multiple OS backends, including native Win32/POSIX and SDL.

## Key Technologies & Architecture

*   **Language:** Primarily C11, with some C++20 features used (dual compilation support).
*   **Build System:** `Makefile` driven build, targeting different executables (C, C++, SDL, stress tests).
*   **Dependencies:**
    *   `SDL3` (as a submodule in `external/SDL`) for the SDL backend.
    *   `utest.h` (as a submodule in `external/utest`) for the testing framework.
*   **Core Concepts:**
    *   **Single Header Library:** The main library code is in `sp.h`. Implementation is included by defining `SP_IMPLEMENTATION`.
    *   **Custom Allocators:** Centralized memory management using `sp_allocator_t`, with implementations like `sp_bump_allocator_t` and `sp_allocator_malloc_t`.
    *   **Data Structures:** Provides `sp_dynamic_array_t`, `sp_fixed_array_t`, `sp_ht`, `sp_ring_buffer_t`.
    *   **String Handling:** A dedicated `sp_str_t` type and extensive functions for string manipulation, formatting, and building (`sp_str_builder_t`).
    *   **OS Abstraction:** Abstracts OS-specific functionalities (threads, mutexes, semaphores, file system operations) under a unified interface, switchable between native (Win32/POSIX) and SDL backends.
    *   **File Monitoring:** Includes `sp_file_monitor_t` for watching file system changes.
    *   **Formatting:** A flexible `sp_fmt` system for type-safe string formatting.
    *   **Hashing:** Utilities for hashing strings and bytes (`sp_hash_t`).

## Building and Running

The project uses a `Makefile` for building. Key targets are defined within the `Makefile`.

**Prerequisites:**

*   `gcc` and `g++` compilers.
*   `make`.
*   `cmake` (for building the SDL dependency).
*   `bear` (optional, for generating `compile_commands.json` for clangd).

**Build Commands (from the project root):**

*   `make all`: Builds all targets (C, C++, SDL, stress tests) and generates `compile_commands.json` for clangd.
*   `make c`: Builds the C test executable (`build/bin/sp-c`).
*   `make cpp`: Builds the C++ test executable (`build/bin/sp-cpp`).
*   `make sdl`: Builds the SDL test executable (`build/bin/sp-sdl`). This also builds the SDL3 shared library.
*   `make stress`: Builds the stress test executable (`build/bin/sp-stress`).
*   `make build`: Builds all main targets (equivalent to `make c cpp stress sdl`).
*   `make test`: Builds all targets and runs the test suite for each.
*   `make clangd`: Generates `compile_commands.json` for clangd LSP support.

**Clean Commands:**

*   `make clean`: Removes the output binaries (`build/bin`).
*   `make nuke`: Removes the entire build directory (`build`).

## Development Conventions

*   **Testing:** Unit tests are written using the `utest.h` framework within `test.c`.
*   **Memory Management:** Prefer using the custom allocator system (`sp_alloc`, `sp_free`, etc.) over direct `malloc/free`.
*   **String Handling:** Use `sp_str_t` and associated functions for string operations.
*   **Data Structures:** Utilize the provided generic data structures (`sp_dynamic_array_t`, `sp_ht`, `sp_ring_buffer_t`).
*   **Formatting:** Use the `sp_fmt` system for creating formatted strings.
*   **OS Interaction:** Use the abstraction layer functions in `sp.h` (e.g., `sp_os_*`, `sp_thread_*`, `sp_mutex_*`, `sp_semaphore_*`) for portability.



# sp.h Examples and Patterns

A comprehensive collection of examples demonstrating idiomatic usage of sp.h APIs.

## Quick Start Examples
Start here if you're new to sp.h:

1. [zero_initialize.md](zero_initialize.md) - Zero initialization patterns
2. [string_literal.md](string_literal.md) - String literal macros
3. [format_string.md](format_string.md) - Type-safe string formatting
4. [dynamic_array.md](dynamic_array.md) - Gunslinger-style dynamic arrays
5. [path_operations.md](path_operations.md) - Cross-platform path manipulation

## Core Patterns

### Memory Management
- [context_allocator.md](context_allocator.md) - Context-based allocation
- [bump_allocator.md](bump_allocator.md) - Fast temporary allocations

### String Operations
- [string_builder_newline.md](string_builder_newline.md) - String builder with indentation
- [string_slicing.md](string_slicing.md) - Efficient substring operations
- [string_map.md](string_map.md) - Functional string transformations
- [parse_numbers.md](parse_numbers.md) - Safe string to number conversions
- [cstring_boundaries.md](cstring_boundaries.md) - C strings only at API boundaries

### Data Structures
- [hash_table.md](hash_table.md) - Type-safe hash tables
- [ring_buffer.md](ring_buffer.md) - Circular buffers with iteration

### System Integration
- [file_monitor.md](file_monitor.md) - Cross-platform file watching
- [thread_sync.md](thread_sync.md) - Threading and synchronization
- [toml_parsing.md](toml_parsing.md) - TOML configuration parsing

## Advanced Patterns

### Control Flow
- [carr_for.md](carr_for.md) - C array iteration macro
- [switch_patterns.md](switch_patterns.md) - Switch case best practices
- [switch_over_if.md](switch_over_if.md) - Prefer switch to if/else chains
- [rval_pattern.md](rval_pattern.md) - Compound literal casts
- [qsort_compare.md](qsort_compare.md) - Sort comparison functions

### Terminal Output
- [ansi_colors.md](ansi_colors.md) - Colored terminal output
- [logging.md](logging.md) - Formatted logging output

### Metaprogramming
- [macro_utilities.md](macro_utilities.md) - Macro helper utilities
- [enum_names.md](enum_names.md) - Enum to string conversion

### Platform Integration
- [sdl_integration.md](sdl_integration.md) - SDL3 cross-platform APIs
- [test_macros.md](test_macros.md) - Unit testing helpers

### Low-Level Operations
- [hash_functions.md](hash_functions.md) - Hashing strings and data
- [substr_macros.md](substr_macros.md) - Substring helper macros

## API Catalog
- [api_catalog.md](api_catalog.md) - Complete enumeration of all sp.h APIs with line numbers

## Philosophy

The examples follow sp.h's philosophy:
- **No standard library** - Avoid stdlib except in specific platform code
- **Zero allocation by default** - Use stack and context allocators
- **Type safety through macros** - Leverage C preprocessor for ergonomics
- **Cross-platform by design** - Single API for Windows/Linux/macOS
- **No comments** - Code should be self-documenting

Each example shows:
- **Good** - Idiomatic sp.h usage
- **Bad** - Common stdlib/unsafe patterns to avoid

## Contributing

When adding new examples:
1. Use the existing format (description, Good/Bad code blocks)
2. Reference actual usage from spn.h, space/, and test.c
3. Include line numbers for API references
4. No comments in code blocks
5. Keep examples concise and focused
