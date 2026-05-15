docker run --rm -it --platform linux/arm64 -v $PWD:/sp -w /sp debian:stable-slim /sp/build/aarch64-linux-gnu/test/$1
