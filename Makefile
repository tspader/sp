all: gnu

gnu:
	gcc /home/spader/source/sp/test/main.c -I/home/spader/source/sp/. -I/home/spader/source/sp/test/tools -DSP_IMPLEMENTATION -I/home/spader/.local/share/spn/cache/store/utest/7b80550f/include -I/home/spader/.local/share/spn/cache/store/argparse/15dde6e4/include -std=c99 -g -static -o /home/spader/source/sp/build/debug/store/bin/core-gnu-static -fuse-ld=mold

cosmo:
	cosmocc -g /home/spader/source/sp/test/main.c -I/home/spader/source/sp/. -I/home/spader/source/sp/test/tools -DSP_IMPLEMENTATION -I/home/spader/.local/share/spn/cache/store/utest/7b80550f/include -I/home/spader/.local/share/spn/cache/store/argparse/15dde6e4/include -std=c99 -static -o /home/spader/source/sp/build/debug/store/bin/core-cosmo-static -static

musl:
	musl-gcc /home/spader/source/sp/test/main.c -I/home/spader/source/sp/. -I/home/spader/source/sp/test/tools -DSP_IMPLEMENTATION -I/home/spader/.local/share/spn/cache/store/utest/7b80550f/include -I/home/spader/.local/share/spn/cache/store/argparse/15dde6e4/include -std=c99 -g -static -o /home/spader/source/sp/build/debug/store/bin/core-musl-static -static
