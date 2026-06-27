// TODO(Task 6): implement non-blocking socket I/O around ByteBuffer.
#include "miniredis/server/connection.h"
#include <unistd.h>
#include <utility>
#include <sys/socket.h>
#include <cerrno>

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
    char buffer[65536];
    bool saw_data = false;
    while(true){
        auto n = recv(fd_, buffer, sizeof(buffer), 0);
        if(n > 0){
            saw_data = true;
            auto result = input_.Append(buffer, static_cast<size_t>(n));
            if(!result.ok()) { return false; }
            continue;
        } else if(n == 0){
            if(saw_data){
                closing_ = true;
                return true;
            }
            return false;
        } else {
            if(errno == EAGAIN || errno == EWOULDBLOCK) { return true; }
            if(errno == EINTR) { continue; }
            return false;
        }
    }
}

bool Connection::Write() {
    while(true){
        auto buffer = output_.Peek();
        if(buffer.empty()) break;
        auto n = send(fd_, buffer.data(), buffer.size(), MSG_NOSIGNAL);
        if(n > 0){
            output_.Consume(static_cast<size_t>(n));
        } else {
            if(errno == EAGAIN || errno == EWOULDBLOCK) { return true; }
            if(errno == EINTR) { continue; }
            return false;
        }
    }
    return true;
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
