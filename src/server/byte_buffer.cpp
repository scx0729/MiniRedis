// TODO(Task 0): implement ByteBuffer append/peek/consume semantics.
// Keep the public interface in include/miniredis/server/byte_buffer.h unchanged.
#include "miniredis/server/byte_buffer.h"
#include "miniredis/base/error.h"
#include "miniredis/base/result.h"
#include <string_view>

namespace miniredis {

ByteBuffer::ByteBuffer(size_t max_size) : max_size_(max_size) {}

Result<void> ByteBuffer::Append([[maybe_unused]] const char* data, [[maybe_unused]] size_t len) {
    // Append 之后，缓冲区里未消费的数据会变成 Available() + len
    if(Available() + len > max_size_){
        return Result<void>::Err(ErrorCode::kMaxInputExceeded, "buffer limit exceeded");
    }
    buffer_.append(data, len);
    return Result<void>::Ok();
}

Result<void> ByteBuffer::Append(std::string_view data) {
    return Append(data.data(), data.size());
}

std::string_view ByteBuffer::Peek() const {
    return std::string_view(buffer_).substr(consumed_);
}

Result<void> ByteBuffer::Consume([[maybe_unused]] size_t n) {
    if(consumed_ > (buffer_.size() / 2) && consumed_ >= 4096){
        Compact();
    }
    if(consumed_ + n > buffer_.size()){
        return Result<void>::Err(ErrorCode::kProtocolError, "consume beyond available");
    }
    consumed_ += n;
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
    buffer_.erase(0, consumed_);
    consumed_ = 0;
}

}  // namespace miniredis
