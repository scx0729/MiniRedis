// TODO(Task 6): implement non-blocking socket I/O around ByteBuffer.
#include "miniredis/server/connection.h"
#include <unistd.h>
#include <utility>

namespace miniredis {

Connection::Connection(int fd, size_t max_input_size, size_t max_output_size)
    : fd_(fd), input_(max_input_size), output_(max_output_size) {}

Connection::~Connection() {
    Close();
}

Connection::Connection(Connection&& other) noexcept
    : fd_(other.fd_),
      input_(std::move(other.input_)),
      output_(std::move(other.output_)),
      closing_(other.closing_) {
    other.fd_ = -1;
}

Connection& Connection::operator=(Connection&& other) noexcept {
    if (this != &other) {
        Close();
        fd_ = other.fd_;
        input_ = std::move(other.input_);
        output_ = std::move(other.output_);
        closing_ = other.closing_;
        other.fd_ = -1;
    }
    return *this;
}

bool Connection::Read() {
    // TODO(Task 6): recv() until EAGAIN/EWOULDBLOCK, appending bytes to input_.
    return false;
}

bool Connection::Write() {
    // TODO(Task 6): send() from output_ until empty or EAGAIN/EWOULDBLOCK.
    return false;
}

bool Connection::QueueOutput(const char* data, size_t len) {
    return output_.Append(data, len).ok();
}

bool Connection::QueueOutput(const std::string& data) {
    return QueueOutput(data.data(), data.size());
}

bool Connection::HasOutput() const {
    return output_.Available() > 0;
}

void Connection::Close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

}  // namespace miniredis
