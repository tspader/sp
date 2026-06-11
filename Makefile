ifeq (,$(findstring jobserver,$(MAKEFLAGS)))
  MAKEFLAGS += -j$(shell nproc)
endif

ifeq ($(MODE), cpp)
  CFLAGS_LANG = -std=c++11 -x c++
  BUILD_ROOT = build/cpp
  DEFAULT_CC = c++
  ZIG_CC = zig c++
else
  CFLAGS_LANG = -std=c99
  BUILD_ROOT = build
  DEFAULT_CC = cc
  ZIG_CC = zig cc
endif

ifdef TRIPLE
  CC := $(ZIG_CC) --target=$(TRIPLE)
  BUILD_DIR = $(BUILD_ROOT)/$(TRIPLE)
else ifeq ($(origin CC),command line)
  BUILD_DIR = $(BUILD_ROOT)/$(CC)
else
  CC := $(DEFAULT_CC)
  BUILD_DIR = $(BUILD_ROOT)
endif

CFLAGS_PLATFORM =
ifneq (,$(findstring linux-none,$(TRIPLE)))
  CFLAGS_PLATFORM = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined -DSP_FREESTANDING
endif
ifneq (,$(findstring wasm32-freestanding,$(TRIPLE)))
  CFLAGS_PLATFORM = -nostdlib -fno-sanitize=undefined
endif

ifneq (,$(findstring windows,$(TRIPLE)))
  EXE := .exe
else ifneq (,$(findstring wasm,$(TRIPLE)))
  EXE := .wasm
else
  EXE :=
endif

CFLAGS = $(CFLAGS_LANG) -g -Werror=return-type -fsanitize=undefined,alignment -fno-sanitize-recover=all $(CFLAGS_PLATFORM)
CFLAGS_TEST = -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -I. -Itest/tools -Itest
CFLAGS_BENCH = $(CFLAGS_LANG) -g -Werror=return-type -O2 -DSP_IMPLEMENTATION -DUBENCH_ENABLE_PERF_COUNTERS -I. -Itest/bench -Itest/tools

TESTS = amalg app array asset cli etc cv env format fmon fs glob ht io math process ps rb str thread time mem prompt leak
BENCHES = glob heap
EXAMPLES = app array cli format hash_table io zero_copy ls palette prompt prompt_fancy signal wc
TRIPLES = \
  x86_64-linux-none x86_64-linux-gnu x86_64-linux-musl \
  aarch64-linux-none aarch64-linux-gnu aarch64-linux-musl \
  aarch64-macos \
  x86_64-windows-gnu \
  wasm32-freestanding wasm32-wasi

TEST_DIR = $(BUILD_DIR)/test
EXAMPLE_DIR = $(BUILD_DIR)/example
BENCH_DIR = $(BUILD_DIR)/bench
TEST_BINARIES = $(addsuffix $(EXE),$(addprefix $(TEST_DIR)/,$(TESTS)))
EXAMPLE_BINARIES = $(addsuffix $(EXE),$(addprefix $(EXAMPLE_DIR)/,$(EXAMPLES)))
BENCH_BINARIES = $(addsuffix $(EXE),$(addprefix $(BENCH_DIR)/,$(BENCHES)))

SP_HEADERS = sp.h $(wildcard sp/*.h)
TEST_SOURCES = $(wildcard test/*/*.c) $(wildcard test/*/*.h) $(wildcard test/*/*/*.c) $(wildcard test/*/*/*.h)

.PHONY: all clean tests examples bench smoke big c cpp gcc tcc $(TRIPLES)
all: examples tests
tests: $(TEST_BINARIES)
examples: $(EXAMPLE_BINARIES)
bench: $(BENCH_BINARIES)

$(EXAMPLE_DIR)/%$(EXE): example/%.c $(SP_HEADERS) | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) -I. -o $@ $<

$(TEST_DIR)/%$(EXE): test/%.c $(SP_HEADERS) $(TEST_SOURCES) | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -o $@ $<

$(BENCH_DIR)/%$(EXE): test/bench/%.c $(SP_HEADERS) test/bench/ubench.h test/tools/table.h | $(BENCH_DIR)
	$(CC) $(CFLAGS_BENCH) -o $@ $<

$(TRIPLES):
	+$(MAKE) TRIPLE=$@ examples tests

big: c cpp
c:; +$(MAKE) $(TRIPLES) examples tests gcc
cpp:; +$(MAKE) MODE=cpp $(TRIPLES) examples tests
gcc:; +$(MAKE) CC=gcc examples tests
tcc:; +$(MAKE) CC=tcc examples tests
wasm:
	+$(MAKE) wasm32-wasi wasm32-freestanding
	+$(MAKE) MODE=cpp wasm32-wasi wasm32-freestanding

$(BUILD_DIR) $(EXAMPLE_DIR) $(TEST_DIR) $(BENCH_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
