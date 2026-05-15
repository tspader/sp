docker run --rm -it --platform linux/arm64 -v $PWD:/sp -w /sp alpine:latest /sp/build/aarch64-linux-musl/test/$1
