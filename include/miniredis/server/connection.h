#pragma once

#include "byte_buffer.h"
#include <cstddef>
#include <cstdint>

namespace miniredis {

// Represents a client connection with non-blocking socket I/O
class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    // Non-copyable, movable
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;

    // Read from socket into input buffer. Returns false on connection close/error.
    bool Read();

    // Write from output buffer to socket. Returns false on error.
    // Returns true if all output data has been sent.
    bool Write();

    // Queue data to be sent
    void QueueOutput(const char* data, size_t len);
    void QueueOutput(const std::string& data);

    // Check if there is data waiting to be sent
    bool HasOutput() const;

    // Get the file descriptor
    int Fd() const { return fd_; }

    // Get input buffer for reading
    ByteBuffer& InputBuffer() { return input_; }
    const ByteBuffer& InputBuffer() const { return input_; }

    // Get output buffer
    ByteBuffer& OutputBuffer() { return output_; }

    // Close the connection
    void Close();

    // Whether connection is open
    bool IsOpen() const { return fd_ >= 0; }

    // Mark for closing (after writing remaining data)
    void MarkClosing() { closing_ = true; }
    bool IsClosing() const { return closing_; }

private:
    int fd_;
    ByteBuffer input_;
    ByteBuffer output_;
    bool closing_ = false;
};

}  // namespace miniredis
