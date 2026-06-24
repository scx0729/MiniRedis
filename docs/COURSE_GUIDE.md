# MiniRedis17 课程指南

## 概述

MiniRedis17 是一个实践项目，你将用 C++17 构建一个 Redis 风格的内存数据库服务器。项目分为 9 个任务（Task 0-8），每个任务建立在前一个的基础上。

## 前置知识

- C++17（类、模板、智能指针、std::variant、std::optional）
- 基础网络知识（TCP、socket）
- 基础数据结构（哈希表、双端队列、堆）
- Linux 命令行

## 快速开始

1. 克隆仓库
2. 构建：`cmake -S . -B build && cmake --build build -j`
3. 运行测试：`ctest --test-dir build --output-on-failure`
4. 从 `tasks/task0/README.md` 开始

## 任务依赖关系

```
Task 0 (ByteBuffer, Result)
  └── Task 1 (RESP 编码器)
        └── Task 2 (RESP 解析器)
              └── Task 3 (数据库, 字符串命令)
                    └── Task 4 (列表, 哈希命令)
                          └── Task 5 (会话, 管线化)
                                └── Task 6 (TCP, epoll)
                                      └── Task 7 (TTL, 过期)
                                            └── Task 8 (AOF, 恢复)
```

## 如何完成每个任务

1. 仔细阅读任务 README
2. 查看头文件了解接口
3. 运行现有测试，查看哪些失败
4. 增量式实现
5. 频繁运行测试
6. 检查边界情况

## 测试代码

```bash
# 运行所有测试
ctest --test-dir build --output-on-failure

# 运行特定测试
./build/byte_buffer_test

# 使用 Sanitizer 运行
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

## 用 redis-cli 测试

服务器运行后（Task 6），可以用 redis-cli 测试：
```bash
./build/miniredis-server --config config/miniredis.conf &
redis-cli -p 6380 PING
redis-cli -p 6380 SET hello world
redis-cli -p 6380 GET hello
```

## 获取帮助

- 阅读任务 README
- 查看 `docs/ARCHITECTURE.md` 了解系统概览
- 查看 `docs/PROTOCOL.md` 了解 RESP 细节
- 查看 `docs/COMMANDS.md` 了解命令规格
- 查看测试文件了解预期行为


## 分阶段测试开关

学生 starter 默认 `MINIREDIS_TASK_LEVEL=0`。每完成一个任务后，用 `-DMINIREDIS_TASK_LEVEL=N` 重新配置 CMake 来打开从 Task 0 到 Task N 的公开测试。不要直接运行未来任务测试来判断当前任务是否完成。
