# Task 2 API: 增量 RESP 解析器

## 目标

实现可跨多次 Parse 调用保留状态的 RESP2 解析器，支持流水线和安全限制。

## 主要文件

- `include/miniredis/resp/parser.h`
- `src/resp/parser.cpp`

## 规则

- 不修改公开函数签名。
- 可以新增私有 helper，但保持模块边界清晰。
- 返回 `Result<T>` 的函数应使用错误码表达可预期失败。
- 保持二进制安全，不依赖 C 字符串终止符。
- 后续任务的 TODO 不应在当前任务中提前实现，除非 README 明确要求。
