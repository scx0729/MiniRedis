#!/bin/bash
set -e

echo "=== Building with AddressSanitizer + UBSan ==="
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON \
  -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j"$(nproc)"

echo "=== Running tests with sanitizers ==="
ctest --test-dir build-asan --output-on-failure
echo "=== Sanitizer tests complete ==="
