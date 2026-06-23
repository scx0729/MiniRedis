#include "miniredis/server/byte_buffer.h"
#include <algorithm>
#include <cstring>

namespace miniredis {

ByteBuffer::ByteBuffer(size_t max_size) : max_size_(max_size) {}

Result<void> ByteBuffer::Append(const char* data, size_t len) {
    if (Available() + len > max_size_) {
        return Result<void>::Err(ErrorCode::kMaxInputExceeded, "buffer limit exceeded");
    }
    buffer_.append(data, len);
    return Result<void>::Ok();
}

Result<void> ByteBuffer::Append(std::string_view data) {
    return Append(data.data(), data.size());
}

std::string_view ByteBuffer::Peek() const {
    if (consumed_ >= buffer_.size()) return {};
    return std::string_view(buffer_).substr(consumed_);
}

Result<void> ByteBuffer::Consume(size_t n) {
    if (consumed_ + n > buffer_.size()) {
        return Result<void>::Err(ErrorCode::kProtocolError, "consume beyond available");
    }
    consumed_ += n;
    // Auto-compact when consumed > half of buffer
    if (consumed_ > buffer_.size() / 2 && consumed_ > 4096) {
        Compact();
    }
    return Result<void>::Ok();
}

size_t ByteBuffer::Available() const {
    return buffer_.size() - consumed_;
}

size_t ByteBuffer::Capacity() const {
    return buffer_.capacity();
}

size_t ByteBuffer::MaxSize() const {
    return max_size_;
}

void ByteBuffer::Clear() {
    buffer_.clear();
    consumed_ = 0;
}

void ByteBuffer::Compact() {
    if (consumed_ == 0) return;
    if (consumed_ >= buffer_.size()) {
        buffer_.clear();
        consumed_ = 0;
        return;
    }
    buffer_.erase(0, consumed_);
    consumed_ = 0;
}

}  // namespace miniredis
