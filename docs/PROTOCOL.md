# MiniRedis17 RESP2 Protocol

## Overview

MiniRedis17 uses RESP2 (Redis Serialization Protocol) for client-server communication. All data is binary-safe.

## RESP2 Data Types

### Simple String
```
+OK\r\n
```
Prefix: `+`, terminated by CRLF. Cannot contain CR or LF inside the value.

### Error
```
-ERR unknown command\r\n
```
Prefix: `-`, terminated by CRLF. Format: `-ERRCODE message\r\n`

### Integer
```
:1000\r\n
```
Prefix: `:`, signed 64-bit integer in decimal, terminated by CRLF.

### Bulk String
```
$6\r\nhello\r\n
```
Prefix: `$`, followed by byte count in decimal, CRLF, then the data bytes, then CRLF.
- Binary safe
- Can contain \0 bytes
- Length includes all bytes up to the specified count

### Null Bulk String
```
$-1\r\n
```
Represents a non-existent string (different from empty string `$0\r\n\r\n`).

### Array
```
*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
```
Prefix: `*`, followed by element count, CRLF, then each element encoded in RESP format.
- Elements can be of any type
- Arrays can be nested

### Null Array
```
*-1\r\n
```
Represents a non-existent array.

## Commands

Commands are sent as RESP Arrays of Bulk Strings:
```
*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n
```

The first element is the command name (case-insensitive). Remaining elements are arguments.

## Pipeline

Multiple commands can be sent on the same connection without waiting for responses. The server processes them in order and sends all responses.

## Binary Safety

Both keys and values are binary-safe. The length prefix in Bulk String ensures correct handling of \0 bytes and arbitrary binary data.

## Parser Requirements

The parser must be incremental:
- Accept partial input and return NeedMoreData
- Track state across multiple Parse() calls
- Never assume one recv() contains a complete message
- Handle multiple complete commands in one input chunk
- Handle one command split across multiple input chunks

## Error Handling

Protocol errors (malformed input) cause the server to:
1. Send a RESP Error response
2. Close the connection

This is stricter than Redis (which tries to continue) but simpler for a teaching project.
