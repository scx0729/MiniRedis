# MiniRedis17 Architecture

## Module Overview

### 1. Base Layer (`base/`)

**Result<T>**: A discriminated union for error handling. Either holds a value of type T or an Error. Used throughout the codebase instead of exceptions for expected failures.

**Error**: Error code enum + optional message string. Categories: Protocol, Command, Type, IO, Config.

**Clock**: Abstract clock interface. Production code uses system clock; tests use FakeClock for deterministic TTL testing.

### 2. RESP Layer (`resp/`)

**Value**: Represents a RESP2 value. Variant of:
- SimpleString (string)
- Error (string)
- Integer (int64_t)
- BulkString (string, binary-safe, may be null)
- Array (vector of optional<Value>, may be null)

**Encoder**: Converts Value to RESP wire format bytes.

**Parser**: Incremental state machine that consumes bytes and produces Values. Tracks state across calls (handles partial input). Returns ParseResult with status (Complete/NeedMoreData/ProtocolError), consumed byte count, and parsed value.

### 3. Storage Layer (`storage/`)

**Value types**: StringValue (string), ListValue (deque<string>), HashValue (ordered map<string,string>).

**Database**: Key-value store using unordered_map<string, Entry>. Each Entry holds a variant value + optional expiration time. Provides typed accessors. Checks type before operations (returns WrongType error).

**Expiration**: Manages key expiration using a min-heap of (expiry_time, key). Supports:
- Lazy check: on key access, check if expired
- Active sweep: periodic scan of heap, delete expired keys

### 4. Command Layer (`command/`)

**Command**: Interface defining command name, arity, flags (write/readonly), and execute function.

**Registry**: Maps command names (case-insensitive) to Command objects. Static registration.

**Executor**: Validates argument count, dispatches to command handler, returns RESP Value response.

### 5. Server Layer (`server/`)

**ByteBuffer**: Growable byte buffer with append/peek/consume operations. Tracks consumed vs unconsumed regions. Has configurable maximum size.

**Connection**: Owns a socket fd. Has input ByteBuffer and output ByteBuffer. Handles non-blocking read/write with partial write tracking.

**Session**: Owns a Connection + Parser + reference to Database/Registry. Loop: parse commands from input buffer → execute → encode responses to output buffer. Handles pipeline (multiple commands per read).

**EventLoop**: epoll wrapper. Manages listen socket and connection sockets. Dispatches read/write/error/close events.

**Server**: Top-level. Binds listen socket, creates EventLoop, accepts connections, handles signals for graceful shutdown.

### 6. Persistence Layer (`persistence/`)

**AofWriter**: Appends RESP-encoded write commands to AOF file after successful execution. Configurable fsync (always/never).

**AofReplay**: Reads AOF file on startup, parses RESP commands, re-executes them against Database. Handles truncated tail gracefully.

### 7. Config Layer (`config/`)

**Config**: Parses configuration file (key=value format). Provides typed accessors with defaults. Validated at startup.

## Data Ownership

```
Server
  ├── Config
  ├── EventLoop
  │     └── epoll_fd
  ├── Database (unique_ptr)
  ├── Expiration (unique_ptr)
  ├── CommandRegistry (unique_ptr)
  ├── AofWriter (unique_ptr, optional)
  └── Connections (map<int, unique_ptr<Connection>>)
        └── Session (per connection)
              ├── Parser
              ├── Input ByteBuffer
              └── Output ByteBuffer
```

All ownership is via unique_ptr. No raw owning pointers. Session borrows Database and Registry via reference (lifetime guaranteed by Server).

## Request Processing Flow

1. epoll reports EPOLLIN on connection fd
2. Connection::Read() appends bytes to input ByteBuffer
3. Session::ProcessInput() loops:
   a. Parser::Parse() on unconsumed input
   b. If complete: extract command name + args from Array
   c. Registry::Find(command_name)
   d. Executor::Run(command, args, database)
   e. Encoder::Encode(response) → append to output ByteBuffer
   f. Consume parsed bytes from input
   g. Repeat while more complete commands available
4. If output ByteBuffer has data, register EPOLLOUT
5. epoll reports EPOLLOUT → Connection::Write() sends from output ByteBuffer
6. If all output sent, unregister EPOLLOUT

## Graceful Shutdown

1. SIGINT/SIGTERM sets atomic flag
2. EventLoop checks flag, stops accepting new connections
3. Existing connections finish pending I/O
4. AOF flush
5. Close all connections
6. Exit with code 0

## Limit Constants

| Parameter | Default | Config Key |
|-----------|---------|------------|
| Max client input buffer | 64 MB | max-client-input-buffer |
| Max client output buffer | 64 MB | max-client-output-buffer |
| Max bulk string length | 512 MB | max-bulk-length |
| Max array elements | 10000 | (hardcoded) |
| Max nested depth | 100 | (hardcoded) |
| Active expire interval | 100 ms | active-expire-interval-ms |
| Active expire sample count | 20 | (hardcoded) |
