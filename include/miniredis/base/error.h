#pragma once

#include <cstdint>
#include <string>

namespace miniredis {

enum class ErrorCode : uint8_t {
    kOk = 0,
    // Protocol errors
    kProtocolError,
    kInvalidBulkLength,
    kInvalidArrayLength,
    kMaxDepthExceeded,
    kMaxInputExceeded,
    // Command errors
    kWrongNumberOfArguments,
    kUnknownCommand,
    kSyntaxError,
    // Type errors
    kWrongType,
    kNotInteger,
    kOverflow,
    // Key errors
    kKeyNotFound,
    // IO errors
    kIoError,
    kConfigError,
};

inline const char* ErrorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::kOk: return "OK";
        case ErrorCode::kProtocolError: return "ERR protocol error";
        case ErrorCode::kInvalidBulkLength: return "ERR invalid bulk length";
        case ErrorCode::kInvalidArrayLength: return "ERR invalid array length";
        case ErrorCode::kMaxDepthExceeded: return "ERR max depth exceeded";
        case ErrorCode::kMaxInputExceeded: return "ERR max input exceeded";
        case ErrorCode::kWrongNumberOfArguments: return "ERR wrong number of arguments";
        case ErrorCode::kUnknownCommand: return "ERR unknown command";
        case ErrorCode::kSyntaxError: return "ERR syntax error";
        case ErrorCode::kWrongType: return "WRONGTYPE Operation against a key holding the wrong kind of value";
        case ErrorCode::kNotInteger: return "ERR value is not an integer or out of range";
        case ErrorCode::kOverflow: return "ERR increment or decrement would overflow";
        case ErrorCode::kKeyNotFound: return "ERR no such key";
        case ErrorCode::kIoError: return "ERR I/O error";
        case ErrorCode::kConfigError: return "ERR configuration error";
    }
    return "ERR unknown error";
}

struct Error {
    ErrorCode code;
    std::string message;

    Error() : code(ErrorCode::kOk) {}
    Error(ErrorCode c) : code(c), message(ErrorCodeToString(c)) {}
    Error(ErrorCode c, std::string msg) : code(c), message(std::move(msg)) {}

    bool ok() const { return code == ErrorCode::kOk; }
    explicit operator bool() const { return !ok(); }
};

}  // namespace miniredis
