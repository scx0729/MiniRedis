#include "miniredis/persistence/aof_replay.h"
#include "miniredis/command/command.h"
#include "miniredis/command/executor.h"
#include "miniredis/command/registry.h"
#include "miniredis/resp/parser.h"
#include "miniredis/storage/database.h"
#include <fcntl.h>
#include <unistd.h>

namespace miniredis {

Result<int64_t> AofReplay::Replay(const std::string& filename, Database& db,
                                   const CommandRegistry& registry) {
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        // File doesn't exist - that's OK
        if (errno == ENOENT) return Result<int64_t>::Ok(0);
        return Result<int64_t>::Err(ErrorCode::kIoError, "cannot open AOF file");
    }

    // Create a temporary AOF writer that is null (we don't want to re-write during replay)
    // We pass a dummy context
    CommandContext ctx{&db, nullptr};

    RespParser parser;
    int64_t count = 0;
    char buf[65536];
    std::string accumulated;

    while (true) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return Result<int64_t>::Err(ErrorCode::kIoError, "error reading AOF file");
        }
        if (n == 0) break;  // EOF

        accumulated.append(buf, static_cast<size_t>(n));

        // Parse all complete commands
        while (!accumulated.empty()) {
            auto result = parser.Parse(accumulated);

            if (result.status == ParseStatus::kNeedMoreData) {
                break;
            }

            if (result.status == ParseStatus::kProtocolError) {
                // Truncated or corrupted tail - stop replay
                // This is expected for partial writes at end of file
                close(fd);
                return Result<int64_t>::Ok(count);
            }

            // Complete command
            accumulated.erase(0, result.consumed);

            const auto& value = *result.value;
            if (value.type() != RespType::kArray || value.is_null()) {
                continue;  // skip invalid entries
            }

            const auto& arr = value.array();
            if (arr.empty()) continue;

            // Convert to command args
            std::vector<std::string> args;
            bool valid = true;
            for (const auto& elem : arr) {
                if (!elem.has_value() || elem->type() != RespType::kBulkString || elem->is_null()) {
                    valid = false;
                    break;
                }
                args.push_back(elem->bulk_string());
            }

            if (!valid || args.empty()) continue;

            // Execute command (with aof=nullptr to avoid re-writing)
            CommandExecutor::Execute(ctx, registry, args);
            ++count;
        }
    }

    close(fd);
    return Result<int64_t>::Ok(count);
}

}  // namespace miniredis
