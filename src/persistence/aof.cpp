// TODO(Task 8): implement the Append-Only File writer.
#include "miniredis/persistence/aof.h"
#include "miniredis/base/error.h"
#include "miniredis/base/result.h"
#include <cerrno>
#include <cstring>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>

namespace miniredis {

AofWriter::AofWriter(const std::string& filename, bool fsync_always)
    : filename_(filename), fd_(-1), fsync_always_(fsync_always) {
    fd_ = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
}

AofWriter::~AofWriter() {
    if (fd_ >= 0) {
        Flush();
        close(fd_);
        fd_ = -1;
    }
}

Result<void> AofWriter::Write([[maybe_unused]] const std::string& data) {
    if (fd_ < 0) return Result<void>::Err(ErrorCode::kIoError, "AOF file not open");
    auto remaining = data.size();
    auto ptr = data.data();
    while(remaining > 0){
        auto n = write(fd_, ptr, remaining);
        if(n < 0) {
            if(errno == EINTR) continue;
            return Result<void>::Err(ErrorCode::kIoError, strerror(errno));
        }
        ptr += n;
        remaining -= static_cast<size_t>(n);
    }
    if(fsync_always_){
        auto r = Flush();
        if(!r.ok()) return r;
    }
    return Result<void>::Ok();
}

Result<void> AofWriter::Flush() {
    if(fd_ < 0) return Result<void>::Ok();
    if (fdatasync(fd_) < 0) {
        return Result<void>::Err(ErrorCode::kIoError, strerror(errno));
    }
    return Result<void>::Ok();
}

}  // namespace miniredis
