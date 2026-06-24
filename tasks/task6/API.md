# Task 6 API: 非阻塞 TCP 与 epoll

## 目标

实现真实 TCP server、非阻塞读写、epoll 分发和优雅关闭。

## 主要文件

- `include/miniredis/server/connection.h`
- `include/miniredis/server/event_loop.h`
- `include/miniredis/server/server.h`
- `src/server/connection.cpp`
- `src/server/event_loop.cpp`
- `src/server/server.cpp`

## 规则

- 不修改公开函数签名。
- 可以新增私有 helper，但保持模块边界清晰。
- 返回 `Result<T>` 的函数应使用错误码表达可预期失败。
- 保持二进制安全，不依赖 C 字符串终止符。
- 后续任务的 TODO 不应在当前任务中提前实现，除非 README 明确要求。
