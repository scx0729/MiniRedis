# Task 2: Incremental RESP Parser

## Background

Network data arrives in chunks. A single `recv()` might give you half a command, or three commands concatenated. An incremental parser processes what it has and remembers its state for the next chunk. This is the most technically challenging part of the protocol layer.

## Learning Goals

- State machine design for incremental parsing
- Handling half-packets (need more data) vs full packets
- Handling multiple commands in one buffer
- Parser safety limits (max depth, max length)
- The difference between "need more data" and "protocol error"

## Your Task

Implement the `RespParser` class in `src/resp/parser.cpp`. The parser must:

1. Accept partial input and return `NeedMoreData` when a complete message isn't available
2. Parse one complete message and return it with the number of bytes consumed
3. Preserve state between calls for incremental parsing
4. Handle all RESP2 types (Simple String, Error, Integer, Bulk String, Array)
5. Enforce safety limits (max bulk length, max array elements, max depth)
6. Return `ProtocolError` for malformed input

### Critical Test Cases

Your parser MUST handle:
- Each byte fed one at a time
- Random chunk sizes
- Header and body separated
- CR and LF separated across chunks
- Multiple complete commands in one input
- Complete command followed by half command
- Binary data with null bytes

## Files to Modify

- `src/resp/parser.cpp` (required)
- `tests/unit/resp_parser_test.cpp` (required)

## Key Interfaces

```cpp
class RespParser {
public:
    ParseResult Parse(std::string_view input);
    void Reset();
};

struct ParseResult {
    ParseStatus status;  // Complete, NeedMoreData, ProtocolError
    size_t consumed;     // bytes consumed (valid if Complete)
    std::optional<RespValue> value;  // parsed value (valid if Complete)
    Error error;         // error info (valid if ProtocolError)
};
```

## Implementation Hints

- Consider a state machine approach: track what you're expecting next (type prefix, CRLF, data, etc.)
- For Bulk String: parse the length first, then accumulate data bytes
- For Array: parse elements recursively (but watch depth limits)
- Keep a buffer for incomplete data across calls

## Common Mistakes

- Re-parsing the entire buffer from scratch on each call (wasteful, hides bugs)
- Not tracking how many bytes were consumed (leading to re-processing)
- Confusing `$-1\r\n` (null) with `$0\r\n\r\n` (empty)
- Not limiting recursion depth for nested arrays
- Returning NeedMoreData when there's actually a protocol error

## Testing

```bash
cmake --build build-debug --target resp_parser_test
./build-debug/resp_parser_test
```

## Grading

- Correctness: 50%
- Incremental parsing (byte-by-byte, random chunks): 20%
- Error handling: 15%
- Safety limits: 10%
- Code structure: 5%
