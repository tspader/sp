# Makefile for memory leak tracking tests
#
# Usage:
#   make        - Build all tests
#   make clean  - Remove all build artifacts
#   make run    - Build and run all tests
#   make fs     - Build and run filesystem leak test
#   make ps     - Build and run subprocess leak test

CC ?= cc
CFLAGS = -Wall -Wextra -I../.. -Wno-implicit-fallthrough
LDFLAGS = -lm

# Debug build by default
CFLAGS += -O0 -g

BUILD_DIR = build
TESTS = fs_leak ps_leak

.PHONY: all clean run fs ps

all: $(addprefix $(BUILD_DIR)/,$(TESTS))

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/fs_leak: fs_leak.c ../../sp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BUILD_DIR)/ps_leak: ps_leak.c ../../sp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

run: all
	@echo "=== Running Filesystem Leak Test ==="
	@./$(BUILD_DIR)/fs_leak
	@echo ""
	@echo "=== Running Subprocess Leak Test ==="
	@./$(BUILD_DIR)/ps_leak

fs: $(BUILD_DIR)/fs_leak
	@./$(BUILD_DIR)/fs_leak

ps: $(BUILD_DIR)/ps_leak
	@./$(BUILD_DIR)/ps_leak
