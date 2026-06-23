# Task 3: Database and String Commands

## Background

Now that we can parse RESP and encode responses, we need a storage engine and commands. The database maps keys (binary-safe strings) to typed values. This task introduces the type system and String commands.

## Learning Goals

- Separation between protocol layer and storage layer
- Type-safe value storage using std::variant
- Command argument validation
- Integer overflow detection
- The difference between "key not found" and "wrong type"

## Your Task

### 1. Implement Database

Implement the `Database` class in `src/storage/database.cpp`:
- String value storage
- Get/Set/Del/Exists/Type operations
- Type checking (WrongType error when using wrong command on wrong type)

### 2. Implement String Commands

Implement commands in `src/command/string_commands.cpp`:
- `SET key value [EX seconds] [PX milliseconds] [NX|XX]`
- `GET key`
- `MSET key value [key value ...]`
- `MGET key [key ...]`
- `APPEND key value`
- `STRLEN key`
- `INCR key` / `DECR key`

### 3. Implement Generic Commands

- `DEL key [key ...]`
- `EXISTS key [key ...]`
- `TYPE key`

### 4. Write Tests

## SET Options

- `EX seconds` - set expiry (covered in Task 7, but parsing should work now)
- `PX milliseconds` - set expiry in ms
- `NX` - only set if key doesn't exist
- `XX` - only set if key exists
- Last NX/XX wins if both specified

## INCR/DECR Rules

- If key doesn't exist, treat as 0 before incrementing
- If value is not a valid integer, return error
- Check for int64_t overflow

## Files to Modify

- `src/storage/database.cpp` (required)
- `src/command/string_commands.cpp` (required)
- `tests/unit/database_test.cpp` (required)
- `tests/integration/string_command_test.cpp` (required)

## Common Mistakes

- Not checking type before operations (LLEN on a String key should error)
- Using `atoi` for integer parsing (doesn't detect errors)
- Not handling int64_t overflow in INCR/DECR
- SET NX returning OK even when key already exists

## Testing

```bash
cmake --build build-debug --target database_test string_command_test
./build-debug/database_test
./build-debug/string_command_test
```

## Grading

- Correctness: 55%
- Type safety (WrongType errors): 15%
- Edge cases (empty value, binary data, overflow): 15%
- Code structure: 10%
- Test coverage: 5%
