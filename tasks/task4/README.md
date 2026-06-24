# Task 4: 列表和哈希命令

## 背景

Redis 在同一个键空间中支持多种数据类型。本任务添加列表（基于双端队列）和哈希（有序映射）类型。你将学习容器命令、索引处理和类型安全。

## 学习目标

- 多类型数据库设计
- 容器操作（push、pop、range）
- 负索引处理
- 空容器清理（删除最后一个元素时删除键）
- 确定性输出排序

## 你的任务

### 列表命令

- `LPUSH key element [element ...]` - 从头部压入，返回新长度
- `RPUSH key element [element ...]` - 从尾部压入，返回新长度
- `LPOP key` - 移除并返回头部元素（空时返回 nil）
- `RPOP key` - 移除并返回尾部元素（空时返回 nil）
- `LLEN key` - 返回长度（键不存在时返回 0）
- `LRANGE key start stop` - 返回范围内的元素

### 哈希命令

- `HSET key field value [field value ...]` - 设置字段，返回新增字段的数量
- `HGET key field` - 获取字段值（不存在时返回 nil）
- `HDEL key field [field ...]` - 删除字段，返回删除的数量
- `HLEN key` - 返回字段数量
- `HGETALL key` - 以排序顺序返回所有字段-值对

## 索引规则（LRANGE）

- 支持负索引：-1 = 最后一个元素，-2 = 倒数第二个
- 越界索引会被钳制到列表边界
- 钳制后如果 start > stop，返回空数组
- 键不存在时返回空数组

## HGETALL 排序

为了测试的确定性，`HGETALL` 必须按排序顺序返回字段。使用 `std::map` 或在输出前排序。

## 空容器清理

当 `LPOP`/`RPOP` 删除最后一个元素时，删除该键。
当 `HDEL` 删除最后一个字段时，删除该键。

## 需要修改的文件

- `src/command/list_commands.cpp`（必须）
- `src/command/hash_commands.cpp`（必须）
- `tests/integration/list_command_test.cpp`（必须）
- `tests/integration/hash_command_test.cpp`（必须）

## 常见错误

- 使用 `std::unordered_map` 存储哈希（HGETALL 输出不确定）
- 不删除空键
- HSET 返回值错误（应该是新增字段数，不是总数）
- LRANGE 负索引没有归一化

## 测试

```bash
cmake --build build-debug --target list_command_test hash_command_test
./build-debug/list_command_test
./build-debug/hash_command_test
```

## 评分

- 正确性：55%
- 索引处理（负索引、越界）：15%
- 类型安全：10%
- 空容器清理：10%
- 代码结构：5%
- 测试覆盖率：5%
