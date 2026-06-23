#pragma once

#include "error.h"
#include <optional>
#include <string>
#include <utility>

namespace miniredis {

template <typename T>
class Result {
public:
    Result(T value) : value_(std::move(value)), error_() {}
    Result(Error error) : value_(), error_(std::move(error)) {}

    static Result<T> Ok(T value) { return Result(std::move(value)); }
    static Result<T> Err(ErrorCode code) { return Result(Error(code)); }
    static Result<T> Err(ErrorCode code, const std::string& msg) {
        return Result(Error(code, msg));
    }

    bool ok() const { return error_.ok(); }
    explicit operator bool() const { return ok(); }

    const T& value() const { return *value_; }
    T& value() { return *value_; }
    T&& move_value() { return std::move(*value_); }

    const Error& error() const { return error_; }
    ErrorCode error_code() const { return error_.code; }
    const std::string& error_message() const { return error_.message; }

private:
    std::optional<T> value_;
    Error error_;
};

// Specialization for void
template <>
class Result<void> {
public:
    Result() : error_() {}
    Result(Error error) : error_(std::move(error)) {}

    static Result<void> Ok() { return Result(); }
    static Result<void> Err(ErrorCode code) { return Result(Error(code)); }
    static Result<void> Err(ErrorCode code, const std::string& msg) {
        return Result(Error(code, msg));
    }

    bool ok() const { return error_.ok(); }
    explicit operator bool() const { return ok(); }

    const Error& error() const { return error_; }
    ErrorCode error_code() const { return error_.code; }
    const std::string& error_message() const { return error_.message; }

private:
    Error error_;
};

}  // namespace miniredis
