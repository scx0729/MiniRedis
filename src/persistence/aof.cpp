// TODO(Task 8): implement the Append-Only File writer.
#include "miniredis/persistence/aof.h"
#include <fcntl.h>
#include <unistd.h>

namespace miniredis {

AofWriter::AofWriter(const std::string& filename, bool fsync_always)
    : filename_(filename), fd_(-1), fsync_always_(fsync_always) {
    fd_ = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
}

AofWriter::~AofWriter() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

Result<void> AofWriter::Write([[maybe_unused]] const std::string& data) {
    // TODO(Task 8): append all bytes to fd_, handling partial writes and EINTR.
    if (fd_ < 0) return Result<void>::Err(ErrorCode::kIoError, "AOF file not open");
    return Result<void>::Ok();
}

Result<void> AofWriter::Flush() {
    // TODO(Task 8): fdatasync/fsync according to project requirements.
    if (fsync_always_) {
        // Placeholder keeps the flag intentionally referenced until students implement it.
    }
    return Result<void>::Ok();
}

}  // namespace miniredis
