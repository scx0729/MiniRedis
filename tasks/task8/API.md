# Task 8 API: AOF 持久化与恢复

## 目标

实现 RESP 格式 AOF 写入、启动重放和 TCP 重启恢复。

## 主要文件

- `include/miniredis/persistence/aof.h`
- `include/miniredis/persistence/aof_replay.h`
- `src/persistence/aof.cpp`
- `src/persistence/aof_replay.cpp`
- `src/server/session.cpp`
- `src/server/server.cpp`

## 规则

- 不修改公开函数签名。
- 可以新增私有 helper，但保持模块边界清晰。
- 返回 `Result<T>` 的函数应使用错误码表达可预期失败。
- 保持二进制安全，不依赖 C 字符串终止符。
- 后续任务的 TODO 不应在当前任务中提前实现，除非 README 明确要求。
