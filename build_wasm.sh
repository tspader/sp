#!/bin/bash
set -e
ZIG=/tmp/zig-linux-x86_64-0.11.0/zig
WASM3=/home/user/wasm3/build/wasm3

echo "=== Building hello.wasm ==="
$ZIG cc -target wasm32-wasi -o hello.wasm hello.c 2>&1
echo "=== Build successful! ==="

echo "=== Running with wasm3 ==="
$WASM3 hello.wasm
