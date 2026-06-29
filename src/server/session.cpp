// TODO(Task 5, Task 8): implement session command processing and AOF logging.
#include "miniredis/server/session.h"
#include "miniredis/command/executor.h"
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/parser.h"
#include "miniredis/resp/value.h"
#include "miniredis/persistence/aof.h"
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

namespace miniredis {

Session::Session(Connection& conn, Database& db, const CommandRegistry& registry,
                 CommandContext& cmd_ctx, ParserLimits parser_limits)
    : conn_(conn), db_(db), registry_(registry), cmd_ctx_(cmd_ctx),
      parser_(parser_limits) {}

bool Session::ProcessInput() {
    while(conn_.InputBuffer().Available() > 0 || parser_.HasBufferedData()){
      // 字节喂给解析器
      auto input = conn_.InputBuffer().Peek();
      auto result = parser_.Parse(input);
      if(!conn_.InputBuffer().Consume(input.size()).ok()){
        return false;
      }
      // 根据解析结果分支
      // A:数据不够
      if(result.status == ParseStatus::kNeedMoreData){
        break;
      }
      // B:协议错误
      if(result.status == ParseStatus::kProtocolError){
        auto err = RespEncoder::Encode(RespValue::Error(result.error.message));
        conn_.QueueOutput(err);
        return false;
      }
      // 解析成功 验证解析出的值
      const auto& value = *result.value;
      // Redis协议规定，客户端发来的命令必须是一个RESP数组
      if(value.type() != RespType::kArray || value.is_null()){
        auto err = RespEncoder::Encode(RespValue::Error("ERR protocol error: expected array"));
        conn_.QueueOutput(err);
        return false;
      }
      const auto& arr = value.array();
      if(arr.empty()){
        auto err = RespEncoder::Encode(RespValue::Error("ERR protocol error: empty command"));
        conn_.QueueOutput(err);
        return false;
      }
      // 提取命令参数
      std::vector<std::string> args;
      bool valid = true;
      for(const auto& element : arr){
        if(!element.has_value() || element->is_null() || element->type() != RespType::kBulkString){
          valid = false;
          break;
        }
        args.push_back(element->bulk_string());
      }
      if(!valid || args.empty()){
        auto err = RespEncoder::Encode(RespValue::Error("ERR protocol error: invalid command format"));
        conn_.QueueOutput(err);
        return false;
      }
      // 执行命令
      auto response = CommandExecutor::Execute(cmd_ctx_,registry_, args);
      auto cmd = args[0];
      const auto command = registry_.Find(cmd);
      if(response.type() != RespType::kError && command != nullptr && command->def.aof && cmd_ctx_.aof != nullptr){
        RespValue::ArrayT aof_args;
        for(const auto& arg : args){
          aof_args.push_back(RespValue::BulkString(arg));
        }
        auto encoded_cmd = RespEncoder::Encode(RespValue::MakeArray(aof_args));
        auto write_result = cmd_ctx_.aof->Write(encoded_cmd);
        if(!write_result.ok()){
          auto err = RespEncoder::Encode((RespValue::Error(write_result.error_message())));
          conn_.QueueOutput(err);
          return false;
        }
      }
      // 编码响应并写入输出缓冲区
      auto encoded = RespEncoder::Encode(response);
      if(!conn_.QueueOutput(encoded)){
        return false;
      }
      // 处理QUIT
      for(auto &ch : cmd) { ch = static_cast<char>(std::toupper(ch)); }
      if(cmd == "QUIT"){
        conn_.MarkClosing();
        return true;
      }
    }
    return true;
}

}  // namespace miniredis
