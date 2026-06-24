# MiniRedis17

一个面向教学的 Redis 风格内存数据库服务器，使用 C++17 编写，设计为类似 MIT/CMU 系统课程的课程项目。


## 当前仓库状态

当前路径是学生 starter 版本：实现文件中保留完整函数签名，并用 `TODO(Task N)` 标出学生需要填写的函数体。完整参考实现已复制到本机项目外：

```text
/home/scx/dev/miniredis17-reference-20260624/
```

默认只打开 Task 0 公开测试。推进到第 N 个任务时重新配置：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMINIREDIS_TASK_LEVEL=N
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## 学习目标

完成本项目后，学生将理解：

- C++17 项目结构与 CMake 构建系统
- RAII 与资源所有权模式
- 网络协议设计与增量解析
- 非阻塞 I/O 与 epoll 事件循环
- 单线程服务器架构
- 带类型系统的内存数据库设计
- 键过期机制（惰性 + 主动）
- 仅追加文件（AOF）持久化
- 测试策略（单元测试、集成测试、黑盒测试）

## 架构概览

```
客户端 TCP → 输入缓冲区 → RESP 解析器 → 命令注册表 → 执行器 → 数据库
                                                                      ↓
客户端 TCP ← 输出缓冲区 ← RESP 编码器 ← 响应 ← ← ← ← ← ← ← ← ← ←┘
```

模块说明：
- **RESP**：协议值类型、编码器、增量解析器（状态机）
- **Storage**：支持 String/List/Hash 类型的数据库，带最小堆的过期管理
- **Command**：注册表（大小写不敏感查找）、执行器（参数数量校验）
- **Server**：ByteBuffer、Connection（非阻塞）、Session（管线化）、EventLoop（epoll）
- **Persistence**：AOF 写入器（RESP 格式）、启动时 AOF 重放
- **Config**：键=值 配置文件解析器

## 支持的命令

| 类别 | 命令 |
|------|------|
| 连接 | PING, ECHO, QUIT |
| 通用 | DEL, EXISTS, TYPE |
| 字符串 | SET, GET, MSET, MGET, APPEND, STRLEN, INCR, DECR |
| 列表 | LPUSH, RPUSH, LPOP, RPOP, LLEN, LRANGE |
| 哈希 | HSET, HGET, HDEL, HLEN, HGETALL |
| TTL | EXPIRE, PEXPIRE, TTL, PTTL, PERSIST |

## 不支持的功能

Redis Cluster、主从复制、Sentinel、Lua 脚本、ACL、TLS、RDB、Streams、有序集合、Bitmap、HyperLogLog、GEO、模块、多线程执行、分布式共识。

## 环境要求

- Linux（需要 epoll）
- CMake 3.16+
- C++17 编译器（GCC 9+ 或 Clang 10+）
- 无外部依赖

## 构建

```bash
# Debug 构建
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# Release 构建
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j

# 开启 Sanitizer 构建
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON \
  -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j
```

## 运行测试

```bash
ctest --test-dir build --output-on-failure
```

## 启动服务器

```bash
./build/miniredis-server --config config/miniredis.conf
```

## 用 redis-cli 测试

```bash
redis-cli -p 6380 PING
redis-cli -p 6380 SET key value
redis-cli -p 6380 GET key
redis-cli -p 6380 LPUSH list a b c
redis-cli -p 6380 LRANGE list 0 -1
redis-cli -p 6380 HSET user name alice age 20
redis-cli -p 6380 HGETALL user
redis-cli -p 6380 EXPIRE key 10
redis-cli -p 6380 TTL key
```

## 任务顺序

按顺序完成任务：Task 0 → Task 1 → ... → Task 8

| 任务 | 主题 |
|------|------|
| 0 | 错误模型、ByteBuffer |
| 1 | RESP 值、编码器 |
| 2 | 增量 RESP 解析器 |
| 3 | 数据库、字符串命令 |
| 4 | 列表、哈希命令 |
| 5 | 会话、管线化 |
| 6 | TCP、epoll 事件循环 |
| 7 | TTL、过期机制 |
| 8 | AOF 持久化 |

## 目录结构

```
miniredis17/
├── CMakeLists.txt
├── config/miniredis.conf
├── docs/                    # 架构、协议、命令文档
├── include/miniredis/       # 头文件
├── src/                     # 实现源码
├── tests/
│   ├── unit/                # 单元测试
│   ├── integration/         # 集成测试
│   ├── blackbox/            # 黑盒网络测试
│   └── hidden_spec/         # 隐藏测试规格（教师用）
├── tasks/task0..task8/      # 学生任务说明
├── tools/                   # 工具脚本
└── scripts/                 # 构建/测试脚本
```

## 学术诚信

本项目用于学习。学生应：
- 根据任务说明自行实现代码
- 不抄袭其他学生代码
- 在完成自己的实现之前不要查看参考实现
- 引用任何使用的外部资源

## 许可证

仅限教学使用。
