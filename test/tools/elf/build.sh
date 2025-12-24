#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-./build/debug/store/lib}"

if [ ! -d "$OUTPUT_DIR" ]; then
    echo "Error: output directory does not exist: $OUTPUT_DIR" >&2
    exit 1
fi

for src in "$SCRIPT_DIR"/*.c; do
    name=$(basename "$src" .c)
    cc -c "$src" -o "$OUTPUT_DIR/$name.o" -g
done

echo "Built ELF objects in $OUTPUT_DIR"
