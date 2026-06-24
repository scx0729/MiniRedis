# Task 3 API: 数据库与字符串命令

## 目标

实现 String 类型、通用 key 命令和字符串命令。

## 主要文件

- `include/miniredis/storage/database.h`
- `src/storage/database.cpp`
- `src/command/string_commands.cpp`

## 规则

- 不修改公开函数签名。
- 可以新增私有 helper，但保持模块边界清晰。
- 返回 `Result<T>` 的函数应使用错误码表达可预期失败。
- 保持二进制安全，不依赖 C 字符串终止符。
- 后续任务的 TODO 不应在当前任务中提前实现，除非 README 明确要求。
