#########
# PATHS #
#########
SP_DIR_BUILD:= build
  SP_DIR_BUILD_OUTPUT := $(SP_DIR_BUILD)/bin
    SP_OUTPUT_C := $(SP_DIR_BUILD_OUTPUT)/sp-c
    SP_OUTPUT_CPP := $(SP_DIR_BUILD_OUTPUT)/sp-cpp
    SP_OUTPUT_STRESS := $(SP_DIR_BUILD_OUTPUT)/sp-stress
    SP_OUTPUT_SDL := $(SP_DIR_BUILD_OUTPUT)/sp-sdl
    SDL_OUTPUT := $(SP_DIR_BUILD_OUTPUT)/libSDL3.so
  SP_DIR_BUILD_SDL := $(SP_DIR_BUILD)/sdl
SP_DIR_EXTERNAL := external
  SP_DIR_SDL := $(SP_DIR_EXTERNAL)/SDL
    SP_DIR_SDL_INCLUDE := $(SP_DIR_SDL)/include
SP_MAKEFILE := Makefile
SP_COMPILE_DB := compile_commands.json
SP_SP_H := sp.h

BUILD_TYPE := debug
CMAKE_TYPE := Debug

##################
# COMPILER FLAGS #
##################
SP_FLAG_INCLUDES := -I.
SP_FLAG_WARNINGS := -Wall -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-type-limits -Wno-missing-braces
SP_FLAG_LINKER := -lpthread -lm
SP_FLAG_OPTIMIZATION := -g
SP_FLAG_DEFINES := -DSP_IMPLEMENTATION # Do it here so clangd picks it up when it intercepts our compile commands
SP_FLAGS_COMMON := $(SP_FLAG_INCLUDES) $(SP_FLAG_WARNINGS) $(SP_FLAG_LINKER) $(SP_FLAG_DEFINES) $(SP_FLAG_OPTIMIZATION)

# C
CC := gcc
SP_FLAG_CC_LANGUAGE := -std=c11
SP_FLAGS_CC := $(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE)

# C++
CPP := g++
SP_FLAG_CPP_LANGUAGE := -std=c++20
SP_FLAGS_CPP := $(SP_FLAGS_COMMON) $(SP_FLAG_CPP_LANGUAGE)

# C + Stress
SP_FLAGS_STRESS = $(SP_FLAGS_CC) -DSP_TEST_ENABLE_STRESS_TESTS

# C + SDL
SDL_FLAG_DEFINES := -DCMAKE_BUILD_TYPE=$(CMAKE_TYPE) -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF -DSDL_EXAMPLES=OFF
SDL_CMAKE_FLAGS := $(SDL_FLAG_DEFINES)
SP_FLAGS_SDL := $(SP_FLAGS_CC) -DSP_OS_BACKEND_SDL -I$(SP_DIR_SDL_INCLUDE) -lSDL3

# Miscellaneous flags
SP_FLAGS_RUN := --enable-mixed-units --random-order

SP_SOURCE_FILES := test.c

###########
# TARGETS #
###########
all: clangd build

$(SP_DIR_BUILD_OUTPUT):
	@mkdir -p $(SP_DIR_BUILD_OUTPUT)

$(SP_DIR_BUILD_SDL):
	@mkdir -p $(SP_DIR_BUILD_SDL)

$(SDL_OUTPUT): $(SP_DIR_BUILD_SDL)
	cmake -S$(SP_DIR_SDL) -B$(SP_DIR_BUILD_SDL) $(SDL_CMAKE_FLAGS)
	cmake --build $(SP_DIR_BUILD_SDL) --parallel
	cp $(SP_DIR_BUILD_SDL)/libSDL3.so $(SP_DIR_BUILD_OUTPUT)

$(SP_OUTPUT_CPP): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CPP) $(SP_FLAGS_CPP) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_CPP)

$(SP_OUTPUT_C): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_C)

$(SP_OUTPUT_SDL): $(SDL_OUTPUT) $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_SDL) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_SDL)

$(SP_OUTPUT_STRESS): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_STRESS) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_STRESS)

$(SP_COMPILE_DB): $(SP_MAKEFILE)
	@make clean
	@bear -- make build

c: $(SP_OUTPUT_C)
cpp: $(SP_OUTPUT_CPP)
stress: $(SP_OUTPUT_STRESS)
sdl: $(SP_OUTPUT_SDL)
build: c cpp stress sdl

clangd: $(SP_COMPILE_DB)

test: build
	./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)
	./$(SP_OUTPUT_CPP) $(SP_FLAGS_RUN)
	./$(SP_OUTPUT_STRESS) $(SP_FLAGS_RUN)
	./$(SP_OUTPUT_SDL) $(SP_FLAGS_RUN)

debug: c
	gdb ./$(SP_OUTPUT_C)

clean:
	@rm -rf $(SP_DIR_BUILD_OUTPUT)
	@rm -f $(SP_COMPILE_DB)

nuke:
	@rm -rf $(SP_DIR_BUILD)

.PHONY: c cpp stress sdl build test debug clean nuke all
