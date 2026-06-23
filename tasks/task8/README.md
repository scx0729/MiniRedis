# Task 8: AOF Persistence and Recovery

## Background

Without persistence, all data is lost when the server restarts. An Append-Only File (AOF) logs every write command. On startup, the server replays the AOF to restore state.

## Learning Goals

- Write-Ahead Log concept
- Command logging in RESP format
- Startup replay
- Handling truncated/corrupted tail
- I/O error handling
- Configuration for persistence

## Your Task

### 1. Implement AOF Writer

`src/persistence/aof.cpp`:
- Open file in append mode
- Write RESP-encoded commands
- fsync strategy (always or never)
- Handle I/O errors

### 2. Implement AOF Replay

`src/persistence/aof_replay.cpp`:
- Read AOF file on startup
- Parse and execute commands
- Handle truncated tail gracefully (stop, don't crash)
- Don't re-write replayed commands to AOF

### 3. Integrate with Commands

After a write command succeeds, log it to AOF:
- Only log write commands (SET, DEL, LPUSH, etc.)
- Don't log read commands (GET, LLEN, etc.)
- Log AFTER successful execution

### 4. Add Configuration

- `appendonly yes|no`
- `appendfsync always|no`

## AOF Format

Use RESP format for the AOF file. Each command is a RESP Array:
```
*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
```

## Truncated Tail Handling

If the AOF file ends with an incomplete command (e.g., server crashed mid-write), the replay should:
1. Successfully replay all complete commands
2. Stop at the first parse error
3. Not crash

## Files to Modify

- `src/persistence/aof.cpp` (required)
- `src/persistence/aof_replay.cpp` (required)
- `src/config/config.cpp` (add AOF config parsing)
- `tests/unit/aof_test.cpp` (required)
- `tests/integration/recovery_test.cpp` (required)

## Testing

Test with actual server restart:
```bash
# Start server, write data, kill, restart, verify data
./build-debug/miniredis-server &
redis-cli -p 6380 SET key value
kill $(pgrep miniredis-server)
./build-debug/miniredis-server &
redis-cli -p 6380 GET key  # should return "value"
kill $(pgrep miniredis-server)
```

```bash
cmake --build build-debug --target aof_test recovery_test
./build-debug/aof_test
./build-debug/recovery_test
```

## Grading

- Correctness: 45%
- Truncated tail handling: 20%
- I/O error handling: 15%
- Configuration: 10%
- Code structure: 5%
- Test coverage: 5%
