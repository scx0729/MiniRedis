# Task 3: 数据库和字符串命令

## 背景

现在我们能解析 RESP 和编码响应了，接下来需要一个存储引擎和命令。数据库将键（二进制安全的字符串）映射到带类型的值。本任务介绍类型系统和字符串命令。

## 学习目标

- 协议层和存储层的分离
- 使用 std::variant 进行类型安全的值存储
- 命令参数校验
- 整数溢出检测
- "键不存在"和"类型错误"的区别

## 你的任务

### 1. 实现数据库

在 `src/storage/database.cpp` 中实现 `Database` 类：
- 字符串值存储
- Get/Set/Del/Exists/Type 操作
- 类型检查（对错误类型的键使用命令时返回 WrongType 错误）

### 2. 实现字符串命令

在 `src/command/string_commands.cpp` 中实现命令：
- `SET key value [EX seconds] [PX milliseconds] [NX|XX]`
- `GET key`
- `MSET key value [key value ...]`
- `MGET key [key ...]`
- `APPEND key value`
- `STRLEN key`
- `INCR key` / `DECR key`

### 3. 实现通用命令

- `DEL key [key ...]`
- `EXISTS key [key ...]`
- `TYPE key`

### 4. 编写测试

## SET 选项

- `EX seconds` - 设置过期时间（Task 7 详细讲解，但现在解析应该能工作）
- `PX milliseconds` - 以毫秒设置过期时间
- `NX` - 仅在键不存在时设置
- `XX` - 仅在键存在时设置
- 如果同时指定 NX 和 XX，后者生效

## INCR/DECR 规则

- 如果键不存在，先当作 0 再递增
- 如果值不是有效的整数，返回错误
- 检查 int64_t 溢出

## 需要修改的文件

- `src/storage/database.cpp`（必须）
- `src/command/string_commands.cpp`（必须）
- `tests/unit/database_test.cpp`（必须）
- `tests/integration/string_command_test.cpp`（必须）

## 常见错误

- 操作前不检查类型（对 String 键执行 LLEN 应该报错）
- 使用 `atoi` 解析整数（不能检测错误）
- INCR/DECR 没有处理 int64_t 溢出
- SET NX 在键已存在时仍返回 OK

## 测试

```bash
cmake --build build-debug --target database_test string_command_test
./build-debug/database_test
./build-debug/string_command_test
```

## 评分

- 正确性：55%
- 类型安全（WrongType 错误）：15%
- 边界情况（空值、二进制数据、溢出）：15%
- 代码结构：10%
- 测试覆盖率：5%
