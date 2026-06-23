# Task 1: RESP Value Types and Encoder

## Background

Redis uses RESP (Redis Serialization Protocol) for client-server communication. Before we can parse incoming data, we need to represent RESP values in memory and encode them to wire format.

## Learning Goals

- RESP2 data model (Simple String, Error, Integer, Bulk String, Array)
- Binary safety and why length-prefixed formats are needed
- The difference between Null and Empty in protocol design
- Recursive data structures (arrays can contain arrays)

## Your Task

### 1. Implement RESP Value types

The `RespValue` class in `include/miniredis/resp/value.h` should be complete. Verify it handles all RESP2 types.

### 2. Implement the Encoder

Implement `RespEncoder::Encode` in `src/resp/encoder.cpp`:

- `+OK\r\n` for Simple String
- `-ERR message\r\n` for Error
- `:42\r\n` for Integer
- `$5\r\nhello\r\n` for Bulk String (length-prefixed)
- `$-1\r\n` for Null Bulk String
- `*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n` for Array
- `*-1\r\n` for Null Array

### 3. Write tests

Test cases in `tests/unit/resp_encoder_test.cpp`:
- Empty string, string with CRLF, string with null bytes
- Positive, negative, zero integers
- Null vs Empty Bulk String
- Nested arrays
- Array with null elements

## Files to Modify

- `src/resp/encoder.cpp` (required)
- `tests/unit/resp_encoder_test.cpp` (required)

## Key Interfaces

```cpp
class RespEncoder {
public:
    static std::string Encode(const RespValue& value);
    static std::string Encode(const std::optional<RespValue>& value);
};
```

## Invariants

- Null Bulk String (`$-1\r\n`) is different from Empty Bulk String (`$0\r\n\r\n`)
- Bulk String length counts bytes, not characters
- Array length counts elements, not bytes
- All lines end with `\r\n`

## Common Mistakes

- Confusing Null Bulk String with Empty Bulk String
- Not binary-safe (using C string functions that stop at `\0`)
- Wrong length calculation for Bulk String

## Testing

```bash
cmake --build build-debug --target resp_encoder_test
./build-debug/resp_encoder_test
```

## Grading

- Correctness: 60%
- Binary safety (null bytes, CRLF in data): 15%
- Null vs Empty distinction: 10%
- Code structure: 10%
- Test coverage: 5%
