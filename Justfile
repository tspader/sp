#!/usr/bin/env just --justfile

default:
    @just --list

build_type := "debug"

flags := if build_type == "release" {
    "-O2 -DNDEBUG"
} else if build_type == "asan" {
    "-g -O1 -fsanitize=address -fno-omit-frame-pointer"
} else {
    "-g -O0"
}

exe_name := if build_type == "release" {
    "test-release"
} else if build_type == "asan" {
    "test-asan"
} else {
    "test"
}

build:
    @echo "Building sp test suite ({{build_type}})..."
    @mkdir -p build
    gcc -std=c11 {{flags}} \
        -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function \
        -I. \
        test.c \
        -lpthread -lm \
        -o build/{{exe_name}}
    @echo "Build successful! Executable: build/{{exe_name}}"

run: build
    @echo "Running sp tests ({{build_type}})..."
    @./build/{{exe_name}}

rerun:
    @echo "Running sp tests ({{build_type}})..."
    @./build/{{exe_name}}

clean:
    @echo "Cleaning build directory..."
    @rm -rf build

build-all:
    @just build_type=debug build
    @just build_type=release build
    @just build_type=asan build

test-all:
    @echo "=== Running Debug Build ==="
    @just build_type=debug run
    @echo ""
    @echo "=== Running Release Build ==="
    @just build_type=release run
    @echo ""
    @echo "=== Running ASAN Build ==="
    @just build_type=asan run