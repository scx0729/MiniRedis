# Task 6: 非阻塞 TCP 和 epoll 事件循环

## 背景

这是所有组件汇聚的地方：真实的网络 I/O。你将使用非阻塞 socket 和 epoll 实现一个 TCP 服务器，处理多个客户端、部分读写和优雅关闭。

## 学习目标

- Socket 生命周期（socket、bind、listen、accept、close）
- 非阻塞 I/O 及其重要性
- 使用 epoll 进行事件驱动 I/O
- 部分读和部分写
- 连接清理和文件描述符管理
- 信号处理实现优雅关闭

## 你的任务

### 1. 实现 Connection

`src/server/connection.cpp`：
- 从 socket 非阻塞读取到输入缓冲区
- 从输出缓冲区非阻塞写入到 socket
- 处理 EAGAIN、EINTR、连接关闭

### 2. 实现 EventLoop

`src/server/event_loop.cpp`：
- epoll_create、epoll_ctl、epoll_wait
- 处理 EPOLLIN、EPOLLOUT、EPOLLERR、EPOLLHUP
- 将事件分发到回调函数

### 3. 实现 Server

`src/server/server.cpp`：
- 创建监听 socket（非阻塞，SO_REUSEADDR）
- 接受连接
- 将 EventLoop 与 Session 连接起来
- 处理 SIGINT/SIGTERM 实现优雅关闭

### 4. 实现 main.cpp

将所有组件与配置加载连接起来。

## epoll 注意事项

- 使用水平触发模式（更简单、更正确）
- EPOLLIN 时始终排空输入缓冲区（读到 EAGAIN 为止）
- 仅在输出缓冲区有数据时注册 EPOLLOUT
- 输出缓冲区为空时取消 EPOLLOUT 注册

## 需要修改的文件

- `src/server/connection.cpp`（必须）
- `src/server/event_loop.cpp`（必须）
- `src/server/server.cpp`（必须）
- `src/main.cpp`（必须）

## 常见错误

- 使用阻塞 I/O（违背了 epoll 的初衷）
- 没有处理部分写（数据丢失）
- 出错时没有关闭连接
- 连接销毁后 epoll 仍引用它（use-after-free）
- 没有忽略 SIGPIPE

## 测试

实现完成后，用以下命令测试：
```bash
./build-debug/miniredis-server --config config/miniredis.conf &
redis-cli -p 6380 PING
redis-cli -p 6380 SET key value
redis-cli -p 6380 GET key
kill %1
```

## 评分

- 正确性：45%
- 非阻塞 I/O：20%
- 连接清理：15%
- 优雅关闭：10%
- 代码结构：10%
