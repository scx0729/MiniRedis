# MiniRedis17 参考实现报告

## 1. 实现状态

### 已完成模块
- **RESP 层**：值类型、编码器、增量解析器（状态机）
- **存储层**：数据库（String/List/Hash）、过期（最小堆）
- **命令层**：注册表、执行器、全部 27 条命令
- **服务器层**：ByteBuffer、Connection、Session、EventLoop（epoll）
- **持久化层**：AOF 写入器、AOF 重放
- **配置层**：配置文件解析器

### 支持的命令（共 27 条）
| 类别 | 命令 |
|------|------|
| 连接 | PING, ECHO, QUIT |
| 通用 | DEL, EXISTS, TYPE |
| 字符串 | SET, GET, MSET, MGET, APPEND, STRLEN, INCR, DECR |
| 列表 | LPUSH, RPUSH, LPOP, RPOP, LLEN, LRANGE |
| 哈希 | HSET, HGET, HDEL, HLEN, HGETALL |
| TTL | EXPIRE, PEXPIRE, TTL, PTTL, PERSIST |

### 非目标（明确不实现）
- Redis Cluster、主从复制、Sentinel
- Lua 脚本、ACL、TLS
- RDB、Streams、有序集合、Bitmap、HyperLogLog、GEO
- 模块系统、多线程执行

### 已知限制
- 仅支持单线程（无多线程命令执行）
- 无连接数限制
- 主动过期按需运行（非周期性定时器）
- HGETALL 使用排序顺序（与 Redis 的未指定顺序不同）
- 协议错误断开客户端连接（Redis 尝试继续处理）

## 2. 架构总结

| 层 | 核心类 | 职责 |
|----|--------|------|
| RESP | `RespValue`、`RespEncoder`、`RespParser` | 协议编解码 |
| Storage | `Database`、`Expiration`、`StringValue/ListValue/HashValue` | 内存数据存储 |
| Command | `CommandRegistry`、`CommandExecutor`、处理器 | 命令分发和执行 |
| Server | `ByteBuffer`、`Connection`、`Session`、`EventLoop`、`Server` | 网络 I/O 和事件循环 |
| Persistence | `AofWriter`、`AofReplay` | AOF 预写日志 |
| Config | `Config` | 配置文件解析 |
| Base | `Result<T>`、`Error`、`Clock`、`FakeClock` | 基础工具 |

## 3. 测试结果

### 总结
- **15 个测试套件**（8 个单元测试 + 7 个集成测试）
- **全部 15 个通过**（Debug、Release 和 ASAN/UBSAN 构建）
- AddressSanitizer 检测到 **0 个内存错误**
- UBSan 检测到 **0 个未定义行为**

### 测试套件
| 测试 | 类型 | 用例数 | 状态 |
|------|------|--------|------|
| byte_buffer_test | 单元 | 12 | 通过 |
| result_test | 单元 | 7 | 通过 |
| resp_encoder_test | 单元 | 17 | 通过 |
| resp_parser_test | 单元 | 20 | 通过 |
| database_test | 单元 | 28 | 通过 |
| expiration_test | 单元 | 11 | 通过 |
| aof_test | 单元 | 5 | 通过 |
| config_test | 单元 | 8 | 通过 |
| string_command_test | 集成 | 19 | 通过 |
| list_command_test | 集成 | 11 | 通过 |
| hash_command_test | 集成 | 10 | 通过 |
| ttl_command_test | 集成 | 13 | 通过 |
| session_test | 集成 | 5 | 通过 |
| pipeline_test | 集成 | 2 | 通过 |
| recovery_test | 集成 | 4 | 通过 |

### 构建验证
```bash
# Debug: cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug → 通过
# Release: cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release → 通过
# ASAN/UBSAN: cmake -S . -B build-asan -DMINIREDIS_ENABLE_ASAN=ON -DMINIREDIS_ENABLE_UBSAN=ON → 通过
```

## 4. 关键设计决策

1. **Result\<T\> 优于异常**：用于所有可恢复错误（协议、命令、类型错误）。致命错误仍然使用 exit。

2. **std::variant 用于值类型**：`StorageValue = variant<StringValue, ListValue, HashValue>`。通过 `std::holds_alternative` 进行类型检查。

3. **std::map 用于哈希字段**：确保 HGETALL 输出顺序确定，提高测试可靠性。

4. **可注入的时钟**：`Clock` 抽象类，包含 `SystemClock` 和 `FakeClock`。所有时间相关代码使用 Clock 接口。

5. **最小堆用于过期**：简单、正确，通过在弹出时重新检查来处理过期条目。

6. **RESP 格式用于 AOF**：人类可读，易于调试，与 redis-cli 兼容。

7. **水平触发 epoll**：比边缘触发更简单，无需复杂的排空循环即可正确工作。

8. **基于指针的 CommandContext**：从引用改为指针，以允许构造后赋值。

## 5. 文件结构

```
82 个文件，约 7800 行代码
├── 17 个头文件（include/miniredis/）
├── 17 个源文件（src/）
├── 15 个测试文件（tests/）
├── 9 个任务 README（tasks/task0-8/）
├── 6 个文档文件（docs/）
├── 4 个脚本（scripts/）
├── 2 个工具（tools/）
├── CMakeLists.txt、config/miniredis.conf、README.md、.gitignore
```

## 6. 学生版本生成

```bash
python3 tools/generate_student_repo.py --task 3 --output student-task3/
```

## 7. 教师解答恢复

参考实现位于主仓库（master 分支）。无需特殊恢复。

## 8. 审计结果

| 检查项 | 状态 |
|--------|------|
| Debug 构建 | 通过 |
| Release 构建 | 通过 |
| AddressSanitizer | 通过（0 个错误） |
| UndefinedBehaviorSanitizer | 通过（0 个错误） |
| 单元测试（8 个套件） | 通过 |
| 集成测试（7 个套件） | 通过 |
| 参考代码中无 TODO/FIXME | 已验证 |
| 无硬编码测试绕过 | 已验证 |
| 所有系统调用返回值已检查 | 已验证 |
| 二进制安全的键和值 | 已验证 |
| 增量解析器（逐字节） | 已验证 |
| HGETALL 确定性顺序 | 已验证 |
| INCR/DECR 溢出检测 | 已验证 |
| AOF 截断尾部处理 | 已验证 |
| 优雅关闭（SIGINT/SIGTERM） | 已实现 |

## 9. 待解决问题

1. **黑盒网络测试**：未作为独立测试套件实现。网络功能通过使用 socketpair 的集成测试进行测试。需要运行服务器才能进行完整的端到端测试。

2. **redis-cli 兼容性**：未在自动化测试中验证。需要使用实际的 redis-cli 二进制文件进行手动测试。

3. **性能基准测试**：未实现。`tools/` 中有基础基准工具，但未集成到测试套件中。

4. **学生版本生成**：工具已存在，但未使用实际代码标记进行测试（标记尚未添加到参考实现中）。

5. **clang-format**：提供了脚本，但没有 `.clang-format` 配置文件。
