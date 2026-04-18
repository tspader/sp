MAKEFLAGS += -j$(shell nproc)

ifdef TRIPLE
CC := zig cc --target=$(TRIPLE)
BUILD_DIR = build/$(TRIPLE)
else
CC := cc
BUILD_DIR = build
endif

# Detect freestanding from triple: *-none or *-none-*
FREESTANDING := $(if $(findstring -none,$(TRIPLE)),1,)

# Append .exe for Windows targets so the PE binaries run on Windows.
EXE := $(if $(findstring windows,$(TRIPLE)),.exe,)

ifdef FREESTANDING
CFLAGS_PLATFORM = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DSP_FREESTANDING -DSP_DEFINE_BUILTINS
LDFLAGS =
else
CFLAGS_PLATFORM =
LDFLAGS = -lm
endif

CFLAGS = -std=c99 -g -Werror=return-type $(CFLAGS_PLATFORM)

TESTS = amalg app asset context core cv format fmon fs glob ht io leak linkage ps rb str thread time mem prompt
EXAMPLES = app array elf format hash_table io ls palette prompt signal wc

TEST_DIR = $(BUILD_DIR)/test
TEST_BINARIES = $(addsuffix $(EXE), $(addprefix $(TEST_DIR)/, $(TESTS)))
TEST_BINS = $(TEST_DIR)/process$(EXE)

EXAMPLE_DIR = $(BUILD_DIR)/example
EXAMPLE_BINARIES = $(addsuffix $(EXE), $(addprefix $(EXAMPLE_DIR)/, $(EXAMPLES)))

.PHONY: all clean tests examples

all: examples tests runner

tests: $(TEST_BINARIES)
examples: $(EXAMPLE_BINARIES)
runner: build/sp

############
# EXAMPLES #
############
$(EXAMPLE_DIR)/prompt$(EXE): example/prompt.c sp.h sp/sp_prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -I. -o $@ $<

$(EXAMPLE_DIR)/%$(EXE): example/%.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -I. -o $@ $<

#########
# TESTS #
#########
CFLAGS_TEST = -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -I. -Itools -Itest/tools -Itest/tools/process

build/sp: tools/sp.c sp.h | build/
	cc -g -I. -o $@ $<

$(TEST_DIR)/%$(EXE): test/%.c sp.h | $(TEST_DIR) $(TEST_DIR)/process$(EXE)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) $(LDFLAGS) -o $@ $<

$(TEST_DIR)/process$(EXE): test/tools/process/process.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) $(LDFLAGS) -o $@ $<

$(TEST_DIR)/fs$(EXE): test/fs.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -Itest/fs $(LDFLAGS) -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

TRIPLES = x86_64-linux-none aarch64-linux-none x86_64-windows-gnu x86_64-linux-musl x86_64-linux-gnu

.PHONY: compile $(TRIPLES)

compile: $(TRIPLES)

$(TRIPLES):
	$(MAKE) TRIPLE=$@ examples tests

clean:
	rm -rf $(BUILD_DIR)
