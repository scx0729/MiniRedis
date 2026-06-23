#pragma once

#include "miniredis/base/result.h"
#include <cstdint>
#include <string>

namespace miniredis {

// AOF (Append-Only File) writer
class AofWriter {
public:
    explicit AofWriter(const std::string& filename, bool fsync_always = true);
    ~AofWriter();

    // Non-copyable
    AofWriter(const AofWriter&) = delete;
    AofWriter& operator=(const AofWriter&) = delete;

    // Append a RESP-encoded command to the AOF file
    Result<void> Write(const std::string& data);

    // Flush buffered data to disk
    Result<void> Flush();

    // Get filename
    const std::string& Filename() const { return filename_; }

private:
    std::string filename_;
    int fd_;
    bool fsync_always_;
};

}  // namespace miniredis
