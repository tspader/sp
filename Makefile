CC := clang

CFLAGS_FREESTANDING = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DBUILD_FREESTANDING_EXAMPLE
CFLAGS_DEBUG = -O0 -g

BUILD_DIR = build
EXAMPLE_DIR = build/example
TEST_DIR = build/test

CFLAGS_TEST = -std=c99 -g -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -Werror=return-type
INCLUDES_TEST = -I. -Itools -Itest/tools -Itest/tools/process
LDFLAGS_TEST = -lm

TEST_NAMES = app asset context core cv elf fmon fs glob ht io leak linkage ps rb str time mem amalg prompt
TESTS = $(addprefix $(TEST_DIR)/, $(TEST_NAMES))
TEST_BINS = $(TEST_DIR)/process

EXAMPLE_NAMES = array hash_table ls wc
EXAMPLES = $(addprefix $(EXAMPLE_DIR)/, $(EXAMPLE_NAMES))
CFLAGS_EXAMPLE = -std=c99 -g -Werror=return-type

FREESTANDING =              \
	$(BUILD_DIR)/freestanding \
	$(EXAMPLE_DIR)/jit        \
	$(EXAMPLE_DIR)/signal     \
	$(EXAMPLE_DIR)/prompt

.PHONY: all clean tests freestanding

all: freestanding

freestanding: $(FREESTANDING)
tests: $(TEST_BINS) $(TESTS)
examples: $(EXAMPLES)

$(BUILD_DIR)/freestanding: test/freestanding.c sp.h | $(BUILD_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/jit: example/freestanding/jit.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/prompt: example/cli/prompt.c sp.h sp/prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/signal: example/freestanding/signal.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/%: example/%.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_EXAMPLE) $(LDFLAGS_TEST) -I. -o $@ $<

$(TEST_DIR)/process: test/tools/process/process.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) $(LDFLAGS_TEST) -o $@ $<

$(TEST_DIR)/%: test/%.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) $(LDFLAGS_TEST) -o $@ $<

$(TEST_DIR)/fs: test/fs.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) -Itest/fs $(LDFLAGS_TEST) -o $@ $<

$(TEST_DIR)/ps: test/ps.c sp.h | $(TEST_DIR) $(TEST_DIR)/process

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR)
