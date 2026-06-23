# MiniRedis17 Supported Commands

## Connection Commands

### PING
- **Arity**: -1 (variable)
- **Returns**: PONG, or echoes argument
- **Examples**:
  - `PING` → `+PONG\r\n`
  - `PING hello` → `$5\r\nhello\r\n`

### ECHO message
- **Arity**: 2
- **Returns**: Bulk String (the message)
- **Example**: `ECHO hello` → `$5\r\nhello\r\n`

### QUIT
- **Arity**: 1
- **Returns**: +OK
- **Effect**: Closes connection after sending response

## Generic Key Commands

### DEL key [key ...]
- **Arity**: -2
- **Returns**: Integer (number of keys removed)
- **Note**: Ignores non-existent keys

### EXISTS key [key ...]
- **Arity**: -2
- **Returns**: Integer (number of keys that exist)

### TYPE key
- **Arity**: 2
- **Returns**: Simple String (none/string/list/hash)

## String Commands

### SET key value [EX seconds] [PX milliseconds] [NX|XX]
- **Arity**: -3
- **Returns**: OK, or nil if NX/XX condition not met
- **Options**:
  - `EX seconds` - set expiry in seconds
  - `PX milliseconds` - set expiry in milliseconds
  - `NX` - only set if key does not exist
  - `XX` - only set if key already exists
- **Behavior**: Overwrites existing value. Clears existing TTL unless new TTL specified.
- **Conflict rules**: Last EX/PX wins. NX and XX are mutually exclusive; last one wins.

### GET key
- **Arity**: 2
- **Returns**: Bulk String or nil
- **Error**: WRONGTYPE if key holds non-string type

### MSET key value [key value ...]
- **Arity**: -3 (odd number of args = error)
- **Returns**: +OK
- **Note**: Atomic set of multiple keys

### MGET key [key ...]
- **Arity**: -2
- **Returns**: Array of Bulk Strings (nil for missing keys)

### APPEND key value
- **Arity**: 3
- **Returns**: Integer (length of string after append)
- **Note**: Creates key if not exists

### STRLEN key
- **Arity**: 2
- **Returns**: Integer (length of string), 0 if key not exists
- **Error**: WRONGTYPE if key holds non-string type

### INCR key
- **Arity**: 2
- **Returns**: Integer (value after increment)
- **Note**: If key not exists, starts from 0. Error if value is not integer. Checks int64_t overflow.

### DECR key
- **Arity**: 2
- **Returns**: Integer (value after decrement)
- **Note**: Same rules as INCR

## List Commands

### LPUSH key element [element ...]
- **Arity**: -3
- **Returns**: Integer (list length after push)

### RPUSH key element [element ...]
- **Arity**: -3
- **Returns**: Integer (list length after push)

### LPOP key
- **Arity**: 2
- **Returns**: Bulk String or nil
- **Note**: Removes key if list becomes empty

### RPOP key
- **Arity**: 2
- **Returns**: Bulk String or nil
- **Note**: Removes key if list becomes empty

### LLEN key
- **Arity**: 2
- **Returns**: Integer (list length), 0 if key not exists
- **Error**: WRONGTYPE if key holds non-list type

### LRANGE key start stop
- **Arity**: 4
- **Returns**: Array of Bulk Strings
- **Note**:
  - Supports negative indices (-1 = last element)
  - Out of range indices are clamped
  - Returns empty array if key not exists

## Hash Commands

### HSET key field value [field value ...]
- **Arity**: -4
- **Returns**: Integer (number of new fields added, not counting updated fields)

### HGET key field
- **Arity**: 3
- **Returns**: Bulk String or nil

### HDEL key field [field ...]
- **Arity**: -3
- **Returns**: Integer (number of fields removed)

### HLEN key
- **Arity**: 2
- **Returns**: Integer (number of fields), 0 if key not exists
- **Error**: WRONGTYPE if key holds non-hash type

### HGETALL key
- **Arity**: 2
- **Returns**: Array of Bulk Strings [field1, value1, field2, value2, ...]
- **Note**: Returns fields in sorted order (deterministic for testing)

## TTL Commands

### EXPIRE key seconds
- **Arity**: 3
- **Returns**: 1 if timeout set, 0 if key not exists

### PEXPIRE key milliseconds
- **Arity**: 3
- **Returns**: 1 if timeout set, 0 if key not exists

### TTL key
- **Arity**: 2
- **Returns**: Integer (TTL in seconds), -1 if no expiry, -2 if key not exists
- **Note**: Rounded down (floor)

### PTTL key
- **Arity**: 2
- **Returns**: Integer (TTL in milliseconds), -1 if no expiry, -2 if key not exists

### PERSIST key
- **Arity**: 2
- **Returns**: 1 if expiry removed, 0 if key not exists or has no expiry

## TTL Semantics

- `SET key value` clears existing TTL
- `SET key value EX n` sets new TTL
- `INCR`, `APPEND`, `LPUSH`, etc. preserve existing TTL
- `EXPIRE` on non-existent key returns 0
- `EXPIRE` with <= 0 seconds immediately deletes the key
- TTL is stored in milliseconds internally
- `TTL` returns floor(seconds_remaining)
- Expired keys are removed lazily (on access) and actively (periodic sweep)

## Differences from Redis

- HGETALL returns fields in sorted order (Redis uses unspecified order from dict)
- Protocol errors disconnect the client (Redis tries to continue)
- No MULTI/EXEC, SUBSCRIBE, etc. (see out of scope)
- Configuration syntax is simplified (key=value, not Redis config format)
