# Task 7: TTL and Expiration System

## Background

Redis keys can have a Time-To-Live (TTL). Expired keys should be invisible to reads and eventually cleaned up. This requires a clock abstraction for testability, lazy expiration (on access), and active expiration (periodic sweep).

## Learning Goals

- Time semantics in databases
- Lazy vs active expiration strategies
- Injectable clock for deterministic testing
- Expiration index design (min-heap)
- Handling stale expiration entries

## Your Task

### 1. Implement Clock Abstraction

`include/miniredis/base/clock.h` should have `FakeClock` for testing.

### 2. Implement Expiration Manager

`src/storage/expiration.cpp`:
- Min-heap of (expire_at_ms, key) pairs
- Add/Remove key from tracking
- ActiveExpire: sweep expired keys from heap

### 3. Implement TTL Commands

`src/command/expire_commands.cpp`:
- `EXPIRE key seconds` / `PEXPIRE key milliseconds`
- `TTL key` / `PTTL key` (return remaining time)
- `PERSIST key` (remove expiry)

### 4. Integrate with Database

- Lazy expiration: check on access, delete if expired
- SET with EX/PX option
- SET clears existing TTL
- INCR/APPEND/etc preserve existing TTL

## TTL Rules

- `TTL` returns floor(seconds_remaining)
- `PTTL` returns milliseconds
- Return -1 if key exists but has no expiry
- Return -2 if key doesn't exist
- EXPIRE with <= 0 seconds immediately deletes the key

## Stale Heap Entries

When a key's TTL is updated, the old entry remains in the heap. The active expiration must check if the key still has the same expiry time before deleting.

## Files to Modify

- `src/storage/expiration.cpp` (required)
- `src/storage/database.cpp` (add TTL integration)
- `src/command/expire_commands.cpp` (required)
- `tests/unit/expiration_test.cpp` (required)
- `tests/integration/ttl_command_test.cpp` (required)

## Testing

Do NOT use `sleep()` in tests. Use `FakeClock` to advance time deterministically.

```bash
cmake --build build-debug --target expiration_test ttl_command_test
./build-debug/expiration_test
./build-debug/ttl_command_test
```

## Grading

- Correctness: 50%
- Lazy + active expiration: 20%
- Deterministic testing (no sleep): 15%
- Stale entry handling: 10%
- Code structure: 5%
