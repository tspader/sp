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
SP_MAKEFILE := Makefile
SP_SP_H := sp.h


##################
# COMPILER FLAGS #
##################
SP_FLAG_INCLUDES := -I. -Itest -Itest/tools
SP_FLAG_WARNINGS := -Wall -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-type-limits -Wno-missing-braces
SP_FLAG_LINKER := -lpthread -lm -Lbuild/bin
SP_FLAG_OPTIMIZATION := -g
SP_FLAG_DEFINES := -DSP_IMPLEMENTATION
ifeq ($(OS),Windows_NT)
  CC := gcc
  MAKE := make
  CMAKE := cmake
  SP_FLAG_RPATH :=
else
  CC := bear --append -- tcc
  MAKE := bear --append -- make
  CMAKE := bear --append -- cmake

  ifeq ($(shell uname),Darwin)
    SP_FLAG_RPATH := -Wl,-rpath,@loader_path
  endif

  ifeq ($(shell uname),Linux)
    SP_FLAG_RPATH := -Wl,-rpath,\$$ORIGIN
  endif
endif

SP_FLAGS_COMMON := $(SP_FLAG_INCLUDES) $(SP_FLAG_WARNINGS) $(SP_FLAG_LINKER) $(SP_FLAG_RPATH) $(SP_FLAG_OPTIMIZATION) $(SP_FLAG_DEFINES)


# C
SP_FLAG_CC_LANGUAGE := -std=c11
SP_FLAGS_CC := $(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE) $$(spn print)

# C++
CPP := g++
SP_FLAG_CPP_LANGUAGE := -std=c++20
SP_FLAGS_CPP := $(SP_FLAGS_COMMON) $(SP_FLAG_CPP_LANGUAGE) $$(spn print)

# C + Stress
SP_FLAGS_STRESS = $(SP_FLAGS_CC) -DSP_TEST_ENABLE_STRESS_TESTS

# Miscellaneous flags
SP_FLAGS_RUN := --enable-mixed-units --random-order | grep -E "test cases ran|PASSED|FAILED"

SP_SOURCE_FILES := test/main.c
SP_TEST_SOURCES := sp.h test/test.h

###########
# TARGETS #
###########
all: build

.PHONY: c cpp stress ps build test debug deps clean all

# Ensure dependencies are built before any compilation
deps:
	@spn build
	@mkdir -p $(SP_DIR_BUILD_OUTPUT)

$(SP_DIR_BUILD_OUTPUT): | deps

build/bin/c: $(SP_TEST_SOURCES) | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) test/main.c -o build/bin/c

build/bin/cpp: $(SP_TEST_SOURCES) | $(SP_DIR_BUILD_OUTPUT)
	$(CPP) $(SP_FLAGS_CPP) -x c++ test/main.c -o build/bin/cpp

build/bin/app: $(SP_TEST_SOURCES) test/app.c | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) test/app.c -o build/bin/app

build/bin/file_monitor: $(SP_TEST_SOURCES) test/file_monitor.c | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) test/file_monitor.c -o build/bin/file_monitor

build/bin/ps: $(SP_TEST_SOURCES) test/ps.c build/bin/process | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) test/ps.c -o build/bin/ps

build/bin/process: test/tools/process.* | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) test/tools/process.c -o build/bin/process

build/bin/stress: $(SP_TEST_SOURCES) test/stress.c | $(SP_DIR_BUILD_OUTPUT)
	$(CC) $(SP_FLAGS_CC) test/stress.c -o build/bin/stress

build: build/bin/c build/bin/app build/bin/file_monitor build/bin/ps build/bin/process build/bin/stress
test: c cpp app file_monitor ps stress

c: build/bin/c
	./build/bin/c $(SP_FLAGS_RUN)

cpp: build/bin/cpp
	./build/bin/cpp $(SP_FLAGS_RUN)

app: build/bin/app
	./build/bin/app $(SP_FLAGS_RUN)

file_monitor: build/bin/file_monitor
	./build/bin/file_monitor $(SP_FLAGS_RUN)

ps: build/bin/ps
	./build/bin/ps $(SP_FLAGS_RUN)

stress: build/bin/stress
	./build/bin/stress $(SP_FLAGS_RUN)


debug: c
	gdb --args ./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)

clean:
	@rm -rf $(SP_DIR_BUILD)
	@rm -f $(SP_COMPILE_DB)

