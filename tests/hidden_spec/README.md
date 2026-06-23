# Hidden Test Specifications

This directory contains specifications for hidden tests that are not shown to students.
Instructors can use these to create additional test cases for grading.

## Task 0: ByteBuffer
- Concurrent append/consume cycles
- Buffer exactly at max size
- Zero-length operations
- Very large data (near max_size)

## Task 1: RESP Encoder
- Bulk String with embedded CRLF
- Integer at int64_t boundaries (INT64_MAX, INT64_MIN)
- Deeply nested arrays (10+ levels)
- Array with mixed types

## Task 2: RESP Parser
- 1000+ commands concatenated
- Bulk string of exactly max_bulk_length
- Array of exactly max_array_length elements
- Protocol error after partial valid data
- Multiple CR without LF
- LF without preceding CR

## Task 3: String Commands
- SET with NX on existing key of different type (should not set)
- INCR on value "9223372036854775807" (INT64_MAX, should overflow)
- DECR on value "-9223372036854775808" (INT64_MIN, should overflow)
- MGET with 1000 keys
- APPEND with binary data containing null bytes

## Task 4: List/Hash Commands
- LPUSH/RPUSH with 1000 elements
- LRANGE on list of 10000 elements
- HGETALL with 1000 fields
- HSET then HDEL all fields (key should be deleted)
- Wrong type: HSET on a List key

## Task 5: Session/Commands
- Pipeline of 100 commands in one TCP segment
- Command split across two TCP reads
- Protocol error followed by valid command on same connection
- QUIT in middle of pipeline

## Task 6: Network
- 100 concurrent clients
- Client sends one byte at a time
- Client sends all data then immediately closes
- Client never reads responses (slow client)
- Server SIGINT while clients connected
- FD reuse after client disconnect

## Task 7: TTL
- EXPIRE with negative seconds (should delete key)
- SET key with PX, then EXPIRE with EX (EX should override)
- Multiple keys expiring at same time
- Active expiration under load

## Task 8: AOF
- AOF file with 1000 commands
- AOF file with binary data in values
- AOF file corrupted in the middle (not just tail)
- AOF disabled (appendonly no)
- Recovery with expired keys in AOF
