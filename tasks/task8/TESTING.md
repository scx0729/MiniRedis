# Task 8 Testing: AOF 持久化与恢复

## 配置

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMINIREDIS_TASK_LEVEL=8
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## 当前任务公开测试

- `aof_test`
- `recovery_test`
- `blackbox_tcp_test`

## 建议流程

1. 先只运行当前任务的单个测试目标。
2. 修一个断言，跑一次对应测试。
3. 当前任务通过后，运行 `ctest --test-dir build --output-on-failure` 确认历史任务没有回归。
4. 网络和持久化任务需要关注超时、进程退出和临时文件清理。
