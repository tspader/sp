BUILD_TYPE := debug
CMAKE_TYPE := Debug
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
  DLL_EXTENSION := dylib
else ifeq ($(UNAME), Linux)
	DLL_EXTENSION := so
endif


#########
# PATHS #
#########
SP_DIR_BUILD:= build
  SP_DIR_BUILD_OUTPUT := $(SP_DIR_BUILD)/bin
    SP_OUTPUT_C := $(SP_DIR_BUILD_OUTPUT)/sp-c
    SP_OUTPUT_CPP := $(SP_DIR_BUILD_OUTPUT)/sp-cpp
    SP_OUTPUT_STRESS := $(SP_DIR_BUILD_OUTPUT)/sp-stress
    SP_OUTPUT_SDL := $(SP_DIR_BUILD_OUTPUT)/sp-sdl
SP_MAKEFILE := Makefile
SP_COMPILE_DB := compile_commands.json
SP_SP_H := sp.h

SPN := $(shell command -v spn)
BEAR := $(shell command -v bear)


##################
# COMPILER FLAGS #
##################
ifeq ($(SPN),)
SP_FLAG_INCLUDES :=
else
SP_FLAG_INCLUDES := $(shell $(SPN) print --compiler gcc)
endif
SP_FLAG_WARNINGS := -Wall -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-type-limits -Wno-missing-braces
SP_FLAG_LINKER := -lpthread -lm -Lbuild/bin
SP_FLAG_OPTIMIZATION := -g
ifeq ($(OS),Windows_NT)
  CC := gcc
  MAKE := make
  CMAKE := cmake
  SP_FLAG_RPATH :=
else
  ifeq ($(BEAR),)
    CC := gcc
    MAKE := make
    CMAKE := cmake
  else
    CC := bear --append -- gcc
    MAKE := bear --append -- make
    CMAKE := bear --append -- cmake
  endif

  ifeq ($(shell uname),Darwin)
    SP_FLAG_RPATH := -Wl,-rpath,@loader_path
  endif

  ifeq ($(shell uname),Linux)
    SP_FLAG_RPATH := -Wl,-rpath,\$$ORIGIN
  endif
endif

SP_FLAGS_COMMON := $(SP_FLAG_INCLUDES) $(SP_FLAG_WARNINGS) $(SP_FLAG_LINKER) $(SP_FLAG_RPATH) $(SP_FLAG_OPTIMIZATION)


# C
SP_FLAG_CC_LANGUAGE := -std=c11
SP_FLAGS_CC := $(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE)

# C++
CPP := g++
SP_FLAG_CPP_LANGUAGE := -std=c++20
SP_FLAGS_CPP := $(SP_FLAGS_COMMON) $(SP_FLAG_CPP_LANGUAGE)

# C + Stress
SP_FLAGS_STRESS = $(SP_FLAGS_CC) -DSP_TEST_ENABLE_STRESS_TESTS

# C + SDL
SP_FLAGS_SDL := $(SP_FLAGS_CC) -DSP_OS_BACKEND_SDL

# Miscellaneous flags
SP_FLAGS_RUN := --enable-mixed-units --random-order

SP_SOURCE_FILES := test.c

EXAMPLES := ansi_colors \
            bump_allocator \
            c_strings_only_at_api_boundaries \
            carr_for \
            compound_literals \
            context_allocator \
            dynamic_array \
            enum_names \
            error_avoid_nesting \
            file_monitor \
            format_string \
            hash_functions \
            logging \
            macro_utilities \
            parse_numbers \
            path_operations \
            qsort_compare \
            ring_buffer \
            sdl_integration \
            string_literal \
            string_map \
            string_padding \
            string_prefix_checking \
            string_slicing \
            switch_over_if \
            switch_patterns \
            test_macros \
            thread_sync \
            zero_initialize

EXAMPLE_TARGETS := $(addprefix $(SP_DIR_BUILD_OUTPUT)/,$(EXAMPLES))

###########
# TARGETS #
###########
all: clangd build

.PHONY: c cpp stress sdl build test debug clean all examples $(EXAMPLES)

$(SP_DIR_BUILD_OUTPUT):
	@mkdir -p $(SP_DIR_BUILD_OUTPUT)
ifneq ($(SPN),)
	$(SPN) build
	$(SPN) copy $(SP_DIR_BUILD_OUTPUT)
endif

$(SP_DIR_BUILD_SDL):
	@mkdir -p $(SP_DIR_BUILD_SDL)

$(SP_OUTPUT_CPP): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CPP) $(SP_FLAGS_CPP) -x c++ test.c -o $(SP_OUTPUT_CPP)

$(SP_OUTPUT_C): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_C)

$(SP_OUTPUT_SDL): $(SDL_OUTPUT) $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_SDL) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_SDL)

$(SP_OUTPUT_STRESS): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_STRESS) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_STRESS)

$(SP_DIR_BUILD_OUTPUT)/%: examples/%.c | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) $< -o $@

examples: $(EXAMPLE_TARGETS)

$(EXAMPLES): %: $(SP_DIR_BUILD_OUTPUT)/%

$(SP_COMPILE_DB): $(SP_MAKEFILE)
	@make clean
	@bear -- make build

build: $(SP_OUTPUT_C) $(SP_OUTPUT_CPP) $(SP_OUTPUT_STRESS) $(SP_OUTPUT_SDL)
test: c cpp stress sdl

c: $(SP_OUTPUT_C)
	./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)

cpp: $(SP_OUTPUT_CPP)
	./$(SP_OUTPUT_CPP) $(SP_FLAGS_RUN)

stress: $(SP_OUTPUT_STRESS)
	./$(SP_OUTPUT_STRESS) $(SP_FLAGS_RUN)

sdl: $(SP_OUTPUT_SDL)
	./$(SP_OUTPUT_SDL) $(SP_FLAGS_RUN)

clangd: $(SP_COMPILE_DB)

debug: c
	gdb --args ./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)

clean:
	@rm -rf $(SP_DIR_BUILD)
	@rm -f $(SP_COMPILE_DB)

