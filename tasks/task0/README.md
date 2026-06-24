# Task 0: 错误模型、ByteBuffer 和项目搭建

## 背景

在构建网络服务器之前，我们需要一些基础组件：一种不依赖异常的错误处理方式，以及一个可以增长和逐步消费的字节缓冲区。这些对于非阻塞 I/O 至关重要，因为一次 `recv()` 调用永远无法保证获取到完整的请求。

## 学习目标

完成本任务后，你将理解：
- CMake 项目结构以及如何构建和测试
- RAII（资源获取即初始化）
- 使用 Result<T> 进行无异常错误处理
- 网络程序的字节缓冲区设计
- 为什么不能假设一次 `recv()` 就能获取完整请求

## 当前状态

项目已能编译，测试框架已搭建好。`ByteBuffer` 和 `Result<T>` 类型的头文件接口已定义，但实现部分是空的桩代码。

## 你的任务

### 1. 实现 `Result<T>`（如果尚未完成）

`Result<T>` 类型应持有 `T` 类型的值或一个 `Error`。在整个代码库中，它被用来替代异常处理可预期的错误。

### 2. 实现 `ByteBuffer`

在 `src/server/byte_buffer.cpp` 中实现 `ByteBuffer` 类：

- `Append(data, len)`：向缓冲区追加数据。如果超过最大大小则返回错误。
- `Peek()`：返回未消费数据的视图，不消费数据。
- `Consume(n)`：标记 n 字节为已消费。
- `Available()`：返回未消费的字节数。
- `Compact()`：将未消费的数据移到缓冲区前端以回收空间。

### 3. 编写测试

在 `tests/unit/byte_buffer_test.cpp` 中添加测试用例：
- 空缓冲区
- 多次追加
- 部分消费
- 全部消费
- 最大大小限制
- 压缩后的数据

## 需要修改的文件

- `src/server/byte_buffer.cpp`（必须）
- `tests/unit/byte_buffer_test.cpp`（必须）

## 关键接口

```cpp
class ByteBuffer {
public:
    explicit ByteBuffer(size_t max_size);
    Result<void> Append(const char* data, size_t len);
    std::string_view Peek() const;
    Result<void> Consume(size_t n);
    size_t Available() const;
    void Compact();
    void Clear();
};
```

## 不变量

- `Available()` 始终返回 `Peek()` 中未消费的字节数
- `Consume(n)` 之后，`Peek()` 从之前的视图的第 n 字节开始
- `Append` 在会超过 `max_size` 时永远不会成功
- `Compact()` 不改变 `Available()` 的数据，只改变其在内存中的位置

## 常见错误

- 忘记在 `Consume` 中更新已消费偏移量
- `Peek` 返回的视图包含了已消费的字节
- 没有检查缓冲区大小计算的溢出

## 测试

```bash
cmake --build build-debug --target byte_buffer_test
./build-debug/byte_buffer_test
```

## 评分

- 正确性：60%
- 边界情况（空缓冲区、最大大小、消费超过可用量）：20%
- 资源安全：10%
- 代码结构：10%
