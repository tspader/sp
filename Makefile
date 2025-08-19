##############
# BUILD TYPE #
##############
BUILD_TYPE ?= debug
ifeq ($(BUILD_TYPE),release)
  FLAGS := -O2 -DNDEBUG
	EXE_NAME := test-release
else ifeq ($(BUILD_TYPE),asan)
  FLAGS := -g -O1 -fsanitize=address -fno-omit-frame-pointer
  EXE_NAME := test-asan
else
  FLAGS := -g -O0
  EXE_NAME := test
endif


##################
# COMPILER FLAGS #
##################
WARNINGS := -Wall -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-parentheses -Wno-type-limits -Wno-missing-braces
CC := gcc
CFLAGS := -std=c11 $(FLAGS) $(WARNINGS) -I.

CPP := g++
CPPFLAGS := -std=c++20 $(FLAGS) $(WARNINGS) -I.

LOADER_FLAGS := -lpthread -lm

RUN_FLAGS := --enable-mixed-units --random-order

#########
# PATHS #
#########
BUILD_DIR := build
C_TARGET := $(BUILD_DIR)/$(EXE_NAME)-c11
CPP_TARGET := $(BUILD_DIR)/$(EXE_NAME)-cpp20
STRESS_TARGET := $(BUILD_DIR)/$(EXE_NAME)-c11-stress

SOURCES := test.c
HEADERS := sp.h

###########
# TARGETS #
###########
.PHONY: all
all: $(C_TARGET) $(CPP_TARGET) $(STRESS_TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(CPP_TARGET): $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CPP) $(CPPFLAGS) $(SOURCES) $(LOADER_FLAGS) -o $(CPP_TARGET)

$(C_TARGET): $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) $(LOADER_FLAGS) -o $(C_TARGET)

$(STRESS_TARGET): $(SOURCES) $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DSP_TEST_ENABLE_STRESS_TESTS $(SOURCES) $(LOADER_FLAGS) -o $(STRESS_TARGET)

.PHONY: c cpp run debug clean

c: $(C_TARGET)
cpp: $(CPP_TARGET)
stress: $(STRESS_TARGET)

run: c cpp stress
	./$(C_TARGET) $(RUN_FLAGS)
	./$(CPP_TARGET) $(RUN_FLAGS)
	./$(STRESS_TARGET) $(RUN_FLAGS)

debug: c
	gdb ./$(C_TARGET)

clean:
	@rm -rf $(BUILD_DIR)
