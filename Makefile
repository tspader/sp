CC := clang

CFLAGS_FREESTANDING = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined
CFLAGS_DEBUG = -O0 -g
CFLAGS_RELEASE = -O2
CFLAGS_SMALL = -Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS_SMALL = -Wl,--gc-sections

BUILD_DIR = build
EXAMPLE_DIR = build/example

TARGETS =                   \
	$(BUILD_DIR)/freestanding \
	$(EXAMPLE_DIR)/jit        \
	$(EXAMPLE_DIR)/prompt

.PHONY: all clean

all: $(TARGETS)

$(BUILD_DIR)/freestanding: test/freestanding.c sp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/jit: example/freestanding/jit/jit.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/prompt: example/cli/prompt.c sp.h sp/prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_RELEASE) -I. -Wall -o $@ $< -lm

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

clean:
	rm -rf $(BUILD_DIR)
