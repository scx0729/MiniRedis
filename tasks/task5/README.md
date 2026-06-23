# Task 5: Session and Command Pipeline

## Background

A Redis client can send multiple commands without waiting for responses (pipelining). The Session ties together the parser, command executor, and connection buffers. It reads from the input buffer, parses commands, executes them, and writes responses to the output buffer.

## Learning Goals

- How protocol parsing connects to command execution
- Pipeline support (multiple commands per read)
- Session state management
- Error recovery (protocol errors close the connection)
- QUIT command behavior

## Your Task

Implement `Session::ProcessInput()` in `src/server/session.cpp`:

1. Loop: try to parse a command from the input buffer
2. If parser needs more data, break
3. If protocol error, send error response and return false
4. Extract command args from the parsed RESP Array
5. Execute the command
6. Encode response and append to output buffer
7. Consume parsed bytes from input buffer
8. Handle QUIT command (mark connection for closing)

## Key Design

The Session should be testable without real sockets. It operates on:
- Input ByteBuffer (filled by Connection::Read)
- Output ByteBuffer (drained by Connection::Write)

## Files to Modify

- `src/server/session.cpp` (required)
- `tests/integration/session_test.cpp` (required)

## Common Mistakes

- Not consuming parsed bytes (parser re-parses the same data)
- Not handling protocol errors (infinite loop on bad input)
- Coupling Session directly to socket I/O
- Not handling QUIT specially

## Testing

```bash
cmake --build build-debug --target session_test
./build-debug/session_test
```

## Grading

- Correctness: 50%
- Pipeline support: 20%
- Error handling: 15%
- Code structure: 10%
- Test coverage: 5%
