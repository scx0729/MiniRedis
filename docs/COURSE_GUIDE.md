# MiniRedis17 Course Guide

## Overview

MiniRedis17 is a hands-on project where you build a Redis-style in-memory database server in C++17. The project is divided into 9 tasks (Task 0-8), each building on the previous one.

## Prerequisites

- C++17 (classes, templates, smart pointers, std::variant, std::optional)
- Basic networking (TCP, sockets)
- Basic data structures (hash maps, deques, heaps)
- Linux command line

## Getting Started

1. Clone the repository
2. Build: `cmake -S . -B build && cmake --build build -j`
3. Run tests: `ctest --test-dir build --output-on-failure`
4. Start with Task 0 in `tasks/task0/README.md`

## Task Dependencies

```
Task 0 (ByteBuffer, Result)
  └── Task 1 (RESP Encoder)
        └── Task 2 (RESP Parser)
              └── Task 3 (Database, String commands)
                    └── Task 4 (List, Hash commands)
                          └── Task 5 (Session, Pipeline)
                                └── Task 6 (TCP, epoll)
                                      └── Task 7 (TTL, Expiration)
                                            └── Task 8 (AOF, Recovery)
```

## How to Approach Each Task

1. Read the task README carefully
2. Look at the header files for interfaces
3. Run existing tests to see what fails
4. Implement incrementally
5. Run tests frequently
6. Check edge cases

## Testing Your Code

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
./build/byte_buffer_test

# Run with sanitizers
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

## Testing with redis-cli

Once the server runs (Task 6), you can test with redis-cli:
```bash
./build/miniredis-server --config config/miniredis.conf &
redis-cli -p 6380 PING
redis-cli -p 6380 SET hello world
redis-cli -p 6380 GET hello
```

## Getting Help

- Read the task README
- Check `docs/ARCHITECTURE.md` for system overview
- Check `docs/PROTOCOL.md` for RESP details
- Check `docs/COMMANDS.md` for command specifications
- Look at test files for expected behavior
