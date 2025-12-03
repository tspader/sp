INCLUDE := -I. -Itest -Itest/tools -Iexternal/utest.h -Iexternal/argparse
CC := tcc

.PHONY: all ps core fs

all: core

amalg:
	$(CC) $(INCLUDE) -Wall -Werror -lpthread -lm ./test/amalg.c -std=c99 -o build/debug/amalg -DSP_IMPLEMENTATION

ps:
	$(CC) $(INCLUDE) -Wall -Werror -lpthread -lm ./test/tools/process.c -std=c99 -o build/debug/process -DSP_IMPLEMENTATION
	$(CC) $(INCLUDE) -Wall -Werror -lpthread -lm ./test/ps.c -std=c99 -o build/debug/ps -DSP_IMPLEMENTATION

fs:
	$(CC) $(INCLUDE) -Wall -Werror -lpthread -lm ./test/fs.c -std=c99 -o build/debug/fs -DSP_IMPLEMENTATION

core:
	$(CC) $(INCLUDE) -Wall -Werror -lpthread -lm ./test/core.c -std=c99 -o build/debug/core -DSP_IMPLEMENTATION -lm
