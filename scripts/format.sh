#!/bin/bash
# Format source files using clang-format
# Requires clang-format to be installed

if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found. Install it first."
    exit 1
fi

find include/ src/ tests/ -name "*.h" -o -name "*.cpp" | xargs clang-format -i
echo "Formatted all source files."
