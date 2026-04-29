# Default to parallel when invoked directly. Skipped under a parent jobserver
# (e.g. the `big` target's sub-makes) to avoid oversubscription.
ifeq (,$(findstring jobserver,$(MAKEFLAGS)))
MAKEFLAGS += -j$(shell nproc)
endif

ifdef TRIPLE
CC := zcc --target=$(TRIPLE)
BUILD_DIR = build/$(TRIPLE)
else ifeq ($(origin CC),command line)
BUILD_DIR = build/$(CC)
else
CC := cc
BUILD_DIR = build
endif

WASM := $(if $(findstring wasm,$(TRIPLE)),1,)
WASM_FREESTANDING := $(if $(WASM),$(if $(findstring freestanding,$(TRIPLE)),1,),)
LINUX_FREESTANDING := $(if $(WASM),,$(if $(findstring linux-none,$(TRIPLE)),1,))

ifdef LINUX_FREESTANDING
CFLAGS_PLATFORM = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DSP_FREESTANDING
else ifdef WASM_FREESTANDING
CFLAGS_PLATFORM = -nostdlib
else ifdef WASM
CFLAGS_PLATFORM =
else
CFLAGS_PLATFORM =
endif

CFLAGS = -std=c99 -g -Werror=return-type -fsanitize=undefined,alignment -fno-sanitize-recover=all $(CFLAGS_PLATFORM)

EXE := $(if $(findstring windows,$(TRIPLE)),.exe,)
EXE := $(if $(WASM),.wasm,$(EXE))

TESTS = app
EXAMPLES = app format hash_table
TRIPLES = \
  x86_64-linux-none x86_64-linux-gnu x86_64-linux-musl \
  aarch64-linux-none aarch64-linux-gnu aarch64-linux-musl \
  aarch64-macos \
  x86_64-windows-gnu \
  wasm32-freestanding wasm32-wasi

.PHONY: all clean tests examples $(TRIPLES) big gcc tcc
all: examples tests tools


TEST_DIR = $(BUILD_DIR)/test
TEST_BINARIES = $(addsuffix $(EXE), $(addprefix $(TEST_DIR)/, $(TESTS)))
TEST_BINS = $(TEST_DIR)/process$(EXE)

EXAMPLE_DIR = $(BUILD_DIR)/example
EXAMPLE_BINARIES = $(addsuffix $(EXE), $(addprefix $(EXAMPLE_DIR)/, $(EXAMPLES)))

tests: $(TEST_BINARIES)
examples: $(EXAMPLE_BINARIES)
tools: build/sp

############
# EXAMPLES #
############
$(EXAMPLE_DIR)/prompt$(EXE): example/prompt.c sp.h sp/sp_prompt.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) -I. -o $@ $<

$(EXAMPLE_DIR)/%$(EXE): example/%.c sp.h | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) -I. -o $@ $<

#########
# TESTS #
#########
CFLAGS_TEST = -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -I. -Itools -Itest/tools -Itest/tools/process

build/sp: tools/sp.c sp.h | build/
	cc -g -I. -o $@ $<

$(TEST_DIR)/%$(EXE): test/%.c sp.h | $(TEST_DIR) $(TEST_DIR)/process$(EXE)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -o $@ $<

$(TEST_DIR)/process$(EXE): test/tools/process/process.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -o $@ $<

$(TEST_DIR)/fs$(EXE): test/fs.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -Itest/fs -o $@ $<

$(TEST_DIR)/mem$(EXE): test/mem.c sp.h | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -Itest/mem -o $@ $<

#########
# CROSS #
#########
$(TRIPLES):
	+$(MAKE) TRIPLE=$@ examples tests

big: $(TRIPLES) gcc tcc examples tests

gcc:
	+$(MAKE) CC=gcc examples tests

tcc:
	+$(MAKE) CC=tcc examples tests

clean:
	rm -rf $(BUILD_DIR)

#########
# UTILS #
#########
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXAMPLE_DIR):
	mkdir -p $(EXAMPLE_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

