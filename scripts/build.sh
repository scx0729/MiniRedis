#!/bin/bash
set -e

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="build-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"

echo "=== Building MiniRedis17 ($BUILD_TYPE) ==="
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" -j"$(nproc)"
echo "=== Build complete: $BUILD_DIR ==="
