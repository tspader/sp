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

ifneq (,$(findstring wasm32,$(TRIPLE)))
  RUNNER = wasmtime run
endif

CFLAGS = $(CFLAGS_LANG) -g -Werror=return-type -fsanitize=undefined,alignment -fno-sanitize-recover=all $(CFLAGS_PLATFORM)
CFLAGS_TEST = -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION -DSP_CLI_TEST_DIR='"$(CURDIR)/test/cli"' -DSP_GDB_TOOLS_DIR='"$(CURDIR)/tools/gdb"' -I. -Itest/tools -Itest
CFLAGS_BENCH = $(CFLAGS_LANG) -g -Werror=return-type -O2 -DSP_IMPLEMENTATION -DUBENCH_ENABLE_PERF_COUNTERS -I. -Itest/bench -Itest/tools

MBEDTLS_DIR = tools/vendor/mbedtls
MBEDTLS_INC = -I$(MBEDTLS_DIR)/include
MBEDTLS_SRCS = $(wildcard $(MBEDTLS_DIR)/library/*.c)
MBEDTLS_OBJS = $(patsubst $(MBEDTLS_DIR)/library/%.c,$(BUILD_DIR)/mbedtls/%.o,$(MBEDTLS_SRCS))
MBEDTLS_LIB = $(BUILD_DIR)/mbedtls/libmbedtls.a
CFLAGS_MBEDTLS = -x c -std=c99 -O2 $(MBEDTLS_INC) -I$(MBEDTLS_DIR)/library

ifdef TRIPLE
  MBEDTLS_AR = zig ar
else
  MBEDTLS_AR = ar
endif

MBEDTLS_OK = 1
ifneq (,$(findstring linux-none,$(TRIPLE)))
  MBEDTLS_OK =
endif
ifneq (,$(findstring wasm,$(TRIPLE)))
  MBEDTLS_OK =
endif
ifneq (,$(findstring freestanding,$(TRIPLE)))
  MBEDTLS_OK =
endif

TLS_LDLIBS =
TLS_DEFINES =
ifneq (,$(findstring windows,$(TRIPLE)))
  TLS_LDLIBS = -lws2_32 -lcrypt32 -ladvapi32 -lbcrypt
endif
ifeq ($(TRIPLE),)
  ifeq ($(shell uname -s),Darwin)
    TLS_LDLIBS = -framework Security -framework CoreFoundation
    TLS_DEFINES = -DSP_TLS_MACOS_SECTRUST
  endif
endif

TESTS = amalg app array asset cli etc cv env format fmon fs glob ht io math process ps rb str thread time mem prompt leak tls
BENCHES = glob heap
EXAMPLES = app array cli format hash_table io zero_copy ls palette prompt prompt_fancy signal tls wc
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

.PHONY: all clean tests examples bench smoke big c cpp gcc tcc check ci $(TRIPLES)
all: examples tests
tests: $(TEST_BINARIES)
examples: $(EXAMPLE_BINARIES)
bench: $(BENCH_BINARIES)

$(BUILD_DIR)/mbedtls/%.o: $(MBEDTLS_DIR)/library/%.c | $(BUILD_DIR)/mbedtls
	$(CC) $(CFLAGS_MBEDTLS) -c -o $@ $<

$(MBEDTLS_LIB): $(MBEDTLS_OBJS)
	$(MBEDTLS_AR) rcs $@ $(MBEDTLS_OBJS)

ifdef MBEDTLS_OK
$(EXAMPLE_DIR)/tls$(EXE): example/tls.c $(SP_HEADERS) $(MBEDTLS_LIB) | $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) -I. -DSP_TLS_WITH_MBEDTLS $(TLS_DEFINES) $(MBEDTLS_INC) -o $@ $< -x none $(MBEDTLS_LIB) $(TLS_LDLIBS)

$(TEST_DIR)/tls$(EXE): test/tls.c $(SP_HEADERS) $(TEST_SOURCES) $(MBEDTLS_LIB) | $(TEST_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_TEST) -DSP_TLS_WITH_MBEDTLS $(TLS_DEFINES) $(MBEDTLS_INC) -o $@ $< -x none $(MBEDTLS_LIB) $(TLS_LDLIBS)
endif

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
ci:
	+$(MAKE) check
	+$(MAKE) MODE=cpp check
check: all
	@for t in $(TEST_BINARIES); do \
		echo "> $$t"; \
		$(RUNNER) $$t || exit 1; \
		echo ""; \
	done
wasm:
	+$(MAKE) wasm32-wasi wasm32-freestanding
	+$(MAKE) MODE=cpp wasm32-wasi wasm32-freestanding

$(BUILD_DIR) $(EXAMPLE_DIR) $(TEST_DIR) $(BENCH_DIR) $(BUILD_DIR)/mbedtls:
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
