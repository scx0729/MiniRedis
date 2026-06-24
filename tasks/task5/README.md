# Task 5: 会话和命令管线化

## 背景

Redis 客户端可以在不等待响应的情况下发送多个命令（管线化）。Session 将解析器、命令执行器和连接缓冲区组合在一起。它从输入缓冲区读取数据，解析命令，执行命令，并将响应写入输出缓冲区。

## 学习目标

- 协议解析如何连接到命令执行
- 管线化支持（一次读取处理多个命令）
- 会话状态管理
- 错误恢复（协议错误关闭连接）
- QUIT 命令的行为

## 你的任务

在 `src/server/session.cpp` 中实现 `Session::ProcessInput()`：

1. 循环：尝试从输入缓冲区解析一条命令
2. 如果解析器需要更多数据，跳出循环
3. 如果是协议错误，发送错误响应并返回 false
4. 从解析出的 RESP 数组中提取命令参数
5. 执行命令
6. 编码响应并追加到输出缓冲区
7. 从输入缓冲区消费已解析的字节
8. 处理 QUIT 命令（标记连接为待关闭）

## 关键设计

Session 应该可以在没有真实 socket 的情况下测试。它操作于：
- 输入 ByteBuffer（由 Connection::Read 填充）
- 输出 ByteBuffer（由 Connection::Write 排空）

## 需要修改的文件

- `src/server/session.cpp`（必须）
- `tests/integration/session_test.cpp`（必须）

## 常见错误

- 没有消费已解析的字节（解析器重复解析相同数据）
- 没有处理协议错误（对错误输入死循环）
- Session 直接耦合 socket I/O
- 没有特殊处理 QUIT

## 测试

```bash
cmake --build build-debug --target session_test
./build-debug/session_test
```

## 评分

- 正确性：50%
- 管线化支持：20%
- 错误处理：15%
- 代码结构：10%
- 测试覆盖率：5%
