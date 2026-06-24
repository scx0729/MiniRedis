// TODO(Task 0): implement ByteBuffer append/peek/consume semantics.
// Keep the public interface in include/miniredis/server/byte_buffer.h unchanged.
#include "miniredis/server/byte_buffer.h"

namespace miniredis {

ByteBuffer::ByteBuffer(size_t max_size) : max_size_(max_size) {}

Result<void> ByteBuffer::Append(const char*, size_t) {
    // TODO(Task 0): append bytes while enforcing max_size_.
    return Result<void>::Ok();
}

Result<void> ByteBuffer::Append(std::string_view data) {
    return Append(data.data(), data.size());
}

std::string_view ByteBuffer::Peek() const {
    // TODO(Task 0): return only the unconsumed slice.
    return {};
}

Result<void> ByteBuffer::Consume(size_t) {
    // TODO(Task 0): advance the consumed offset and reject over-consume.
    return Result<void>::Ok();
}

size_t ByteBuffer::Available() const {
    // TODO(Task 0): report unconsumed byte count.
    return 0;
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
    // TODO(Task 0): reclaim consumed prefix without changing remaining bytes.
}

}  // namespace miniredis
