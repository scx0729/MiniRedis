# MiniRedis17 Implementation Plan

## Project Overview

MiniRedis17 is a teaching-oriented Redis-style in-memory database server written in C++17. It is designed as a course project similar to MIT/CMU systems courses, where students implement components incrementally.

## Scope

### In Scope
- RESP2 protocol (encode and parse)
- Binary-safe keys and values
- TCP long connections with non-blocking sockets
- epoll event loop (single-threaded)
- Incremental protocol parsing
- Pipeline support (multiple commands per connection)
- Partial read/write handling
- String, List, and Hash types
- Key expiration (lazy + active)
- AOF persistence and recovery
- Configuration loading
- Logging, error handling, graceful shutdown
- Unit, integration, and black-box tests
- Compatibility with `redis-cli` for supported commands

### Out of Scope
- Redis Cluster, replication, Sentinel
- Lua scripting, ACL, TLS
- RDB, Streams, Sorted Sets, Bitmap, HyperLogLog, GEO
- Module system
- Full Redis configuration syntax
- Multi-threaded command execution
- Distributed consensus
- Production-grade security

## Architecture Layers

```
┌─────────────────────────────────────────┐
│              main.cpp                    │
├─────────────────────────────────────────┤
│  Config  │  Server  │  EventLoop        │
├─────────────────────────────────────────┤
│  Connection  │  Session                 │
├─────────────────────────────────────────┤
│  RESP Parser  │  RESP Encoder           │
├─────────────────────────────────────────┤
│  Command Registry  │  Command Executor   │
├─────────────────────────────────────────┤
│  Database  │  Expiration                │
├─────────────────────────────────────────┤
│  AOF Writer  │  AOF Replay              │
├─────────────────────────────────────────┤
│  ByteBuffer  │  Result<T>  │  Clock     │
└─────────────────────────────────────────┘
```

## Implementation Phases

### Phase 1: Foundation (Task 0)
- Error model (Result<T>)
- ByteBuffer
- CMake build system
- Test framework integration

### Phase 2: RESP Protocol (Tasks 1-2)
- RESP Value types
- RESP Encoder
- Incremental RESP Parser (state machine)

### Phase 3: Storage & Commands (Tasks 3-4)
- Database with type system
- String commands (SET, GET, MSET, MGET, APPEND, STRLEN, INCR, DECR)
- List commands (LPUSH, RPUSH, LPOP, RPOP, LLEN, LRANGE)
- Hash commands (HSET, HGET, HDEL, HLEN, HGETALL)
- Command registry and executor

### Phase 4: Network Layer (Tasks 5-6)
- Session (command pipeline)
- Connection management
- epoll event loop
- TCP server with graceful shutdown

### Phase 5: Expiration (Task 7)
- Clock abstraction with FakeClock
- Lazy expiration
- Active expiration (periodic sweep)
- EXPIRE, PEXPIRE, TTL, PTTL, PERSIST commands

### Phase 6: Persistence (Task 8)
- AOF writer (RESP format)
- AOF replay on startup
- Truncated tail handling
- Configuration (appendonly, appendfsync)

## Data Flow

```
Client TCP bytes
  → Input Buffer (ByteBuffer)
  → RESP Parser (incremental state machine)
  → Request = Array<Optional<BulkString>>
  → Command Registry lookup (case-insensitive)
  → Command Executor validates args
  → Database read/write
  → RESP Response encoding
  → Output Buffer (ByteBuffer)
  → TCP send (partial write aware)
```

## Thread Model

Single-threaded event loop. All command execution, parsing, and I/O happen on one thread. No mutexes needed for database access.

## Error Handling Strategy

- `Result<T>` type for recoverable errors (parsing, command execution)
- System call return values always checked
- Fatal errors log and exit cleanly
- Protocol errors disconnect the client
- Command errors return RESP Error to client

## Testing Strategy

1. **Unit tests**: ByteBuffer, RESP Encoder, RESP Parser, Database, Expiration, AOF, Config
2. **Integration tests**: Parser+Session, Session+Command, Command+Database, TTL, AOF Replay
3. **Black-box tests**: Real server via TCP (single command, pipeline, partial writes, multi-client, restart, AOF, graceful shutdown)
4. **Hidden tests**: Edge cases documented in `tests/hidden_spec/` for instructors

## Build System

CMake 3.16+, no internet dependencies. Supports:
- Debug / Release builds
- ASAN / UBSAN
- CTest integration
- Single-command build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Student Version Generation

Using code region markers in the reference implementation:
```cpp
// BEGIN STUDENT TODO(task=N)
// ... reference implementation ...
// END STUDENT TODO(task=N)
```

A Python tool (`tools/generate_student_repo.py`) strips future task implementations and replaces with compilable stubs.
