# MiniRedis17 Instructor Guide

## Overview

This guide helps instructors set up, deploy, and grade the MiniRedis17 course project.

## Generating Student Versions

Use the provided tool to generate student repositories:

```bash
python3 tools/generate_student_repo.py --task 3 --output student-task3/
```

This creates a version where:
- Tasks 0-2: code is kept (student has completed these)
- Task 3: code is replaced with stubs + TODO comments
- Tasks 4-8: code is removed entirely

## Restoring Full Solution

The reference implementation is in the main repository. To restore:
```bash
git checkout master
```

## Grading

Each task has a rubric in `tasks/taskN/RUBRIC.md`. General weights:
- Correctness: 50-60%
- Edge cases: 15-20%
- Resource safety: 10%
- Code structure: 10%
- Test coverage: 5%

## Hidden Tests

See `tests/hidden_spec/README.md` for hidden test specifications. These test edge cases not covered by the public tests.

## Running All Checks

```bash
# Debug build + tests
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j
ctest --test-dir build-debug --output-on-failure

# Release build
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j

# Sanitizer build
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

## Common Student Errors

1. **Parser**: Not truly incremental (re-parsing from scratch each time)
2. **ByteBuffer**: Returning consumed data from Peek()
3. **INCR**: Not checking int64_t overflow
4. **HGETALL**: Using unordered_map (non-deterministic order)
5. **TTL**: Using real time in tests (non-deterministic)
6. **epoll**: Using blocking I/O
7. **AOF**: Not handling truncated tail

## Task Dependencies

Task N depends on all tasks 0..N-1 being complete. Each task's start version should compile and pass all tests for previous tasks.

## Configuration

Default config: `config/miniredis.conf`. Students can modify for testing. Key settings:
- `port 6380` (different from default Redis 6379)
- `appendonly yes|no`
- `appendfsync always|no`
