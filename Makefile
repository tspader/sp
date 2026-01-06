CC_X64 = zig cc -target x86_64-linux-musl
CC_ARM64 = zig cc -target aarch64-linux-musl
STRIP = strip

CFLAGS_NOSTDLIB = -nostdlib -static -fno-stack-protector -fno-sanitize=undefined
CFLAGS_SP_NOSTDLIB = $(CFLAGS_NOSTDLIB) -DSP_FREESTANDING -DSP_IMPLEMENTATION
CFLAGS_SP_BUILTIN = $(CFLAGS_SP_NOSTDLIB) -DSP_BUILTIN
CFLAGS_SP_ELF = $(CFLAGS_SP_NOSTDLIB) -DSP_ELF_ENABLE
CFLAGS_DEBUG = -O0 -g
CFLAGS_RELEASE = -O2
CFLAGS_SMALL = -Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables
LDFLAGS_SMALL = -Wl,--gc-sections

BUILD_DIR = build

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

HAS_QEMU_ARM := $(shell which qemu-aarch64 2>/dev/null)
HAS_DOCKER := $(shell docker info >/dev/null 2>&1 && echo yes)

ifeq ($(UNAME_S),Darwin)
  DOCKER_X64 = docker run --rm --platform linux/amd64 -v $(CURDIR):/work -w /work alpine:latest
  DOCKER_ARM64 = docker run --rm --platform linux/arm64 -v $(CURDIR):/work -w /work alpine:latest
  RUN_X64 = $(DOCKER_X64)
  RUN_ARM64 = $(DOCKER_ARM64)
else ifeq ($(UNAME_M),aarch64)
  RUN_X64 = qemu-x86_64
  RUN_ARM64 =
else
  RUN_X64 =
  RUN_ARM64 = qemu-aarch64
endif

.PHONY: all clean test test-nostdlib test-nostdlib-x64 test-nostdlib-arm64 test-builtin test-builtin-x64 test-builtin-arm64 test-jit test-jit-x64

all: test

test: test-nostdlib test-builtin test-jit

test-nostdlib: test-nostdlib-x64 test-nostdlib-arm64

test-nostdlib-x64: $(BUILD_DIR)/test_nostdlib_x64
ifeq ($(UNAME_S),Darwin)
ifndef HAS_DOCKER
	$(error docker not running)
endif
endif
	$(RUN_X64) ./$(BUILD_DIR)/test_nostdlib_x64

$(BUILD_DIR)/test_nostdlib_x64: test/nostdlib.c sp.h | $(BUILD_DIR)
	$(CC_X64) $(CFLAGS_SP_NOSTDLIB) $(CFLAGS_DEBUG) -I. -o $@ test/nostdlib.c

test-nostdlib-arm64: $(BUILD_DIR)/test_nostdlib_arm64
ifeq ($(UNAME_S),Darwin)
ifndef HAS_DOCKER
	$(error docker not running)
endif
else ifneq ($(UNAME_M),aarch64)
ifndef HAS_QEMU_ARM
	$(error qemu-aarch64 not found)
endif
endif
	$(RUN_ARM64) ./$(BUILD_DIR)/test_nostdlib_arm64

$(BUILD_DIR)/test_nostdlib_arm64: test/nostdlib.c sp.h | $(BUILD_DIR)
	$(CC_ARM64) $(CFLAGS_SP_NOSTDLIB) $(CFLAGS_DEBUG) -I. -o $@ test/nostdlib.c

test-builtin: test-builtin-x64 test-builtin-arm64

test-builtin-x64: $(BUILD_DIR)/test_builtin_x64
ifeq ($(UNAME_S),Darwin)
ifndef HAS_DOCKER
	$(error docker not running)
endif
endif
	$(RUN_X64) ./$(BUILD_DIR)/test_builtin_x64

$(BUILD_DIR)/test_builtin_x64: test/builtin.c sp.h | $(BUILD_DIR)
	$(CC_X64) $(CFLAGS_SP_BUILTIN) $(CFLAGS_DEBUG) -I. -o $@ test/builtin.c

test-builtin-arm64: $(BUILD_DIR)/test_builtin_arm64
ifeq ($(UNAME_S),Darwin)
ifndef HAS_DOCKER
	$(error docker not running)
endif
else ifneq ($(UNAME_M),aarch64)
ifndef HAS_QEMU_ARM
	$(error qemu-aarch64 not found)
endif
endif
	$(RUN_ARM64) ./$(BUILD_DIR)/test_builtin_arm64

$(BUILD_DIR)/test_builtin_arm64: test/builtin.c sp.h | $(BUILD_DIR)
	$(CC_ARM64) $(CFLAGS_SP_BUILTIN) $(CFLAGS_DEBUG) -I. -o $@ test/builtin.c

test-jit: test-jit-x64

test-jit-x64: $(BUILD_DIR)/test_jit_x64
ifeq ($(UNAME_S),Darwin)
ifndef HAS_DOCKER
	$(error docker not running)
endif
endif
	-$(RUN_X64) ./$(BUILD_DIR)/test_jit_x64

$(BUILD_DIR)/test_jit_x64: example/freestanding/jit/jit.c sp.h | $(BUILD_DIR)
	$(CC_X64) $(CFLAGS_SP_NOSTDLIB) $(CFLAGS_DEBUG) -I. -o $@ example/freestanding/jit/jit.c

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
