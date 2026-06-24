# Task 1 API: RESP 值与编码器

## 目标

把内存中的 RespValue 编码为 RESP2 wire format，保持二进制安全。

## 主要文件

- `include/miniredis/resp/value.h`
- `include/miniredis/resp/encoder.h`
- `src/resp/encoder.cpp`

## 规则

- 不修改公开函数签名。
- 可以新增私有 helper，但保持模块边界清晰。
- 返回 `Result<T>` 的函数应使用错误码表达可预期失败。
- 保持二进制安全，不依赖 C 字符串终止符。
- 后续任务的 TODO 不应在当前任务中提前实现，除非 README 明确要求。
