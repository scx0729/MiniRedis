# Task 0: Error Model, ByteBuffer, and Project Setup

## Background

Before building a network server, we need foundational building blocks: a way to represent errors without exceptions, and a byte buffer that can grow and be consumed incrementally. These are essential for non-blocking I/O where you never get a complete request in one read.

## Learning Goals

After completing this task, you will understand:
- CMake project structure and how to build/test
- RAII (Resource Acquisition Is Initialization)
- Error handling without exceptions using Result<T>
- Byte buffer design for network programs
- Why you can't assume one recv() gives you a complete request

## Current State

The project compiles and the test framework is set up. The `ByteBuffer` and `Result<T>` types have header files with interfaces defined, but the implementations are stubs.

## Your Task

### 1. Implement `Result<T>` (if not already complete)

The `Result<T>` type should hold either a value of type `T` or an `Error`. It's used throughout the codebase instead of exceptions for expected failures.

### 2. Implement `ByteBuffer`

Implement the `ByteBuffer` class in `src/server/byte_buffer.cpp`:

- `Append(data, len)`: Add data to the buffer. Return error if it would exceed max size.
- `Peek()`: Return a view of unconsumed data without consuming it.
- `Consume(n)`: Mark n bytes as consumed.
- `Available()`: Return number of unconsumed bytes.
- `Compact()`: Move unconsumed data to the front to reclaim space.

### 3. Write tests

Add test cases in `tests/unit/byte_buffer_test.cpp` for:
- Empty buffer
- Multiple appends
- Partial consume
- Full consume
- Max size enforcement
- Data after compact

## Files to Modify

- `src/server/byte_buffer.cpp` (required)
- `tests/unit/byte_buffer_test.cpp` (required)

## Key Interfaces

```cpp
class ByteBuffer {
public:
    explicit ByteBuffer(size_t max_size);
    Result<void> Append(const char* data, size_t len);
    std::string_view Peek() const;
    Result<void> Consume(size_t n);
    size_t Available() const;
    void Compact();
    void Clear();
};
```

## Invariants

- `Available()` always returns the number of bytes from `Peek()` that haven't been consumed
- After `Consume(n)`, `Peek()` starts at byte `n` of the previous view
- `Append` never succeeds if it would exceed `max_size`
- `Compact()` doesn't change `Available()` data, only its position in memory

## Common Mistakes

- Forgetting to update the consumed offset in `Consume`
- Returning a view that includes consumed bytes from `Peek`
- Not checking for overflow in buffer size calculations

## Testing

```bash
cmake --build build-debug --target byte_buffer_test
./build-debug/byte_buffer_test
```

## Grading

- Correctness: 60%
- Edge cases (empty, max size, consume beyond available): 20%
- Resource safety: 10%
- Code structure: 10%
