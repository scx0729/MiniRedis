# MiniRedis17 实现计划

## 项目概述

MiniRedis17 是一个面向教学的 Redis 风格内存数据库服务器，使用 C++17 编写。设计为类似 MIT/CMU 系统课程的课程项目，学生逐步实现各个组件。

## 范围

### 范围内
- RESP2 协议（编码和解析）
- 二进制安全的键和值
- 使用非阻塞 socket 的 TCP 长连接
- epoll 事件循环（单线程）
- 增量协议解析
- 管线化支持（每个连接多个命令）
- 部分读写处理
- String、List 和 Hash 类型
- 键过期（惰性 + 主动）
- AOF 持久化和恢复
- 配置加载
- 日志、错误处理、优雅关闭
- 单元测试、集成测试和黑盒测试
- 与 `redis-cli` 的兼容性（支持的命令）

### 范围外
- Redis Cluster、主从复制、Sentinel
- Lua 脚本、ACL、TLS
- RDB、Streams、有序集合、Bitmap、HyperLogLog、GEO
- 模块系统
- 完整的 Redis 配置语法
- 多线程命令执行
- 分布式共识
- 生产级安全

## 架构层次

```
┌─────────────────────────────────────────┐
│              main.cpp                    │
├─────────────────────────────────────────┤
│  Config  │  Server  │  EventLoop        │
├─────────────────────────────────────────┤
│  Connection  │  Session                 │
├─────────────────────────────────────────┤
│  RESP Parser  │  RESP Encoder           │
├─────────────────────────────────────────┤
│  Command Registry  │  Command Executor   │
├─────────────────────────────────────────┤
│  Database  │  Expiration                │
├─────────────────────────────────────────┤
│  AOF Writer  │  AOF Replay              │
├─────────────────────────────────────────┤
│  ByteBuffer  │  Result<T>  │  Clock     │
└─────────────────────────────────────────┘
```

## 实现阶段

### 阶段 1：基础（Task 0）
- 错误模型（Result<T>）
- ByteBuffer
- CMake 构建系统
- 测试框架集成

### 阶段 2：RESP 协议（Task 1-2）
- RESP 值类型
- RESP 编码器
- 增量 RESP 解析器（状态机）

### 阶段 3：存储和命令（Task 3-4）
- 带类型系统的数据库
- 字符串命令（SET、GET、MSET、MGET、APPEND、STRLEN、INCR、DECR）
- 列表命令（LPUSH、RPUSH、LPOP、RPOP、LLEN、LRANGE）
- 哈希命令（HSET、HGET、HDEL、HLEN、HGETALL）
- 命令注册表和执行器

### 阶段 4：网络层（Task 5-6）
- 会话（命令管线化）
- 连接管理
- epoll 事件循环
- 带优雅关闭的 TCP 服务器

### 阶段 5：过期（Task 7）
- 时钟抽象与 FakeClock
- 惰性过期
- 主动过期（周期性扫描）
- EXPIRE、PEXPIRE、TTL、PTTL、PERSIST 命令

### 阶段 6：持久化（Task 8）
- AOF 写入器（RESP 格式）
- 启动时 AOF 重放
- 截断尾部处理
- 配置（appendonly、appendfsync）

## 数据流

```
客户端 TCP 字节
  → 输入缓冲区（ByteBuffer）
  → RESP 解析器（增量状态机）
  → 请求 = Array<Optional<BulkString>>
  → 命令注册表查找（大小写不敏感）
  → 命令执行器校验参数
  → 数据库读写
  → RESP 响应编码
  → 输出缓冲区（ByteBuffer）
  → TCP 发送（支持部分写入）
```

## 线程模型

单线程事件循环。所有命令执行、解析和 I/O 都在一个线程上进行。数据库访问不需要互斥锁。

## 错误处理策略

- `Result<T>` 类型用于可恢复错误（解析、命令执行）
- 系统调用返回值始终检查
- 致命错误记录日志并干净退出
- 协议错误断开客户端连接
- 命令错误返回 RESP Error 给客户端

## 测试策略

1. **单元测试**：ByteBuffer、RESP 编码器、RESP 解析器、数据库、过期、AOF、配置
2. **集成测试**：解析器+会话、会话+命令、命令+数据库、TTL、AOF 重放
3. **黑盒测试**：通过 TCP 连接真实服务器（单命令、管线化、部分写入、多客户端、重启、AOF、优雅关闭）
4. **隐藏测试**：边界情况记录在 `tests/hidden_spec/` 中，供教师使用

## 构建系统

CMake 3.16+，无网络依赖。支持：
- Debug / Release 构建
- ASAN / UBSAN
- CTest 集成
- 单命令构建和测试

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## 学生版本生成

使用参考实现中的代码区域标记：
```cpp
// BEGIN STUDENT TODO(task=N)
// ... 参考实现 ...
// END STUDENT TODO(task=N)
```

Python 工具（`tools/generate_student_repo.py`）会删除未来任务的实现并替换为可编译的桩代码。
