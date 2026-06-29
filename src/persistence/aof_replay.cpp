// TODO(Task 8): implement AOF replay.
#include "miniredis/persistence/aof_replay.h"
#include "miniredis/base/error.h"
#include "miniredis/base/result.h"
#include "miniredis/command/command.h"
#include "miniredis/command/executor.h"
#include "miniredis/resp/parser.h"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace miniredis {

Result<int64_t>
AofReplay::Replay([[maybe_unused]] const std::string &filename,
                  [[maybe_unused]] Database &db,
                  [[maybe_unused]] const CommandRegistry &registry) {
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0) {
    if (errno == ENOENT)
      return Result<int64_t>::Ok(0);
    return Result<int64_t>::Err(ErrorCode::kIoError, strerror(errno));
  }

  RespParser parser;
  CommandContext ctx{&db, nullptr};
  int64_t count = 0;
  char buffer[65536];

  while (true) {
    auto n = read(fd, buffer, sizeof(buffer));
    if (n < 0) {
      if (errno == EINTR)
        continue;
      close(fd);
      return Result<int64_t>::Err(ErrorCode::kIoError, strerror(errno));
    }
    if (n == 0)
      break;

    std::string_view input(buffer, static_cast<size_t>(n));
    while (true) {
      auto result = parser.Parse(input);
      input = "";

      if (result.status == ParseStatus::kNeedMoreData) {
        break;
      }
      if (result.status == ParseStatus::kProtocolError) {
        close(fd);
        return Result<int64_t>::Ok(count);
      }

      const auto &value = *result.value;
      if (value.type() != RespType::kArray || value.is_null()) {
        close(fd);
        return Result<int64_t>::Ok(count);
      }

      std::vector<std::string> args;
      bool valid = true;
      for (const auto &element : value.array()) {
        if (!element.has_value() || element->is_null() ||
            element->type() != RespType::kBulkString) {
          valid = false;
          break;
        }
        args.push_back(element->bulk_string());
      }
      if (!valid || args.empty()) {
        close(fd);
        return Result<int64_t>::Ok(count);
      }

      auto response = CommandExecutor::Execute(ctx, registry, args);
      if (response.type() == RespType::kError) {
        close(fd);
        return Result<int64_t>::Ok(count);
      }
      ++count;

      if (!parser.HasBufferedData()) {
        break;
      }
    }
  }

  close(fd);
  return Result<int64_t>::Ok(count);
}

} // namespace miniredis
