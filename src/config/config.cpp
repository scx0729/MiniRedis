#include "miniredis/config/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace miniredis {

Result<Config> Config::Load(const std::string& filename) {
    Config config;
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Return default config if file not found
        return Result<Config>::Ok(std::move(config));
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        auto result = config.ParseLine(line);
        if (!result.ok()) {
            return Result<Config>::Err(result.error_code(), result.error_message());
        }
    }

    return Result<Config>::Ok(std::move(config));
}

Result<void> Config::ParseLine(const std::string& line) {
    // Find first whitespace to split key and value
    size_t space_pos = line.find_first_of(" \t");
    if (space_pos == std::string::npos) {
        return Result<void>::Err(ErrorCode::kConfigError, "invalid config line: " + line);
    }

    std::string key = line.substr(0, space_pos);
    std::string value = line.substr(space_pos + 1);
    // Trim leading whitespace from value
    size_t start = value.find_first_not_of(" \t");
    if (start != std::string::npos) {
        value = value.substr(start);
    }
    // Trim trailing whitespace
    size_t end = value.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        value = value.substr(0, end + 1);
    }

    if (key == "bind") {
        bind_address = value;
    } else if (key == "port") {
        char* endp = nullptr;
        long val = strtol(value.c_str(), &endp, 10);
        if (*endp != '\0' || val <= 0 || val > 65535) {
            return Result<void>::Err(ErrorCode::kConfigError, "invalid port: " + value);
        }
        port = static_cast<uint16_t>(val);
    } else if (key == "appendonly") {
        appendonly = (value == "yes");
    } else if (key == "appendfilename") {
        appendfilename = value;
    } else if (key == "appendfsync") {
        fsync_always = (value == "always");
    } else if (key == "max-client-input-buffer") {
        char* endp = nullptr;
        unsigned long long val = strtoull(value.c_str(), &endp, 10);
        if (*endp != '\0') {
            return Result<void>::Err(ErrorCode::kConfigError, "invalid buffer size: " + value);
        }
        max_client_input_buffer = static_cast<size_t>(val);
    } else if (key == "max-client-output-buffer") {
        char* endp = nullptr;
        unsigned long long val = strtoull(value.c_str(), &endp, 10);
        if (*endp != '\0') {
            return Result<void>::Err(ErrorCode::kConfigError, "invalid buffer size: " + value);
        }
        max_client_output_buffer = static_cast<size_t>(val);
    } else if (key == "max-bulk-length") {
        char* endp = nullptr;
        unsigned long long val = strtoull(value.c_str(), &endp, 10);
        if (*endp != '\0') {
            return Result<void>::Err(ErrorCode::kConfigError, "invalid bulk length: " + value);
        }
        max_bulk_length = static_cast<size_t>(val);
    } else if (key == "active-expire-interval-ms") {
        char* endp = nullptr;
        long long val = strtoll(value.c_str(), &endp, 10);
        if (*endp != '\0' || val <= 0) {
            return Result<void>::Err(ErrorCode::kConfigError, "invalid interval: " + value);
        }
        active_expire_interval_ms = val;
    } else if (key == "log-level") {
        log_level = value;
    } else {
        // Unknown key - ignore (for forward compatibility)
    }

    return Result<void>::Ok();
}

}  // namespace miniredis
