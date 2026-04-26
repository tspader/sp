ROOT     := ../..
BUILDDIR := build

W64_GCC   := $(ROOT)/tools/w64devkit/bin/gcc.exe
MSYS2_DIR := $(ROOT)/tools/msys64/ucrt64/bin
MSYS2_GCC := PATH="$(MSYS2_DIR):$$PATH" $(MSYS2_DIR)/gcc.exe

CFLAGS := -std=c17 -Wall
INCLUDES := -I$(ROOT) -I$(ROOT)/include -I$(ROOT)/tools -I$(ROOT)/test/tools -iquote $(ROOT)/test/tools/process -I$(ROOT)/sp
DEFINES := -DSP_IMPLEMENTATION -DSP_TEST_IMPLEMENTATION

TESTS := app asset context cv elf fmon fs glob ht io leak linkage mem ps rb str time
ALL_TARGETS := smoke $(TESTS) amalg

.PHONY: all w64 msys2 clean

all: w64 msys2

w64: $(addprefix $(BUILDDIR)/w64/, $(ALL_TARGETS:=.exe)) $(BUILDDIR)/w64/process.exe
msys2: $(addprefix $(BUILDDIR)/msys2/, $(ALL_TARGETS:=.exe)) $(BUILDDIR)/msys2/process.exe

$(BUILDDIR)/w64 $(BUILDDIR)/msys2:
	mkdir -p $@

$(BUILDDIR)/w64/smoke.exe: $(ROOT)/test/smoke.c $(ROOT)/sp.h | $(BUILDDIR)/w64
	$(W64_GCC) $(CFLAGS) $(INCLUDES) -DSP_IMPLEMENTATION -o $@ $<

$(BUILDDIR)/msys2/smoke.exe: $(ROOT)/test/smoke.c $(ROOT)/sp.h | $(BUILDDIR)/msys2
	$(MSYS2_GCC) $(CFLAGS) $(INCLUDES) -DSP_IMPLEMENTATION -o $@ $<

$(BUILDDIR)/w64/amalg.exe: $(ROOT)/test/amalg.c $(ROOT)/sp.h | $(BUILDDIR)/w64
	$(W64_GCC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILDDIR)/msys2/amalg.exe: $(ROOT)/test/amalg.c $(ROOT)/sp.h | $(BUILDDIR)/msys2
	$(MSYS2_GCC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILDDIR)/w64/glob.exe: $(ROOT)/test/glob.c $(ROOT)/sp.h | $(BUILDDIR)/w64
	$(W64_GCC) $(CFLAGS) $(INCLUDES) $(DEFINES) -DSP_GLOB_IMPLEMENTATION -o $@ $<

$(BUILDDIR)/msys2/glob.exe: $(ROOT)/test/glob.c $(ROOT)/sp.h | $(BUILDDIR)/msys2
	$(MSYS2_GCC) $(CFLAGS) $(INCLUDES) $(DEFINES) -DSP_GLOB_IMPLEMENTATION -o $@ $<

$(BUILDDIR)/w64/process.exe: $(ROOT)/test/tools/process/process.c $(ROOT)/sp.h | $(BUILDDIR)/w64
	$(W64_GCC) $(CFLAGS) $(INCLUDES) -DSP_IMPLEMENTATION -o $@ $<

$(BUILDDIR)/msys2/process.exe: $(ROOT)/test/tools/process/process.c $(ROOT)/sp.h | $(BUILDDIR)/msys2
	$(MSYS2_GCC) $(CFLAGS) $(INCLUDES) -DSP_IMPLEMENTATION -o $@ $<

$(BUILDDIR)/w64/%.exe: $(ROOT)/test/%.c $(ROOT)/sp.h | $(BUILDDIR)/w64
	$(W64_GCC) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

$(BUILDDIR)/msys2/%.exe: $(ROOT)/test/%.c $(ROOT)/sp.h | $(BUILDDIR)/msys2
	$(MSYS2_GCC) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $<

clean:
	rm -rf $(BUILDDIR)/w64 $(BUILDDIR)/msys2
