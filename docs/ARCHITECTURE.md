# MiniRedis17 架构设计

## 模块概览

### 1. 基础层（`base/`）

**Result\<T\>**：用于错误处理的可区分联合体。要么持有 T 类型的值，要么持有 Error。在整个代码库中用于替代异常来处理可预期的错误。

**Error**：错误码枚举 + 可选的消息字符串。类别包括：协议错误、命令错误、类型错误、IO 错误、配置错误。

**Clock**：抽象时钟接口。生产代码使用系统时钟；测试使用 FakeClock 实现确定性的 TTL 测试。

### 2. RESP 层（`resp/`）

**Value**：表示一个 RESP2 值。是以下类型的 variant：
- SimpleString（字符串）
- Error（字符串）
- Integer（int64_t）
- BulkString（字符串，二进制安全，可为 null）
- Array（optional\<Value\> 的 vector，可为 null）

**Encoder**：将 Value 转换为 RESP 线路格式的字节序列。

**Parser**：增量状态机，消费字节并产出 Value。跨调用追踪状态（处理不完整输入）。返回 ParseResult，包含状态（Complete/NeedMoreData/ProtocolError）、已消费字节数和解析出的值。

### 3. 存储层（`storage/`）

**值类型**：StringValue（string）、ListValue（deque\<string\>）、HashValue（有序 map\<string,string\>）。

**Database**：使用 unordered_map\<string, Entry\> 实现的键值存储。每个 Entry 持有一个 variant 值 + 可选的过期时间。提供带类型的访问器。操作前检查类型（类型错误时返回 WrongType 错误）。

**Expiration**：使用（过期时间, 键）的最小堆管理键过期。支持：
- 惰性检查：访问键时检查是否过期
- 主动扫描：周期性扫描堆，删除过期键

### 4. 命令层（`command/`）

**Command**：接口，定义命令名称、参数数量、标志（写/只读）和执行函数。

**Registry**：将命令名称（大小写不敏感）映射到 Command 对象。静态注册。

**Executor**：校验参数数量，分发到命令处理器，返回 RESP Value 响应。

### 5. 服务器层（`server/`）

**ByteBuffer**：可增长的字节缓冲区，支持追加/窥探/消费操作。追踪已消费与未消费区域。有可配置的最大大小。

**Connection**：持有一个 socket fd。拥有输入 ByteBuffer 和输出 ByteBuffer。处理非阻塞读写，追踪部分写入。

**Session**：拥有 Connection + Parser + Database/Registry 的引用。循环：从输入缓冲区解析命令 → 执行 → 将响应编码到输出缓冲区。支持管线化（每次读取处理多个命令）。

**EventLoop**：epoll 封装器。管理监听 socket 和连接 socket。分发读/写/错误/关闭事件。

**Server**：顶层组件。绑定监听 socket，创建 EventLoop，接受连接，处理信号以实现优雅关闭。

### 6. 持久化层（`persistence/`）

**AofWriter**：在命令成功执行后，将 RESP 编码的写命令追加到 AOF 文件。可配置 fsync 策略（始终/从不）。

**AofReplay**：启动时读取 AOF 文件，解析 RESP 命令，对 Database 重新执行。优雅地处理截断的尾部数据。

### 7. 配置层（`config/`）

**Config**：解析配置文件（键=值格式）。提供带默认值的类型化访问器。启动时校验。

## 数据所有权

```
Server
  ├── Config
  ├── EventLoop
  │     └── epoll_fd
  ├── Database (unique_ptr)
  ├── Expiration (unique_ptr)
  ├── CommandRegistry (unique_ptr)
  ├── AofWriter (unique_ptr, 可选)
  └── Connections (map<int, unique_ptr<Connection>>)
        └── Session (每个连接一个)
              ├── Parser
              ├── 输入 ByteBuffer
              └── 输出 ByteBuffer
```

所有所有权均通过 unique_ptr 管理。没有裸的拥有指针。Session 通过引用借用 Database 和 Registry（生命周期由 Server 保证）。

## 请求处理流程

1. epoll 报告连接 fd 上的 EPOLLIN 事件
2. Connection::Read() 将字节追加到输入 ByteBuffer
3. Session::ProcessInput() 循环：
   a. Parser::Parse() 对未消费的输入进行解析
   b. 如果完整：从 Array 中提取命令名 + 参数
   c. Registry::Find(command_name)
   d. Executor::Run(command, args, database)
   e. Encoder::Encode(response) → 追加到输出 ByteBuffer
   f. 从输入中消费已解析的字节
   g. 如果还有完整命令则重复
4. 如果输出 ByteBuffer 有数据，注册 EPOLLOUT
5. epoll 报告 EPOLLOUT → Connection::Write() 从输出 ByteBuffer 发送数据
6. 所有输出发送完毕后，取消 EPOLLOUT 注册

## 优雅关闭

1. SIGINT/SIGTERM 设置原子标志
2. EventLoop 检查标志，停止接受新连接
3. 现有连接完成待处理的 I/O
4. AOF 刷盘
5. 关闭所有连接
6. 以退出码 0 退出

## 限制常量

| 参数 | 默认值 | 配置键 |
|------|--------|--------|
| 客户端输入缓冲区上限 | 64 MB | max-client-input-buffer |
| 客户端输出缓冲区上限 | 64 MB | max-client-output-buffer |
| 最大批量字符串长度 | 512 MB | max-bulk-length |
| 最大数组元素数 | 10000 | （硬编码） |
| 最大嵌套深度 | 100 | （硬编码） |
| 主动过期间隔 | 100 ms | active-expire-interval-ms |
| 主动过期采样数 | 20 | （硬编码） |
