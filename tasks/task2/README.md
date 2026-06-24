# Task 2: 增量 RESP 解析器

## 背景

网络数据是分块到达的。一次 `recv()` 可能只返回半个命令，或者三个命令拼接在一起。增量解析器处理当前已有的数据，并记住状态以等待下一块数据。这是协议层中技术难度最高的部分。

## 学习目标

- 增量解析的状态机设计
- 处理半包（需要更多数据）与完整包
- 处理一个缓冲区中的多个命令
- 解析器安全限制（最大深度、最大长度）
- "需要更多数据"和"协议错误"的区别

## 你的任务

在 `src/resp/parser.cpp` 中实现 `RespParser` 类。解析器必须：

1. 接受部分输入，当完整消息不可用时返回 `NeedMoreData`
2. 解析一条完整消息，返回它以及消耗的字节数
3. 在调用之间保持状态以支持增量解析
4. 处理所有 RESP2 类型（简单字符串、错误、整数、批量字符串、数组）
5. 强制安全限制（最大批量长度、最大数组元素数、最大深度）
6. 对格式错误的输入返回 `ProtocolError`

### 关键测试用例

你的解析器必须能处理：
- 每次只喂入一个字节
- 随机大小的数据块
- 头部和体部分离
- CR 和 LF 跨数据块分离
- 一个输入中包含多个完整命令
- 完整命令后跟半个命令
- 包含空字节的二进制数据

## 需要修改的文件

- `src/resp/parser.cpp`（必须）
- `tests/unit/resp_parser_test.cpp`（必须）

## 关键接口

```cpp
class RespParser {
public:
    ParseResult Parse(std::string_view input);
    void Reset();
};

struct ParseResult {
    ParseStatus status;  // Complete、NeedMoreData、ProtocolError
    size_t consumed;     // 消耗的字节数（Complete 时有效）
    std::optional<RespValue> value;  // 解析出的值（Complete 时有效）
    Error error;         // 错误信息（ProtocolError 时有效）
};
```

## 实现提示

- 考虑使用状态机方法：跟踪下一步期望什么（类型前缀、CRLF、数据等）
- 对于批量字符串：先解析长度，然后累积数据字节
- 对于数组：递归解析元素（但注意深度限制）
- 保留一个缓冲区用于跨调用的不完整数据

## 常见错误

- 每次调用都从头重新解析整个缓冲区（浪费且隐藏 bug）
- 没有跟踪消耗了多少字节（导致重复处理）
- 混淆 `$-1\r\n`（null）和 `$0\r\n\r\n`（空）
- 没有限制嵌套数组的递归深度
- 实际是协议错误时却返回 NeedMoreData

## 测试

```bash
cmake --build build-debug --target resp_parser_test
./build-debug/resp_parser_test
```

## 评分

- 正确性：50%
- 增量解析（逐字节、随机数据块）：20%
- 错误处理：15%
- 安全限制：10%
- 代码结构：5%
