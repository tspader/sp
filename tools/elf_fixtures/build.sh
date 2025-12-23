#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-./build/debug/store/lib}"

mkdir -p "$OUTPUT_DIR"

for src in "$SCRIPT_DIR"/*.c; do
    name=$(basename "$src" .c)
    cc -c "$src" -o "$OUTPUT_DIR/$name.o" -g
done

echo "Built ELF fixtures to $OUTPUT_DIR"
