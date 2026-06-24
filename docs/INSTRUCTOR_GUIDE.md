# MiniRedis17 教师指南

## 概述

本指南帮助教师搭建、部署和评分 MiniRedis17 课程项目。

## 生成学生版本

使用提供的工具生成学生仓库：

```bash
python3 tools/generate_student_repo.py --task 3 --output student-task3/
```

生成的版本中：
- Task 0-2：代码保留（学生已完成）
- Task 3：代码替换为桩代码 + TODO 注释
- Task 4-8：代码完全删除

## 恢复完整解答

参考实现位于主仓库中。恢复方法：
```bash
git checkout master
```

## 评分

每个任务的评分标准在 `tasks/taskN/RUBRIC.md` 中。通用权重：
- 正确性：50-60%
- 边界情况：15-20%
- 资源安全：10%
- 代码结构：10%
- 测试覆盖率：5%

## 隐藏测试

参见 `tests/hidden_spec/README.md` 了解隐藏测试规格。这些测试覆盖了公开测试未涉及的边界情况。

## 运行所有检查

```bash
# Debug 构建 + 测试
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j
ctest --test-dir build-debug --output-on-failure

# Release 构建
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j

# Sanitizer 构建
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON -DMINIREDIS_ENABLE_UBSAN=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

## 学生常见错误

1. **解析器**：不是真正的增量（每次都从头重新解析）
2. **ByteBuffer**：Peek() 返回了已消费的数据
3. **INCR**：没有检查 int64_t 溢出
4. **HGETALL**：使用了 unordered_map（顺序不确定）
5. **TTL**：测试中使用了真实时间（不确定性的）
6. **epoll**：使用了阻塞 I/O
7. **AOF**：没有处理截断的尾部

## 任务依赖

Task N 依赖于所有 0..N-1 任务的完成。每个任务的起始版本应该能编译并通过所有之前任务的测试。

## 配置

默认配置：`config/miniredis.conf`。学生可以修改用于测试。关键设置：
- `port 6380`（与默认 Redis 6379 不同）
- `appendonly yes|no`
- `appendfsync always|no`


## 当前本机参考实现副本

本轮转换前的完整参考实现已复制到：

```text
/home/scx/dev/miniredis17-reference-20260624/
```

当前仓库则是学生 starter，源码中保留 TODO 桩。
