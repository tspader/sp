BUILD_TYPE := debug
CMAKE_TYPE := Debug
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
  DLL_EXTENSION := dylib
else ifeq ($(UNAME), Linux)
	DLL_EXTENSION := so
endif

ANSI_FG_RED := \033[31m
ANSI_FG_GREEN := \033[32m
ANSI_FG_BRIGHT_BLACK := \033[90m
ANSI_FG_BRIGHT_CYAN := \033[96m
ANSI_FG_BRIGHT_YELLOW := \033[93m
ANSI_RESET := \033[0m

define print_color
	@printf "$(1)$(2)$(ANSI_RESET)"
endef

define build_target
	@printf '$(ANSI_FG_BRIGHT_YELLOW)$(1)$(ANSI_RESET) $(ANSI_FG_BRIGHT_BLACK)$(2) $(3) $$(spn print) $(4) -o $(1)$(ANSI_RESET)\n'
	@$(2) $(3) $$(spn print) $(4) -o $(1)
endef

define run_tests
	@printf "$(ANSI_FG_BRIGHT_YELLOW)$(1)$(ANSI_RESET) $(ANSI_FG_BRIGHT_BLACK) ./$(1) --enable-mixed-units --random-order\n";
	@OUTPUT=$$(./$(1) --enable-mixed-units --random-order 2>&1); \
	if echo "$$OUTPUT" | grep -q "PASSED"; then \
		COUNT=$$(echo "$$OUTPUT" | grep -oP '\d+(?= tests?)' | head -1); \
		printf ""; \
		printf "$(ANSI_FG_BRIGHT_YELLOW)$(1) $(ANSI_FG_GREEN)OK$(ANSI_RESET) $$COUNT tests\n"; \
	else \
		printf "$(ANSI_FG_RED)FAIL$(ANSI_RESET) "; \
		echo "$$OUTPUT" | grep -E "test cases ran|PASSED|FAILED"; \
	fi
endef


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
SP_FLAG_INCLUDES := -I. -Itest -Itest/tools -Iexternal
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
  CC := tcc
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

SP_SOURCE_FILES := test/main.c
SP_TEST_SOURCES := sp.h test/test.h

###########
# TARGETS #
###########
all: build

.PHONY: c cpp stress ps build test debug deps clean all

# Ensure dependencies are built before any compilation
deps:
	@spn build --output noninteractive
	@mkdir -p $(SP_DIR_BUILD_OUTPUT)

$(SP_DIR_BUILD_OUTPUT): | deps

build/bin/c: $(SP_TEST_SOURCES) | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/c,$(CC),$(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE),test/main.c)

build/bin/cpp: $(SP_TEST_SOURCES) | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/cpp,$(CPP),$(SP_FLAGS_COMMON) $(SP_FLAG_CPP_LANGUAGE),-x c++ test/main.c)

build/bin/app: $(SP_TEST_SOURCES) test/app.c | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/app,$(CC),$(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE),test/app.c)

build/bin/file_monitor: $(SP_TEST_SOURCES) test/file_monitor.c | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/file_monitor,$(CC),$(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE),test/file_monitor.c)

build/bin/ps: $(SP_TEST_SOURCES) test/ps.c build/bin/process | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/ps,$(CC),$(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE),test/ps.c)

build/bin/process: test/tools/process.* | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/process,$(CC),$(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE),test/tools/process.c)

build/bin/stress: $(SP_TEST_SOURCES) test/stress.c | $(SP_DIR_BUILD_OUTPUT)
	$(call build_target,build/bin/stress,$(CC),$(SP_FLAGS_COMMON) $(SP_FLAG_CC_LANGUAGE),test/stress.c)

build: build/bin/c build/bin/app build/bin/file_monitor build/bin/ps build/bin/process build/bin/stress
test: c app file_monitor cpp ps stress

c: build/bin/c
	$(call run_tests,build/bin/c)

cpp: build/bin/cpp
	$(call run_tests,build/bin/cpp)

app: build/bin/app
	$(call run_tests,build/bin/app)

file_monitor: build/bin/file_monitor
	$(call run_tests,build/bin/file_monitor)

ps: build/bin/ps
	$(call run_tests,build/bin/ps)

stress: build/bin/stress
	$(call run_tests,build/bin/stress)


debug: c
	gdb --args ./$(SP_OUTPUT_C) $(SP_FLAGS_RUN)

clean:
	@rm -rf $(SP_DIR_BUILD)
	@rm -f $(SP_COMPILE_DB)

