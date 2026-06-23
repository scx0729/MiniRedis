#pragma once

#include "command.h"
#include <optional>
#include <string>
#include <unordered_map>

namespace miniredis {

class CommandRegistry {
public:
    CommandRegistry();

    // Look up command by name (case-insensitive)
    const Command* Find(const std::string& name) const;

    // Register a command
    void Register(Command cmd);

private:
    static std::string ToUpper(const std::string& s);
    std::unordered_map<std::string, Command> commands_;
};

}  // namespace miniredis
