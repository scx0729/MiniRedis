# Task 0 API: 错误模型与 ByteBuffer

## 目标

实现可增长的 append/peek/consume 字节缓冲区，保证边界检查和 compact 语义。

## 主要文件

- `include/miniredis/base/result.h`
- `include/miniredis/server/byte_buffer.h`
- `src/server/byte_buffer.cpp`

## 规则

- 不修改公开函数签名。
- 可以新增私有 helper，但保持模块边界清晰。
- 返回 `Result<T>` 的函数应使用错误码表达可预期失败。
- 保持二进制安全，不依赖 C 字符串终止符。
- 后续任务的 TODO 不应在当前任务中提前实现，除非 README 明确要求。
