# Task 4: List and Hash Commands

## Background

Redis supports multiple data types on the same key space. This task adds List (deque-based) and Hash (ordered map) types. You'll learn about container commands, index handling, and type safety.

## Learning Goals

- Multi-type database design
- Container operations (push, pop, range)
- Negative index handling
- Empty container cleanup (delete key when last element removed)
- Deterministic output ordering

## Your Task

### List Commands

- `LPUSH key element [element ...]` - push to front, return new length
- `RPUSH key element [element ...]` - push to back, return new length
- `LPOP key` - remove and return front element (nil if empty)
- `RPOP key` - remove and return back element (nil if empty)
- `LLEN key` - return length (0 if key doesn't exist)
- `LRANGE key start stop` - return elements in range

### Hash Commands

- `HSET key field value [field value ...]` - set fields, return count of NEW fields
- `HGET key field` - get field value (nil if not exists)
- `HDEL key field [field ...]` - delete fields, return count deleted
- `HLEN key` - return number of fields
- `HGETALL key` - return all field-value pairs in sorted order

## Index Rules (LRANGE)

- Supports negative indices: -1 = last element, -2 = second to last
- Out of range indices are clamped to list boundaries
- Returns empty array if start > stop after clamping
- Returns empty array if key doesn't exist

## HGETALL Order

For test determinism, `HGETALL` must return fields in sorted order. Use `std::map` or sort before output.

## Empty Container Cleanup

When `LPOP`/`RPOP` removes the last element, delete the key.
When `HDEL` removes the last field, delete the key.

## Files to Modify

- `src/command/list_commands.cpp` (required)
- `src/command/hash_commands.cpp` (required)
- `tests/integration/list_command_test.cpp` (required)
- `tests/integration/hash_command_test.cpp` (required)

## Common Mistakes

- Using `std::unordered_map` for Hash (non-deterministic HGETALL)
- Not deleting empty keys
- Wrong HSET return value (should be count of NEW fields, not total)
- LRANGE with negative indices not normalized

## Testing

```bash
cmake --build build-debug --target list_command_test hash_command_test
./build-debug/list_command_test
./build-debug/hash_command_test
```

## Grading

- Correctness: 55%
- Index handling (negative, out of range): 15%
- Type safety: 10%
- Empty cleanup: 10%
- Code structure: 5%
- Test coverage: 5%
