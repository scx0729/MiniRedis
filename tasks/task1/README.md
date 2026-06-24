# Task 1: RESP 值类型和编码器

## 背景

Redis 使用 RESP（Redis 序列化协议）进行客户端与服务器之间的通信。在解析传入数据之前，我们需要在内存中表示 RESP 值，并将其编码为线路格式。

## 学习目标

- RESP2 数据模型（简单字符串、错误、整数、批量字符串、数组）
- 二进制安全以及为什么需要长度前缀格式
- 协议设计中 Null 和 Empty 的区别
- 递归数据结构（数组可以包含数组）

## 你的任务

### 1. 实现 RESP 值类型

`include/miniredis/resp/value.h` 中的 `RespValue` 类应该已经完成。验证它能处理所有 RESP2 类型。

### 2. 实现编码器

在 `src/resp/encoder.cpp` 中实现 `RespEncoder::Encode`：

- `+OK\r\n` 表示简单字符串
- `-ERR message\r\n` 表示错误
- `:42\r\n` 表示整数
- `$5\r\nhello\r\n` 表示批量字符串（长度前缀）
- `$-1\r\n` 表示空批量字符串（Null）
- `*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n` 表示数组
- `*-1\r\n` 表示空数组（Null）

### 3. 编写测试

在 `tests/unit/resp_encoder_test.cpp` 中编写测试用例：
- 空字符串、包含 CRLF 的字符串、包含空字节的字符串
- 正数、负数、零整数
- Null 批量字符串 vs 空批量字符串
- 嵌套数组
- 包含 null 元素的数组

## 需要修改的文件

- `src/resp/encoder.cpp`（必须）
- `tests/unit/resp_encoder_test.cpp`（必须）

## 关键接口

```cpp
class RespEncoder {
public:
    static std::string Encode(const RespValue& value);
    static std::string Encode(const std::optional<RespValue>& value);
};
```

## 不变量

- Null 批量字符串（`$-1\r\n`）与空批量字符串（`$0\r\n\r\n`）是不同的
- 批量字符串的长度计算的是字节数，不是字符数
- 数组的长度计算的是元素个数，不是字节数
- 所有行以 `\r\n` 结尾

## 常见错误

- 混淆 Null 批量字符串和空批量字符串
- 不具备二进制安全（使用了在 `\0` 处停止的 C 字符串函数）
- 批量字符串的长度计算错误

## 测试

```bash
cmake --build build-debug --target resp_encoder_test
./build-debug/resp_encoder_test
```

## 评分

- 正确性：60%
- 二进制安全（空字节、数据中的 CRLF）：15%
- Null 与 Empty 的区分：10%
- 代码结构：10%
- 测试覆盖率：5%
