 当前仓库是 MiniRedis17 学生 starter 版本，完整参考实现保存在：
  /home/scx/dev/miniredis17-reference-20260624/

  请在现有 Task 0-8 的基础上继续扩展课程项目，新增 3 个任务：

  Task 9: RDB Snapshot 持久化
  Task 10: 主从复制
  Task 11: 发布订阅 Pub/Sub

  目标要求：

  1. 保持当前仓库为学生 TODO starter 风格：
     - 函数签名完整
     - 形参名不能省略
     - TODO 桩能编译
     - 当前任务公开测试 initially fail
     - 前置任务测试保持可运行
     - 使用 MINIREDIS_TASK_LEVEL=N 分阶段启用测试

  2. 同步更新完整参考实现：
     - 参考实现仍放在项目外，或重新生成一份新的参考实现副本
     - 参考实现必须通过所有 Task 0-11 测试
     - 当前学生仓库只保留 TODO 桩，不泄露完整答案

  3. 新增 Task 9: RDB Snapshot
     功能范围：
     - 支持 SAVE 命令
     - 支持配置 dbfilename
     - 支持启动时加载 RDB
     - RDB 可以是教学用自定义二进制格式，不要求兼容 Redis 官方 RDB
     - 需要保存 String/List/Hash 类型
     - 需要保存 TTL/expire_at_ms
     - 需要校验 magic/version/checksum 或长度，能拒绝损坏文件
     - 测试覆盖：
       - 写入 RDB 文件
       - 重启后恢复 String/List/Hash
       - TTL 恢复
       - 损坏 RDB 不崩溃

  4. 新增 Task 10: 主从复制
     功能范围：
     - 单 master，多 replica
     - replica 通过配置 replicaof host port 启动
     - replica 连接 master 后执行简单握手
     - master 将写命令传播给 replica
     - 初始同步可以使用 RDB snapshot 或教学用全量 dump
     - replica 默认只读，写命令返回错误
     - 断线重连可以做基础支持，不要求完整 PSYNC backlog
     - 测试覆盖：
       - replica 启动后同步 master 已有数据
       - master SET/DEL/EXPIRE 后 replica 可见
       - replica 拒绝写命令
       - master 多 replica 广播

  5. 新增 Task 11: Pub/Sub
     功能范围：
     - 支持 SUBSCRIBE channel [channel ...]
     - 支持 UNSUBSCRIBE [channel ...]
     - 支持 PUBLISH channel message
     - 订阅连接进入 Pub/Sub 模式
     - PUBLISH 返回收到消息的订阅者数量
     - 单线程事件循环内管理 channel -> sessions
     - 不要求 pattern subscribe
     - 不要求持久化 Pub/Sub 消息
     - 测试覆盖：
       - 单订阅者收到消息
       - 多订阅者收到消息
       - unsubscribe 后不再收到消息
       - publish 返回订阅者数量
       - 非订阅命令在 Pub/Sub 模式下的限制

  6. CMake 要求：
     - 将 MINIREDIS_TASK_LEVEL 最大值从 8 扩展到 11
     - Task 9/10/11 分别注册对应 unit/integration/blackbox 测试
     - 默认学生仓库仍为 MINIREDIS_TASK_LEVEL=0
     - 参考实现测试应能用 MINIREDIS_TASK_LEVEL=11 全部通过

  7. 文档要求：
     - 新增 tasks/task9/README.md、API.md、TESTING.md、RUBRIC.md
     - 新增 tasks/task10/README.md、API.md、TESTING.md、RUBRIC.md
     - 新增 tasks/task11/README.md、API.md、TESTING.md、RUBRIC.md
     - 更新 README.md 的任务表
     - 更新 docs/COURSE_GUIDE.md
     - 更新 docs/COMMANDS.md
     - 更新 docs/ARCHITECTURE.md
     - 更新 docs/INSTRUCTOR_GUIDE.md
     - 明确说明 RDB、主从复制、Pub/Sub 是教学简化版，不追求 Redis 完整兼容

  8. 实现约束：
     - 不引入外部依赖
     - 使用 C++17
     - 保持单线程 Reactor/epoll 模型
     - 不破坏现有 Task 0-8 API
     - 不删除学生已经需要完成的 TODO
     - 不把参考答案泄露到当前学生 starter 中
     - 如果需要新增类，优先放在 include/miniredis/... 和 src/... 对应模块中

  9. 验收：
     - 当前学生 starter：能完整编译
     - 当前学生 starter 默认 ctest 只跑 Task 0，且 ByteBuffer TODO 预期失败
     - 配置到 MINIREDIS_TASK_LEVEL=11 时能注册全部公开测试
     - 参考实现：Task 0-11 全部测试通过
     - 确认没有段错误、挂起、端口残留进程
     - 最终报告列出：
       - 新增文件
       - 修改文件
       - 学生 TODO 文件
       - 参考实现路径
       - 验证命令和结果
