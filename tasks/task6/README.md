# Task 6: Non-blocking TCP and epoll Event Loop

## Background

This is where everything comes together: real network I/O. You'll implement a TCP server using non-blocking sockets and epoll, handling multiple clients, partial reads/writes, and graceful shutdown.

## Learning Goals

- Socket lifecycle (socket, bind, listen, accept, close)
- Non-blocking I/O and why it matters
- epoll for event-driven I/O
- Partial reads and writes
- Connection cleanup and fd management
- Graceful shutdown with signals

## Your Task

### 1. Implement Connection

`src/server/connection.cpp`:
- Non-blocking read from socket to input buffer
- Non-blocking write from output buffer to socket
- Handle EAGAIN, EINTR, connection close

### 2. Implement EventLoop

`src/server/event_loop.cpp`:
- epoll_create, epoll_ctl, epoll_wait
- Handle EPOLLIN, EPOLLOUT, EPOLLERR, EPOLLHUP
- Dispatch events to callbacks

### 3. Implement Server

`src/server/server.cpp`:
- Create listen socket (non-blocking, SO_REUSEADDR)
- Accept connections
- Wire up EventLoop with Session
- Handle SIGINT/SIGTERM for graceful shutdown

### 4. Implement main.cpp

Wire everything together with config loading.

## epoll Notes

- Use Level Trigger mode (simpler, correct)
- Always drain input buffer on EPOLLIN (read until EAGAIN)
- Register EPOLLOUT only when output buffer has data
- Unregister EPOLLOUT when output buffer is empty

## Files to Modify

- `src/server/connection.cpp` (required)
- `src/server/event_loop.cpp` (required)
- `src/server/server.cpp` (required)
- `src/main.cpp` (required)

## Common Mistakes

- Using blocking I/O (defeats the purpose of epoll)
- Not handling partial write (data loss)
- Not closing connections on error
- Use-after-free when connection is destroyed while epoll still references it
- Not ignoring SIGPIPE

## Testing

After implementation, test with:
```bash
./build-debug/miniredis-server --config config/miniredis.conf &
redis-cli -p 6380 PING
redis-cli -p 6380 SET key value
redis-cli -p 6380 GET key
kill %1
```

## Grading

- Correctness: 45%
- Non-blocking I/O: 20%
- Connection cleanup: 15%
- Graceful shutdown: 10%
- Code structure: 10%
