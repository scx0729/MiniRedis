#!/bin/bash
set -e

BUILD_DIR="${1:-build-debug}"

echo "=== Running tests ($BUILD_DIR) ==="
ctest --test-dir "$BUILD_DIR" --output-on-failure
echo "=== Tests complete ==="
