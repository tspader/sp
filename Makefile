CC := gcc

CFLAGS_FREESTANDING = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined
CFLAGS_DEBUG = -O0 -g

BUILD_DIR = build
EXAMPLE_DIR = build/example

TARGETS =                   \
	$(BUILD_DIR)/freestanding \
	$(EXAMPLE_DIR)/jit \
	$(BUILD_DIR)/minimal

.PHONY: all clean

all: $(TARGETS)

$(BUILD_DIR)/freestanding: test/freestanding.c sp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/jit: example/freestanding/jit/jit.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(BUILD_DIR)/minimal: test/amalg.c sp.h | $(BUILD_DIR)
	$(CC) -I. -Iinclude -Itest/tools -Itest/tools/process -Isp -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -std=c99 -g test/amalg.c -Werror=return-type -o build/debug/store/bin/minimal -lm

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

clean:
	rm -rf $(BUILD_DIR)
