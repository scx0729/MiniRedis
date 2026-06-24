#pragma once

#include "value.h"
#include "miniredis/base/result.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace miniredis {

// Parser limits
struct ParserLimits {
    size_t max_bulk_length = 512 * 1024 * 1024;  // 512 MB
    size_t max_array_length = 10000;
    size_t max_depth = 100;
};

enum class ParseStatus : uint8_t {
    kComplete,
    kNeedMoreData,
    kProtocolError,
};

struct ParseResult {
    ParseStatus status;
    size_t consumed;  // bytes consumed from input
    std::optional<RespValue> value;  // valid if status == kComplete
    Error error;  // valid if status == kProtocolError

    static ParseResult Complete(size_t consumed, RespValue value) {
        return {ParseStatus::kComplete, consumed, std::move(value), {}};
    }

    static ParseResult NeedMoreData() {
        return {ParseStatus::kNeedMoreData, 0, std::nullopt, {}};
    }

    static ParseResult ProtocolError(Error error) {
        return {ParseStatus::kProtocolError, 0, std::nullopt, std::move(error)};
    }
};

// Incremental RESP2 parser. Callers feed newly received bytes; the parser keeps
// incomplete and pipelined input internally until values are completed.
class RespParser {
public:
    explicit RespParser(ParserLimits limits = {});

    // Feed input bytes. Returns parse result.
    // Parser maintains internal state for incremental parsing.
    ParseResult Parse(std::string_view input);

    // Reset parser state for a new connection
    void Reset();

    // Whether the parser has buffered bytes that may contain another value.
    bool HasBufferedData() const;

private:
    // Parsing state
    enum class State : uint8_t {
        kTypePrefix,      // expecting type prefix byte
        kSimpleString,    // reading simple string until CRLF
        kErrorString,     // reading error string until CRLF
        kInteger,         // reading integer until CRLF
        kBulkLength,      // reading bulk string length until CRLF
        kBulkData,        // reading bulk string data
        kArrayLength,     // reading array length until CRLF
    };

    struct Frame {
        RespType type;
        int64_t length;  // for bulk string: byte count; for array: element count
        size_t start_offset;  // for tracking consumed bytes
    };

    ParseResult ParseInternal(std::string_view input, size_t& offset, size_t depth);

    // Find CRLF in input starting from offset. Returns position of \r or npos.
    size_t FindCRLF(std::string_view input, size_t offset) const;

    // Parse integer from string, checking for overflow and invalid chars
    Result<int64_t> ParseInt(std::string_view s) const;

    ParserLimits limits_;
    std::string input_buffer_;  // accumulates partial/pipelined input across calls
};

}  // namespace miniredis
