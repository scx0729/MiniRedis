#pragma once

#include "miniredis/base/result.h"
#include <cstddef>
#include <string>
#include <string_view>

namespace miniredis {

// Growable byte buffer with append/peek/consume semantics.
// Tracks consumed vs unconsumed regions for incremental parsing.
class ByteBuffer {
public:
    explicit ByteBuffer(size_t max_size = 64 * 1024 * 1024);

    // Append data to the buffer. Returns error if would exceed max size.
    Result<void> Append(const char* data, size_t len);
    Result<void> Append(std::string_view data);

    // Peek at unconsumed data without consuming it
    std::string_view Peek() const;

    // Consume n bytes from the front of unconsumed data
    Result<void> Consume(size_t n);

    // Number of unconsumed bytes
    size_t Available() const;

    // Total capacity
    size_t Capacity() const;

    // Max allowed size
    size_t MaxSize() const;

    // Clear all data
    void Clear();

    // Compact: move unconsumed data to front to reclaim space
    void Compact();

private:
    std::string buffer_;
    size_t consumed_ = 0;  // bytes already consumed (before this offset)
    size_t max_size_;
};

}  // namespace miniredis
