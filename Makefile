CC := clang

CFLAGS_FREESTANDING = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DBUILD_FREESTANDING_EXAMPLE
CFLAGS_DEBUG = -O0 -g

BUILD_DIR = build
EXAMPLE_DIR = build/example

TARGETS =                   \
	$(BUILD_DIR)/freestanding \
	$(EXAMPLE_DIR)/jit        \
	$(EXAMPLE_DIR)/signal     \
	$(EXAMPLE_DIR)/prompt

.PHONY: all clean

all: $(TARGETS)

$(BUILD_DIR)/freestanding: test/freestanding.c sp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/jit: example/freestanding/jit.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/prompt: example/cli/prompt.c sp.h sp/prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/signal: example/freestanding/signal.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

clean:
	rm -rf $(BUILD_DIR)
