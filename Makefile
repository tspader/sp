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
SP_MAKEFILE := Makefile
SP_COMPILE_DB := compile_commands.json
SP_SP_H := sp.h


##################
# COMPILER FLAGS #
##################
SP_FLAG_INCLUDES := $(shell spn print --compiler gcc)
SP_FLAG_WARNINGS := -Wall -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-type-limits -Wno-missing-braces
SP_FLAG_LINKER := -lpthread -lm -Lbuild/bin
SP_FLAG_OPTIMIZATION := -g
ifeq ($(OS),Windows_NT)
  CC := gcc
  MAKE := make
  CMAKE := cmake
  SP_FLAG_RPATH :=
else
  CC := bear --append -- gcc
  MAKE := bear --append -- make
  CMAKE := bear --append -- cmake

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

# Miscellaneous flags
SP_FLAGS_RUN := --enable-mixed-units --random-order

SP_SOURCE_FILES := test.c

###########
# TARGETS #
###########
all: clangd build

.PHONY: c cpp stress build test debug clean all

$(SP_DIR_BUILD_OUTPUT):
	@mkdir -p $(SP_DIR_BUILD_OUTPUT)
	spn build
	spn copy $(SP_DIR_BUILD_OUTPUT)

$(SP_OUTPUT_CPP): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CPP) $(SP_FLAGS_CPP) -x c++ test.c -o $(SP_OUTPUT_CPP)

$(SP_OUTPUT_C): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_C)

$(SP_OUTPUT_STRESS): $(SP_SOURCE_FILES) $(SP_SP_H) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_STRESS) $(SP_SOURCE_FILES) -o $(SP_OUTPUT_STRESS)

$(SP_COMPILE_DB): $(SP_MAKEFILE)
	@make clean
	@bear -- make build

build: $(SP_OUTPUT_C)
test: c cpp stress

c: $(SP_OUTPUT_C)
	./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)

cpp: $(SP_OUTPUT_CPP)
	./$(SP_OUTPUT_CPP) $(SP_FLAGS_RUN)

stress: $(SP_OUTPUT_STRESS)
	./$(SP_OUTPUT_STRESS) $(SP_FLAGS_RUN)

clangd: $(SP_COMPILE_DB)

debug: c
	gdb --args ./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)

clean:
	@rm -rf $(SP_DIR_BUILD)
	@rm -f $(SP_COMPILE_DB)

