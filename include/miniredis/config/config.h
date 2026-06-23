#pragma once

#include "miniredis/base/result.h"
#include <cstdint>
#include <string>

namespace miniredis {

struct Config {
    std::string bind_address = "127.0.0.1";
    uint16_t port = 6380;
    bool appendonly = true;
    std::string appendfilename = "miniredis.aof";
    bool fsync_always = true;
    size_t max_client_input_buffer = 64 * 1024 * 1024;
    size_t max_client_output_buffer = 64 * 1024 * 1024;
    size_t max_bulk_length = 512 * 1024 * 1024;
    int64_t active_expire_interval_ms = 100;
    std::string log_level = "info";

    // Load config from file
    static Result<Config> Load(const std::string& filename);

    // Parse a single key=value line
    Result<void> ParseLine(const std::string& line);
};

}  // namespace miniredis
