CC := clang

CFLAGS_FREESTANDING = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DSP_FREESTANDING -DSP_DEFINE_BUILTINS
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

EXAMPLE_NAMES = array elf hash_table ls palette prompt signal wc
EXAMPLES = $(addprefix $(EXAMPLE_DIR)/, $(EXAMPLE_NAMES))
CFLAGS_EXAMPLE = -std=c99 -g -Werror=return-type

FREESTANDING_DIR = $(EXAMPLE_DIR)/freestanding
FREESTANDING = $(addprefix $(FREESTANDING_DIR)/, $(EXAMPLE_NAMES))

.PHONY: all clean tests freestanding examples

all: freestanding

freestanding: $(FREESTANDING)
tests: $(TEST_BINS) $(TESTS)
examples: $(EXAMPLES)

$(FREESTANDING_DIR)/prompt: example/prompt.c sp.h sp/sp_prompt.h | $(FREESTANDING_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(FREESTANDING_DIR)/%: example/%.c sp.h | $(FREESTANDING_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(EXAMPLE_DIR)/prompt: example/prompt.c sp.h sp/sp_prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_EXAMPLE) $(LDFLAGS_TEST) -I. -o $@ $<

$(EXAMPLE_DIR)/%: example/%.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_EXAMPLE) $(LDFLAGS_TEST) -I. -o $@ $<

#########
# TESTS #
#########
$(TEST_DIR)/%: test/%.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) $(LDFLAGS_TEST) -o $@ $<

$(TEST_DIR)/freestanding: test/freestanding.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_FREESTANDING) $(CFLAGS_DEBUG) -I. -o $@ $<

$(TEST_DIR)/process: test/tools/process/process.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) $(LDFLAGS_TEST) -o $@ $<

$(TEST_DIR)/fs: test/fs.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) -Itest/fs $(LDFLAGS_TEST) -o $@ $<

$(TEST_DIR)/ps: test/ps.c sp.h | $(TEST_DIR) $(TEST_DIR)/process

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

$(FREESTANDING_DIR):
	mkdir -p $(FREESTANDING_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR)
