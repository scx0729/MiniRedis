#include "miniredis/server/connection.h"
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

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
    char buf[65536];
    bool saw_data = false;
    while (true) {
        ssize_t n = recv(fd_, buf, sizeof(buf), 0);
        if (n > 0) {
            saw_data = true;
            auto result = input_.Append(buf, static_cast<size_t>(n));
            if (!result.ok()) {
                return false;  // buffer full, close connection
            }
            continue;  // try to read more
        }
        if (n == 0) {
            if (saw_data) {
                closing_ = true;
                return true;
            }
            return false;  // connection closed by peer
        }
        // n < 0
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;  // no more data available
        }
        if (errno == EINTR) {
            continue;  // interrupted, retry
        }
        return false;  // actual error
    }
}

bool Connection::Write() {
    while (HasOutput()) {
        auto data = output_.Peek();
        if (data.empty()) break;

        ssize_t n = send(fd_, data.data(), data.size(), MSG_NOSIGNAL);
        if (n > 0) {
            output_.Consume(static_cast<size_t>(n));
            continue;
        }
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true;  // can't write more right now
            }
            if (errno == EINTR) {
                continue;
            }
            return false;  // actual error
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
