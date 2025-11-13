.PHONY: all core

all: core

core:
	clang -I. -Iexternal/utest.h -Itest -Itest/tools -Wall -Werror -lpthread -lm ./test/main.c -std=c99 -o build/debug/core -DSP_IMPLEMENTATION
