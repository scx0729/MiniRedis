# MiniRedis17

A teaching-oriented Redis-style in-memory database server written in C++17, designed as a course project similar to MIT/CMU systems courses.

## Learning Objectives

By completing this project, students will understand:

- C++17 project structure and CMake build systems
- RAII and resource ownership patterns
- Network protocol design and incremental parsing
- Non-blocking I/O and epoll event loops
- Single-threaded server architecture
- In-memory database design with type systems
- Key expiration mechanisms (lazy + active)
- Append-Only File (AOF) persistence
- Testing strategies (unit, integration, black-box)

## Architecture Overview

```
Client TCP → Input Buffer → RESP Parser → Command Registry → Executor → Database
                                                                      ↓
Client TCP ← Output Buffer ← RESP Encoder ← Response ← ← ← ← ← ← ←┘
```

Modules:
- **RESP**: Protocol value types, encoder, incremental parser (state machine)
- **Storage**: Database with String/List/Hash types, Expiration with min-heap
- **Command**: Registry (case-insensitive lookup), Executor (arity validation)
- **Server**: ByteBuffer, Connection (non-blocking), Session (pipeline), EventLoop (epoll)
- **Persistence**: AOF writer (RESP format), AOF replay on startup
- **Config**: Key=value configuration file parser

## Supported Commands

| Category | Commands |
|----------|----------|
| Connection | PING, ECHO, QUIT |
| Generic | DEL, EXISTS, TYPE |
| String | SET, GET, MSET, MGET, APPEND, STRLEN, INCR, DECR |
| List | LPUSH, RPUSH, LPOP, RPOP, LLEN, LRANGE |
| Hash | HSET, HGET, HDEL, HLEN, HGETALL |
| TTL | EXPIRE, PEXPIRE, TTL, PTTL, PERSIST |

## Not Supported

Redis Cluster, replication, Sentinel, Lua, ACL, TLS, RDB, Streams, Sorted Sets, Bitmap, HyperLogLog, GEO, modules, multi-threaded execution, distributed consensus.

## Requirements

- Linux (epoll required)
- CMake 3.16+
- C++17 compiler (GCC 9+ or Clang 10+)
- No external dependencies

## Build

```bash
# Debug build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Release build
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j

# With sanitizers
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON \
  -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j
```

## Run Tests

```bash
ctest --test-dir build --output-on-failure
```

## Start Server

```bash
./build/miniredis-server --config config/miniredis.conf
```

## Test with redis-cli

```bash
redis-cli -p 6380 PING
redis-cli -p 6380 SET key value
redis-cli -p 6380 GET key
redis-cli -p 6380 LPUSH list a b c
redis-cli -p 6380 LRANGE list 0 -1
redis-cli -p 6380 HSET user name alice age 20
redis-cli -p 6380 HGETALL user
redis-cli -p 6380 EXPIRE key 10
redis-cli -p 6380 TTL key
```

## Task Order

Complete tasks in order: Task 0 → Task 1 → ... → Task 8

| Task | Topic |
|------|-------|
| 0 | Error model, ByteBuffer |
| 1 | RESP Value, Encoder |
| 2 | Incremental RESP Parser |
| 3 | Database, String commands |
| 4 | List, Hash commands |
| 5 | Session, Pipeline |
| 6 | TCP, epoll EventLoop |
| 7 | TTL, Expiration |
| 8 | AOF Persistence |

## Directory Structure

```
miniredis17/
├── CMakeLists.txt
├── config/miniredis.conf
├── docs/                    # Architecture, protocol, command docs
├── include/miniredis/       # Header files
├── src/                     # Implementation
├── tests/
│   ├── unit/                # Unit tests
│   ├── integration/         # Integration tests
│   ├── blackbox/            # Black-box network tests
│   └── hidden_spec/         # Hidden test specs (for instructors)
├── tasks/task0..task8/      # Task descriptions for students
├── tools/                   # Utility scripts
└── scripts/                 # Build/test scripts
```

## Academic Integrity

This project is for learning. Students should:
- Implement code themselves based on task descriptions
- Not copy from other students
- Not look at the reference implementation before completing their own
- Cite any external resources used

## License

Educational use only.
