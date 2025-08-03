#!/bin/bash

# Simple build script for portable sp test suite
# Usage: ./build.sh

set -e  # Exit on any error

echo "Building portable sp test suite..."

# Compiler settings
CC=clang
CFLAGS="-std=c11 -g -O0 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function"
INCLUDES="-I.."
OUTPUT="test"

# Platform-specific settings
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Building for macOS..."
    LIBS=""
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo "Building for Linux..."
    LIBS="-lpthread -lm"  # Add pthread and math libraries
else
    # Other Unix-like systems
    echo "Building for Unix-like system..."
    LIBS="-lpthread -lm"
fi

# Build command
$CC $CFLAGS $INCLUDES ../test.c $LIBS -o $OUTPUT

echo "Build successful! Run with: ./$OUTPUT"

# Make the script executable
chmod +x build.sh
