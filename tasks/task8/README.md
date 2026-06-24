# Task 8: AOF 持久化和恢复

## 背景

没有持久化，服务器重启时所有数据都会丢失。追加式文件（AOF）记录每一条写命令。启动时，服务器重放 AOF 来恢复状态。

## 学习目标

- 预写日志（WAL）概念
- 以 RESP 格式记录命令
- 启动时重放
- 处理截断/损坏的尾部数据
- I/O 错误处理
- 持久化配置

## 你的任务

### 1. 实现 AOF 写入器

`src/persistence/aof.cpp`：
- 以追加模式打开文件
- 写入 RESP 编码的命令
- fsync 策略（始终同步或不同步）
- 处理 I/O 错误

### 2. 实现 AOF 重放

`src/persistence/aof_replay.cpp`：
- 启动时读取 AOF 文件
- 解析并执行命令
- 优雅处理截断的尾部（停止，不崩溃）
- 不要将重放的命令再次写入 AOF

### 3. 与命令集成

写命令成功执行后，记录到 AOF：
- 只记录写命令（SET、DEL、LPUSH 等）
- 不记录读命令（GET、LLEN 等）
- 在成功执行**之后**记录

### 4. 添加配置

- `appendonly yes|no`
- `appendfsync always|no`

## AOF 格式

使用 RESP 格式作为 AOF 文件。每条命令是一个 RESP 数组：
```
*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
```

## 截断尾部处理

如果 AOF 文件以不完整的命令结尾（例如服务器在写入过程中崩溃），重放应该：
1. 成功重放所有完整命令
2. 在第一个解析错误处停止
3. 不崩溃

## 需要修改的文件

- `src/persistence/aof.cpp`（必须）
- `src/persistence/aof_replay.cpp`（必须）
- `src/config/config.cpp`（添加 AOF 配置解析）
- `tests/unit/aof_test.cpp`（必须）
- `tests/integration/recovery_test.cpp`（必须）

## 测试

通过实际的服务器重启来测试：
```bash
# 启动服务器，写入数据，杀死，重启，验证数据
./build-debug/miniredis-server &
redis-cli -p 6380 SET key value
kill $(pgrep miniredis-server)
./build-debug/miniredis-server &
redis-cli -p 6380 GET key  # 应该返回 "value"
kill $(pgrep miniredis-server)
```

```bash
cmake --build build-debug --target aof_test recovery_test
./build-debug/aof_test
./build-debug/recovery_test
```

## 评分

- 正确性：45%
- 截断尾部处理：20%
- I/O 错误处理：15%
- 配置：10%
- 代码结构：5%
- 测试覆盖率：5%
