INCLUDE := -I. -Itest -Itest/tools -Iexternal/utest.h -Iexternal/argparse

.PHONY: all ps

all: ps

ps:
	clang $(INCLUDE) -Wall -Werror -lpthread -lm ./test/tools/process.c -std=c99 -o build/debug/process -DSP_IMPLEMENTATION
	clang $(INCLUDE) -Wall -Werror -lpthread -lm ./test/ps.c -std=c99 -o build/debug/ps -DSP_IMPLEMENTATION

fs:
	clang $(INCLUDE) -Wall -Werror -lpthread -lm ./test/fs.c -std=c99 -o build/debug/fs -DSP_IMPLEMENTATION

core:
	clang $(INCLUDE) -Wall -Werror -lpthread -lm ./test/main.c -std=c99 -o build/debug/core -DSP_IMPLEMENTATION
