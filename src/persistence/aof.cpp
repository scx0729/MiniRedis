#include "miniredis/persistence/aof.h"
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace miniredis {

AofWriter::AofWriter(const std::string& filename, bool fsync_always)
    : filename_(filename), fd_(-1), fsync_always_(fsync_always) {
    fd_ = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd_ < 0) {
        // Try to create in current directory
        fd_ = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
}

AofWriter::~AofWriter() {
    if (fd_ >= 0) {
        Flush();
        close(fd_);
    }
}

Result<void> AofWriter::Write(const std::string& data) {
    if (fd_ < 0) {
        return Result<void>::Err(ErrorCode::kIoError, "AOF file not open");
    }

    const char* ptr = data.data();
    size_t remaining = data.size();

    while (remaining > 0) {
        ssize_t n = write(fd_, ptr, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            return Result<void>::Err(ErrorCode::kIoError, strerror(errno));
        }
        ptr += n;
        remaining -= static_cast<size_t>(n);
    }

    if (fsync_always_) {
        return Flush();
    }

    return Result<void>::Ok();
}

Result<void> AofWriter::Flush() {
    if (fd_ < 0) return Result<void>::Ok();

    if (fdatasync(fd_) < 0) {
        if (errno == EINVAL || errno == ENOSYS) {
            // fdatasync not supported, try fsync
            if (fsync(fd_) < 0) {
                return Result<void>::Err(ErrorCode::kIoError, strerror(errno));
            }
        } else {
            return Result<void>::Err(ErrorCode::kIoError, strerror(errno));
        }
    }
    return Result<void>::Ok();
}

}  // namespace miniredis
