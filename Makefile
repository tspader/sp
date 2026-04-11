ifdef TRIPLE
CC := zig cc --target=$(TRIPLE)
BUILD_DIR = build/$(TRIPLE)
else
CC := cc
BUILD_DIR = build
endif

# Detect freestanding from triple: *-none or *-none-*
FREESTANDING := $(if $(findstring -none,$(TRIPLE)),1,)

ifdef FREESTANDING
CFLAGS_PLATFORM = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DSP_FREESTANDING -DSP_DEFINE_BUILTINS
LDFLAGS_PLATFORM =
else
CFLAGS_PLATFORM =
LDFLAGS_PLATFORM = -lm
endif

CFLAGS_DEBUG = -O0 -g

EXAMPLE_DIR = $(BUILD_DIR)/example
TEST_DIR = $(BUILD_DIR)/test

CFLAGS_TEST = -std=c99 -g -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -Werror=return-type $(CFLAGS_PLATFORM)
INCLUDES_TEST = -I. -Itools -Itest/tools -Itest/tools/process

TEST_NAMES = amalg app asset context core cv elf fmon fs glob ht io leak linkage ps rb str time mem prompt
TESTS = $(addprefix $(TEST_DIR)/, $(TEST_NAMES))
TEST_BINS = $(TEST_DIR)/process

EXAMPLE_NAMES = array elf hash_table ls palette prompt signal wc
EXAMPLES = $(addprefix $(EXAMPLE_DIR)/, $(EXAMPLE_NAMES))
CFLAGS_EXAMPLE = -std=c99 -g -Werror=return-type $(CFLAGS_PLATFORM)

.PHONY: all clean tests examples

all: examples

tests: $(TESTS)
examples: $(EXAMPLES)

$(EXAMPLE_DIR)/prompt: example/prompt.c sp.h sp/sp_prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_EXAMPLE) $(LDFLAGS_PLATFORM) -I. -o $@ $<

$(EXAMPLE_DIR)/%: example/%.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS_EXAMPLE) $(LDFLAGS_PLATFORM) -I. -o $@ $<

#########
# TESTS #
#########
$(TEST_DIR)/%: test/%.c sp.h | $(TEST_DIR) $(TEST_DIR)/process
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) $(LDFLAGS_PLATFORM) -o $@ $<

$(TEST_DIR)/process: test/tools/process/process.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) $(LDFLAGS_PLATFORM) -o $@ $<

$(TEST_DIR)/fs: test/fs.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS_TEST) $(INCLUDES_TEST) -Itest/fs $(LDFLAGS_PLATFORM) -o $@ $<

# $(TEST_DIR)/ps: test/ps.c sp.h | $(TEST_DIR) $(TEST_DIR)/process

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR)
