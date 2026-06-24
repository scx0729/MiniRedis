# Task 7: TTL 和过期系统

## 背景

Redis 的键可以设置生存时间（TTL）。过期的键应对读操作不可见，并最终被清理。这需要一个可测试的时钟抽象、惰性过期（访问时检查）和主动过期（周期性扫描）。

## 学习目标

- 数据库中的时间语义
- 惰性过期 vs 主动过期策略
- 可注入的时钟用于确定性测试
- 过期索引设计（最小堆）
- 处理过期堆中的过期条目

## 你的任务

### 1. 实现时钟抽象

`include/miniredis/base/clock.h` 中应有用于测试的 `FakeClock`。

### 2. 实现过期管理器

`src/storage/expiration.cpp`：
- (过期时间戳, 键) 的最小堆
- 添加/移除键的跟踪
- ActiveExpire：从堆中扫描过期键

### 3. 实现 TTL 命令

`src/command/expire_commands.cpp`：
- `EXPIRE key seconds` / `PEXPIRE key milliseconds`
- `TTL key` / `PTTL key`（返回剩余时间）
- `PERSIST key`（移除过期时间）

### 4. 与数据库集成

- 惰性过期：访问时检查，过期则删除
- SET 带 EX/PX 选项
- SET 清除已有的 TTL
- INCR/APPEND 等保留已有的 TTL

## TTL 规则

- `TTL` 返回剩余秒数的向下取整
- `PTTL` 返回毫秒数
- 键存在但没有过期时间时返回 -1
- 键不存在时返回 -2
- EXPIRE 秒数 <= 0 时立即删除键

## 过期堆中的过期条目

当键的 TTL 更新时，旧条目仍留在堆中。主动过期在删除前必须检查该键是否仍有相同的过期时间。

## 需要修改的文件

- `src/storage/expiration.cpp`（必须）
- `src/storage/database.cpp`（添加 TTL 集成）
- `src/command/expire_commands.cpp`（必须）
- `tests/unit/expiration_test.cpp`（必须）
- `tests/integration/ttl_command_test.cpp`（必须）

## 测试

测试中不要使用 `sleep()`。使用 `FakeClock` 确定性地推进时间。

```bash
cmake --build build-debug --target expiration_test ttl_command_test
./build-debug/expiration_test
./build-debug/ttl_command_test
```

## 评分

- 正确性：50%
- 惰性 + 主动过期：20%
- 确定性测试（无 sleep）：15%
- 过期条目处理：10%
- 代码结构：5%
