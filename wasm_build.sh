#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$DIR/build/wasm"
SRC="$DIR/test/wasm_test.c"

die() { echo "error: $1" >&2; exit 1; }
require() { command -v "$1" &>/dev/null || die "$1 not found"; }

build_freestanding() {
  require zig
  mkdir -p "$BUILD/freestanding"
  zig cc -target wasm32-freestanding \
    -O2 -nostdlib \
    -Wl,--no-entry -Wl,--export=run_tests \
    -DSP_IMPLEMENTATION -DSP_NO_LIBM \
    -I"$DIR" "$SRC" \
    -o "$BUILD/freestanding/sp_test.wasm"
  echo "built: $BUILD/freestanding/sp_test.wasm"
}

build_wasi() {
  require zig
  mkdir -p "$BUILD/wasi"
  zig cc -target wasm32-wasi \
    -O2 \
    -DSP_IMPLEMENTATION \
    -I"$DIR" "$SRC" \
    -o "$BUILD/wasi/sp_test.wasm"
  echo "built: $BUILD/wasi/sp_test.wasm"
}

run_wasmtime() {
  require wasmtime
  wasmtime --dir=/tmp "$BUILD/wasi/sp_test.wasm"
}

run_wasmer() {
  require wasmer
  wasmer --dir=/tmp "$BUILD/wasi/sp_test.wasm"
}

run_wasm3() {
  require wasm3
  wasm3 "$BUILD/wasi/sp_test.wasm"
}

clean() {
  rm -rf "$BUILD"
}

case "${1:-}" in
  freestanding) build_freestanding ;;
  wasi) build_wasi ;;
  wasmtime) run_wasmtime ;;
  wasmer) run_wasmer ;;
  wasm3) run_wasm3 ;;
  clean) clean ;;
  all) build_freestanding; build_wasi ;;
  *) echo "usage: $0 {freestanding|wasi|wasmtime|wasmer|wasm3|clean|all}" ;;
esac
