# C++17 教学型 Redis 项目：完整实现、课程拆分与学生任务生成

你是一名同时负责以下工作的主协调 Agent：

1. C++17 系统软件架构师；
2. Redis 内部机制实现者；
3. 网络服务器开发者；
4. 数据库课程项目设计者；
5. 自动化测试与验收负责人；
6. 教学材料编写者。

你的任务不是简单写一个 Redis Demo，而是制作一个具有 MIT、CMU 等公开系统课程项目风格的教学型工程：

> 使用 C++17 从零实现一个 Redis 风格的单机内存数据库服务器，先完成经过充分测试的完整参考实现，再从参考实现中系统地拆除部分代码，生成可供学生分阶段完成的课程项目。

项目应当具备明确的架构、逐阶段任务、公开测试、隐藏测试设计、验收标准、调试指南和完整参考答案。

项目名称暂定为：

```text
MiniRedis17
```

如果当前仓库已有名称、目录或约定，应优先尊重现有仓库，但必须记录调整原因。

---

# 一、总体目标

最终交付一个 Linux 平台上的 Redis 风格服务端，使用 C++17 编写，支持：

* RESP2 协议；
* 二进制安全的 Key 和 Value；
* TCP 长连接；
* 非阻塞 Socket；
* epoll 事件循环；
* 单线程命令执行模型；
* 增量协议解析；
* 一个连接中连续发送多个命令；
* 一个协议帧被拆成多次网络读取；
* 部分写与发送缓冲区；
* 字符串、列表和哈希类型；
* Key 过期时间；
* 惰性删除和主动过期；
* AOF 持久化；
* 启动时 AOF 恢复；
* 基本配置加载；
* 日志、错误处理和优雅退出；
* 单元测试、集成测试和黑盒测试；
* 可使用 `redis-cli` 对支持的命令子集进行测试。

本项目不是完整复刻 Redis，不要求兼容所有 Redis 行为。

---

# 二、工作方式

## 2.1 主 Agent 职责

你是项目负责人，不应把所有分析和实现都塞进同一上下文。

如果运行环境支持子 Agent，必须主动分配以下工作：

### 架构子 Agent

负责：

* 模块边界；
* 数据结构设计；
* 命令执行流程；
* 网络模型；
* 错误模型；
* 生命周期和资源所有权；
* 可测试性设计。

### 协议与网络子 Agent

负责：

* RESP2 兼容性检查；
* 增量解析状态机；
* 半包、粘包和流水线请求；
* epoll 使用；
* 部分读写；
* 连接关闭；
* 背压和缓冲区上限。

### 存储与持久化子 Agent

负责：

* Value 类型系统；
* TTL 机制；
* AOF 格式；
* AOF 重放；
* 损坏尾部处理；
* 数据一致性语义。

### 测试子 Agent

负责：

* 单元测试；
* 集成测试；
* 黑盒测试；
* 边界条件；
* Sanitizer；
* 故障注入；
* 隐藏测试设计。

### 教学设计子 Agent

负责：

* 阶段拆分；
* 学习目标；
* 学生工作量；
* Task Explain；
* Starter Code；
* 测试提示；
* 常见错误；
* 评分标准。

### 最终审计子 Agent

在所有实现完成后独立检查：

* 是否存在未实现代码；
* 是否存在绕过测试的硬编码；
* 是否有未处理的系统调用错误；
* 是否有未定义行为；
* 是否有协议兼容问题；
* 是否有教学阶段断层；
* 是否能从干净环境构建；
* 学生版本是否泄露参考答案。

主 Agent负责决策、合并、冲突处理和最终交付。

---

# 三、强制工作顺序

必须严格按照以下顺序执行。

## 阶段 A：仓库审计和设计

先检查当前仓库，确认：

* 已有目录结构；
* 已有代码；
* 构建系统；
* 测试框架；
* Git 状态；
* 可用编译器；
* 可用依赖；
* 是否已有协议、网络或存储代码。

然后编写：

```text
docs/IMPLEMENTATION_PLAN.md
docs/ARCHITECTURE.md
docs/PROTOCOL.md
docs/COMMANDS.md
```

在文档中明确：

* 项目范围；
* 非目标；
* 模块依赖；
* 数据流；
* 请求处理流程；
* 生命周期；
* 错误处理；
* 线程模型；
* 持久化语义；
* 测试策略。

不要在架构尚未确定时直接大规模编码。

## 阶段 B：完整参考实现

先完成完整、可运行、可测试的 Instructor Solution。

此时不得提前拆除学生代码，不得使用空函数、伪实现或教学占位符代替真实实现。

完整参考实现必须通过全部测试。

## 阶段 C：独立验收

完整实现完成后，必须执行：

* Debug 构建；
* Release 构建；
* AddressSanitizer；
* UndefinedBehaviorSanitizer；
* 单元测试；
* 集成测试；
* 黑盒网络测试；
* `redis-cli` 兼容测试；
* 异常输入测试；
* AOF 重启恢复测试；
* 优雅退出测试。

发现问题后先修复参考实现。

## 阶段 D：教学拆分

只有参考实现稳定后，才能开始生成学生版本。

学生版本必须从参考实现可重复生成，禁止手工长期维护两个逐渐分叉的代码库。

## 阶段 E：逐阶段验收

对每一个教学任务分别验证：

* 任务开始版本能够正常编译；
* 学生待完成位置清晰；
* 不需要修改未说明的文件；
* 当前任务公开测试 initially fail；
* 完成当前任务后公开测试通过；
* 前面任务不会被破坏；
* 后续任务不会提前暴露实现；
* 任务文档与代码接口一致。

---

# 四、项目范围

## 4.1 必须实现的 RESP2 类型

支持编码和解析：

* Simple String；
* Error；
* Integer；
* Bulk String；
* Null Bulk String；
* Array；
* Null Array。

服务器接收的命令主要使用：

```text
Array<Optional Bulk String>
```

必须正确处理：

* 空 Bulk String；
* 二进制数据；
* `\0` 字节；
* 负长度；
* 非法数字；
* 非法 CRLF；
* 超大长度；
* 深度过大的嵌套；
* 不完整输入；
* 一次输入多个完整请求；
* 一个请求分多次输入；
* 解析错误后的连接处理。

禁止通过按行读取或简单 `split(' ')` 假装实现 RESP。

RESP Parser 必须是增量解析器，能够保留解析状态和未消费输入。

---

## 4.2 必须实现的命令

### 连接命令

```text
PING
ECHO
QUIT
```

### 通用 Key 命令

```text
DEL
EXISTS
TYPE
```

### String 命令

```text
SET
GET
MSET
MGET
APPEND
STRLEN
INCR
DECR
```

`SET` 至少支持：

```text
SET key value
SET key value EX seconds
SET key value PX milliseconds
SET key value NX
SET key value XX
```

允许同时支持：

```text
SET key value NX EX seconds
SET key value XX PX milliseconds
```

必须明确选项冲突规则。

### List 命令

```text
LPUSH
RPUSH
LPOP
RPOP
LLEN
LRANGE
```

必须明确：

* 负索引；
* 越界索引；
* 空列表；
* 最后一个元素弹出后的 Key 删除；
* Wrong Type 错误。

### Hash 命令

```text
HSET
HGET
HDEL
HLEN
HGETALL
```

必须明确：

* 新字段和覆盖字段的返回值；
* 空 Hash 的 Key 删除；
* 返回顺序是否固定。

为了保证测试确定性，`HGETALL` 必须采用稳定、明确、可测试的顺序。可以使用有序容器，也可以在输出前排序，但必须写入规范。

### TTL 命令

```text
EXPIRE
PEXPIRE
TTL
PTTL
PERSIST
```

必须明确：

* Key 不存在；
* Key 无过期时间；
* Key 已过期；
* TTL 舍入规则；
* 修改 Value 后是否保留 TTL；
* `SET` 是否清除旧 TTL；
* `INCR`、`APPEND` 是否保留 TTL。

---

## 4.3 可选高级任务

在核心项目完成后，可以把下列内容设计为 Bonus，不得阻塞核心任务：

```text
MULTI
EXEC
DISCARD
AOF Rewrite
SCAN
Publish/Subscribe
简单内存淘汰策略
性能基准工具
RESP3
```

Bonus 不应污染核心架构。

---

## 4.4 明确不实现

本项目核心范围不包括：

* Redis Cluster；
* 主从复制；
* Sentinel；
* Lua；
* ACL；
* TLS；
* RDB；
* Stream；
* Sorted Set；
* Bitmap；
* HyperLogLog；
* GEO；
* 模块系统；
* 完整 Redis 配置语法；
* 多线程命令并行执行；
* 分布式一致性；
* 生产级安全防护。

文档中必须明确这些非目标，防止项目失控。

---

# 五、建议架构

可以调整名称，但必须保持相同层次和职责边界。

```text
miniredis/
├── CMakeLists.txt
├── README.md
├── cmake/
├── config/
│   └── miniredis.conf
├── docs/
│   ├── ARCHITECTURE.md
│   ├── PROTOCOL.md
│   ├── COMMANDS.md
│   ├── IMPLEMENTATION_PLAN.md
│   ├── COURSE_GUIDE.md
│   └── INSTRUCTOR_GUIDE.md
├── include/miniredis/
│   ├── base/
│   │   ├── result.h
│   │   ├── error.h
│   │   └── clock.h
│   ├── resp/
│   │   ├── value.h
│   │   ├── encoder.h
│   │   └── parser.h
│   ├── storage/
│   │   ├── value.h
│   │   ├── database.h
│   │   └── expiration.h
│   ├── command/
│   │   ├── command.h
│   │   ├── registry.h
│   │   └── executor.h
│   ├── server/
│   │   ├── byte_buffer.h
│   │   ├── connection.h
│   │   ├── session.h
│   │   ├── event_loop.h
│   │   └── server.h
│   ├── persistence/
│   │   ├── aof.h
│   │   └── aof_replay.h
│   └── config/
│       └── config.h
├── src/
│   ├── resp/
│   ├── storage/
│   ├── command/
│   ├── server/
│   ├── persistence/
│   ├── config/
│   └── main.cpp
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── blackbox/
│   ├── fixtures/
│   └── hidden_spec/
├── tasks/
│   ├── task0/
│   ├── task1/
│   ├── task2/
│   ├── task3/
│   ├── task4/
│   ├── task5/
│   ├── task6/
│   ├── task7/
│   └── task8/
├── tools/
│   ├── generate_student_repo.py
│   ├── resp_client.py
│   ├── run_blackbox_tests.py
│   └── benchmark.py
└── scripts/
    ├── format.sh
    ├── build.sh
    ├── test.sh
    └── sanitizer.sh
```

不得为了机械符合此目录而破坏已有仓库；如果调整，记录映射关系。

---

# 六、核心技术要求

## 6.1 C++ 标准和代码规则

必须使用：

```text
C++17
```

要求：

* RAII；
* 明确的资源所有权；
* 禁止裸 owning pointer；
* 使用 `std::unique_ptr`、`std::shared_ptr` 时说明所有权原因；
* 避免全局可变状态；
* 系统调用返回值必须检查；
* 禁止未定义行为；
* 禁止依赖未初始化内存；
* 禁止吞掉异常或错误；
* 禁止静默忽略协议错误；
* 禁止为通过测试进行硬编码；
* 头文件依赖尽量最小；
* 对外接口有必要注释；
* 内部实现不要堆积无意义注释。

编译至少启用：

```text
-Wall
-Wextra
-Wpedantic
-Wconversion
-Wshadow
```

对于无法启用的警告，需要记录原因。

---

## 6.2 构建要求

使用 CMake。

要求：

* 不在构建期间从互联网下载依赖；
* 干净环境可离线构建；
* 支持 Debug 和 Release；
* 支持 Sanitizer；
* 支持 CTest；
* 测试框架需要随仓库提供或使用自包含测试框架；
* 不能依赖 IDE；
* 提供一条命令完成构建；
* 提供一条命令运行全部测试。

例如：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

---

## 6.3 RESP Parser

Parser 必须设计成状态机或等价的显式增量解析结构。

建议接口：

```cpp
class RespParser {
 public:
  ParseResult Parse(std::string_view input);
  void Reset();
};
```

但不得在每次解析时假设输入包含完整消息。

需要能够表示：

```cpp
enum class ParseStatus {
  kComplete,
  kNeedMoreData,
  kProtocolError,
};
```

返回结果必须包含：

* 当前状态；
* 已消费字节数；
* 解析出的 Value；
* 错误原因；
* 错误位置或上下文。

必须限制：

* 最大 Bulk String 长度；
* 最大 Array 元素数；
* 最大嵌套深度；
* 单连接最大缓冲区；
* 单次请求最大字节数。

限制应可配置或集中定义，不能散落魔法数字。

---

## 6.4 网络模型

核心服务器采用：

```text
Linux + non-blocking socket + epoll
```

线程模型：

```text
单线程事件循环
```

要求正确处理：

* `accept4`；
* 非阻塞模式；
* `EPOLLIN`；
* `EPOLLOUT`；
* `EPOLLERR`；
* `EPOLLHUP`；
* `EPOLLRDHUP`；
* `EINTR`；
* `EAGAIN`；
* 部分读取；
* 部分写入；
* 客户端正常关闭；
* 客户端异常关闭；
* 连接对象销毁；
* 文件描述符复用；
* 输出缓冲区；
* 输入缓冲区上限；
* 慢客户端；
* 一个连接多个流水线请求。

不得假设一次 `recv` 得到一个完整命令。

不得假设一次 `send` 能发送全部响应。

连接关闭时不得出现：

* Use After Free；
* 重复关闭；
* epoll 中残留无效指针；
* 文件描述符泄漏。

---

## 6.5 Command 层

协议层、命令层和存储层必须分离。

建议流程：

```text
Socket bytes
  -> Input Buffer
  -> RESP Parser
  -> Request Validation
  -> Command Registry
  -> Command Executor
  -> Database
  -> RESP Response
  -> Output Buffer
  -> Socket
```

每条命令必须统一定义：

* 命令名；
* 参数数量；
* 是否修改数据；
* 是否允许写入 AOF；
* 执行函数；
* 参数错误信息。

命令名大小写不敏感。

不得在 Session 中堆积大量 `if/else if` 实现全部命令。

---

## 6.6 数据库模型

Key 必须二进制安全。

建议使用：

```cpp
using Key = std::string;
```

Value 可以使用：

```cpp
std::variant<StringValue, ListValue, HashValue>
```

必须处理 Wrong Type。

数据库接口不得直接依赖网络对象或 RESP Parser。

时间相关逻辑必须通过可注入 Clock 访问，禁止在所有函数中直接调用系统时间。

测试中需要提供 FakeClock，以便确定性测试 TTL。

---

## 6.7 过期机制

实现两种过期方式：

### 惰性过期

访问 Key 时检查是否到期，到期则删除并视为不存在。

### 主动过期

事件循环周期性处理一批过期 Key。

可以使用：

* 最小堆；
* 时间轮；
* 分桶；
* 采样扫描。

优先选择教学上清晰且能保证正确性的方案。

如果使用最小堆，应处理 TTL 更新后产生的过期旧堆项，例如使用版本号或再次校验当前过期时间。

所有过期逻辑必须使用统一时钟单位，避免秒、毫秒混用。

---

## 6.8 AOF

AOF 使用 RESP 命令格式或其他明确、可调试的追加格式。

优先使用 RESP 格式，便于重放和检查。

必须实现：

* 写命令成功后追加；
* 只记录会修改数据的命令；
* 启动时重放；
* 保持命令顺序；
* 对损坏或截断尾部进行明确处理；
* I/O 错误不得静默忽略；
* 配置是否启用 AOF；
* 明确 fsync 策略。

至少支持：

```text
appendonly yes|no
appendfsync always|no
```

可以将 `everysec` 设计为扩展。

必须明确命令执行与 AOF 写入的先后顺序，以及写入失败时服务器的行为。

---

## 6.9 程序入口与优雅退出

必须支持：

```bash
./miniredis-server --config config/miniredis.conf
```

配置至少包括：

```text
bind
port
appendonly
appendfilename
appendfsync
max-client-input-buffer
max-client-output-buffer
max-bulk-length
active-expire-interval-ms
log-level
```

处理：

```text
SIGINT
SIGTERM
```

收到退出信号后：

1. 停止接受新连接；
2. 结束事件循环；
3. 尽量完成必要的 AOF flush；
4. 关闭客户端连接；
5. 释放资源；
6. 返回明确退出码。

信号处理函数内部不得执行不安全的复杂操作。

---

# 七、完整参考实现要求

参考实现必须是可以独立运行的完整程序。

不得存在：

```cpp
TODO
FIXME
throw std::logic_error("not implemented")
return {};
return nullptr;  // 仅为了占位
```

除非这些内容位于明确的学生版本中。

参考实现需要覆盖：

* 正常路径；
* 输入错误；
* 边界条件；
* 系统调用失败；
* 文件操作失败；
* 网络断开；
* TTL；
* AOF 恢复。

完整实现完成后生成一份：

```text
docs/REFERENCE_IMPLEMENTATION_REPORT.md
```

内容包括：

* 实际架构；
* 关键设计决策；
* 与初始计划的差异；
* 测试结果；
* 已知限制；
* 性能测试结果；
* 后续扩展点。

---

# 八、学生版本生成机制

禁止手工复制一份代码，然后分别维护 Solution 和 Starter。

必须设计可重复生成机制。

推荐方案之一：

## 方案 A：代码区域标记

在参考实现中使用受控标记：

```cpp
// BEGIN STUDENT TODO(task=2)
// 完整参考实现
// END STUDENT TODO(task=2)
```

生成工具根据任务阶段：

* 删除后续任务实现；
* 替换为编译通过的 Stub；
* 保留函数签名；
* 插入简洁 TODO；
* 不泄露参考答案。

## 方案 B：Git Commit 和 Patch

维护：

```text
task0-start
task0-solution
task1-start
task1-solution
...
```

使用脚本从参考分支生成教学检查点。

## 方案 C：结构化补丁清单

使用机器可读的任务清单和补丁生成各阶段版本。

无论选择哪一种，都必须满足：

* 可重复；
* 可验证；
* 不依赖手工记忆；
* 不维护两套逐渐分叉的实现；
* 学生版本不包含参考答案；
* Instructor 版本可以恢复完整实现。

生成工具需要有自己的测试。

---

# 九、课程任务拆分

建议拆分为 Task 0 至 Task 8。

如果实现过程中发现更合理的拆分，可以调整，但必须说明原因，并保证学习曲线平滑。

---

## Task 0：工程基础、错误模型与字节缓冲区

### 学习目标

学生需要理解：

* CMake 项目结构；
* 测试运行方式；
* RAII；
* Result/Error 模型；
* 字节缓冲区；
* 已消费数据和未消费数据；
* 网络程序为什么不能假设一次读取一个完整请求。

### 学生实现内容

建议包括：

* `Result<T>` 或错误返回结构；
* `ByteBuffer`；
* 追加数据；
* 查看连续数据；
* 消费指定字节；
* 压缩已消费空间；
* 缓冲区大小限制。

### 可能修改文件

```text
include/miniredis/base/result.h
include/miniredis/server/byte_buffer.h
src/server/byte_buffer.cpp
tests/unit/byte_buffer_test.cpp
```

### 验收重点

* 空缓冲区；
* 多次追加；
* 部分消费；
* 全部消费；
* 大数据；
* 超出上限；
* 消费越界；
* 数据移动后内容不变。

Task 0 应控制难度，不要让学生一开始就处理 epoll。

---

## Task 1：RESP Value 与编码器

### 学习目标

学生需要理解：

* RESP2 数据模型；
* 二进制安全；
* 编码和网络协议格式；
* Null 和 Empty 的区别；
* 递归数据结构。

### 学生实现内容

* RESP Value 类型；
* Simple String 编码；
* Error 编码；
* Integer 编码；
* Bulk String 编码；
* Null Bulk String；
* Array；
* Null Array。

### 可能修改文件

```text
include/miniredis/resp/value.h
include/miniredis/resp/encoder.h
src/resp/encoder.cpp
tests/unit/resp_encoder_test.cpp
```

### 验收重点

* 空字符串；
* 包含 `\0`；
* 包含 CRLF；
* 负整数；
* 嵌套数组；
* Null Bulk String；
* Empty Bulk String；
* Null Array；
* Empty Array。

---

## Task 2：增量 RESP Parser

### 学习目标

学生需要理解：

* 状态机；
* 增量解析；
* 半包；
* 粘包；
* 流式输入；
* 解析器安全限制；
* “需要更多数据”和“协议错误”的区别。

### 学生实现内容

* 类型前缀识别；
* CRLF 检查；
* 整数解析；
* Bulk String 长度解析；
* Array 递归或显式栈解析；
* 已消费字节统计；
* 不完整输入状态；
* 错误状态；
* 深度和长度限制。

### 可能修改文件

```text
include/miniredis/resp/parser.h
src/resp/parser.cpp
tests/unit/resp_parser_test.cpp
tests/integration/resp_stream_test.cpp
```

### 必测情况

将一个请求按以下方式切割并逐块输入：

```text
每个字节一块
随机大小分块
Header 和 Body 分离
CR 与 LF 分离
多个请求一次输入
完整请求后跟半个请求
```

不得通过把全部输入复制到临时字符串再反复重新解析来掩盖状态设计问题，除非能够证明复杂度和内存行为合理。

---

## Task 3：Database 与 String 命令

### 学习目标

学生需要理解：

* 存储层与协议层分离；
* 命令参数检查；
* 类型系统；
* 整数溢出；
* 原子命令语义；
* 可测试的数据库 API。

### 学生实现内容

* Database 基础结构；
* String Value；
* Command Registry；
* Command Executor；
* `PING`；
* `ECHO`；
* `SET`；
* `GET`；
* `DEL`；
* `EXISTS`；
* `TYPE`；
* `MSET`；
* `MGET`；
* `APPEND`；
* `STRLEN`；
* `INCR`；
* `DECR`。

### 可能修改文件

```text
include/miniredis/storage/value.h
include/miniredis/storage/database.h
src/storage/database.cpp
include/miniredis/command/registry.h
src/command/registry.cpp
src/command/string_commands.cpp
tests/unit/database_test.cpp
tests/integration/string_command_test.cpp
```

### 必须处理

* Key 不存在；
* 空 Value；
* 二进制 Value；
* Wrong Type；
* 非整数；
* 正负整数；
* `int64_t` 溢出；
* MSET 参数数量；
* MGET 缺失 Key；
* SET NX/XX；
* SET EX/PX；
* 选项重复或冲突。

---

## Task 4：List、Hash 与类型安全

### 学习目标

学生需要理解：

* 一个数据库支持多种 Value 类型；
* 容器类型命令；
* 空容器清理；
* 范围索引；
* 类型错误；
* 确定性输出。

### 学生实现内容

List：

```text
LPUSH
RPUSH
LPOP
RPOP
LLEN
LRANGE
```

Hash：

```text
HSET
HGET
HDEL
HLEN
HGETALL
```

### 可能修改文件

```text
include/miniredis/storage/value.h
src/command/list_commands.cpp
src/command/hash_commands.cpp
tests/integration/list_command_test.cpp
tests/integration/hash_command_test.cpp
```

### 必须处理

* Wrong Type；
* 空容器；
* 负索引；
* 越界；
* 删除最后一个元素；
* 重复 Hash Field；
* HSET 返回新增字段数量；
* HGETALL 稳定顺序。

---

## Task 5：Session 与命令流水线

### 学习目标

学生需要理解：

* 协议解析和命令执行的连接方式；
* 一个连接的会话状态；
* Pipeline；
* 输入缓冲区；
* 输出缓冲区；
* 错误恢复；
* QUIT 行为。

### 学生实现内容

* Session；
* 从输入缓冲区循环解析；
* 将 RESP Array 转换成命令参数；
* 命令分发；
* 编码响应；
* 多命令流水线；
* QUIT；
* 协议错误处理。

### 可能修改文件

```text
include/miniredis/server/session.h
src/server/session.cpp
tests/integration/session_test.cpp
tests/integration/pipeline_test.cpp
```

Session 必须可以脱离真实 Socket 测试。

不得把 Socket I/O 与命令执行紧密耦合。

---

## Task 6：非阻塞 TCP 与 epoll 事件循环

### 学习目标

学生需要理解：

* Socket 生命周期；
* 非阻塞 I/O；
* epoll；
* Edge Trigger 和 Level Trigger 的区别；
* 部分读写；
* 背压；
* 连接清理；
* 网络服务端资源管理。

默认优先使用 Level Trigger，除非选择 Edge Trigger 并完整证明所有读写循环正确。

### 学生实现内容

* 监听 Socket；
* bind/listen；
* accept；
* 非阻塞设置；
* epoll 注册；
* 连接表；
* 读事件；
* 写事件；
* 输出缓冲区；
* 连接关闭；
* 错误事件；
* 优雅退出。

### 可能修改文件

```text
include/miniredis/server/connection.h
include/miniredis/server/event_loop.h
include/miniredis/server/server.h
src/server/connection.cpp
src/server/event_loop.cpp
src/server/server.cpp
src/main.cpp
tests/blackbox/network_test.py
```

### 必测情况

* 多客户端；
* 单客户端 Pipeline；
* 每字节发送；
* 客户端发送一半后断开；
* 客户端不读取响应；
* 大响应部分写；
* 文件描述符快速复用；
* 服务端 SIGINT；
* 超出输入缓冲区；
* 超出输出缓冲区。

---

## Task 7：TTL 与过期系统

### 学习目标

学生需要理解：

* 时间语义；
* 惰性过期；
* 主动过期；
* 可注入 Clock；
* 过期索引；
* 陈旧过期记录；
* 确定性时间测试。

### 学生实现内容

```text
EXPIRE
PEXPIRE
TTL
PTTL
PERSIST
```

以及：

* FakeClock；
* 惰性删除；
* 主动过期；
* SET EX/PX；
* TTL 更新；
* TTL 清除；
* 过期后 Key 不可见。

### 可能修改文件

```text
include/miniredis/base/clock.h
include/miniredis/storage/expiration.h
src/storage/expiration.cpp
src/storage/database.cpp
src/command/expire_commands.cpp
tests/unit/expiration_test.cpp
tests/integration/ttl_command_test.cpp
```

测试不得依赖真实 `sleep` 作为主要验证手段。

---

## Task 8：AOF 持久化与恢复

### 学习目标

学生需要理解：

* Write-Ahead Log；
* 命令日志；
* 重放；
* 崩溃尾部；
* I/O 错误；
* 持久化顺序；
* 重启恢复。

### 学生实现内容

* AOF Writer；
* 修改命令序列化；
* 文件追加；
* fsync；
* 启动重放；
* 截断尾部处理；
* 禁止重放时再次写入 AOF；
* AOF 配置；
* 恢复测试。

### 可能修改文件

```text
include/miniredis/persistence/aof.h
include/miniredis/persistence/aof_replay.h
src/persistence/aof.cpp
src/persistence/aof_replay.cpp
src/config/config.cpp
tests/unit/aof_test.cpp
tests/integration/recovery_test.cpp
```

### 必测情况

* 空文件；
* 多条命令；
* 进程重启；
* 过期命令恢复；
* 截断的最后一个命令；
* 中间位置损坏；
* 文件不可写；
* 磁盘写错误模拟；
* 重放时非法命令。

---

# 十、每个 Task Explain 的固定要求

每个目录：

```text
tasks/taskN/
```

至少包含：

```text
README.md
API.md
TESTING.md
RUBRIC.md
```

`README.md` 必须使用适合学生直接阅读的教学语言，不要泄露完整实现。

每个任务说明必须包含以下部分。

## 10.1 背景

解释当前模块在整个系统中的位置，以及为什么需要它。

## 10.2 学习目标

列出学生完成任务后应真正理解的概念，不能只写“完成某个函数”。

## 10.3 当前代码状态

说明：

* 已经提供什么；
* 哪些接口已经固定；
* 哪些测试可以运行；
* 当前服务器能做到什么；
* 当前服务器还不能做到什么。

## 10.4 需要完成的工作

按照逻辑顺序列出具体工作。

不要只写：

```text
实现 parser.cpp
```

应写成：

```text
1. 识别 RESP 类型前缀。
2. 在输入不完整时返回 NeedMoreData。
3. 解析长度时检查符号、溢出和非法字符。
4. 解析 Bulk String 时不得把 Null 和 Empty 混淆。
5. 返回准确的 consumed 字节数。
```

## 10.5 需要修改的文件

明确列出：

* 必须修改；
* 允许修改；
* 不建议修改；
* 禁止修改。

## 10.6 关键接口

展示学生必须理解的函数签名，但不能直接给出完整实现。

## 10.7 实现建议

给出方向、算法和拆解方式，但不要变成逐行答案。

## 10.8 不变量

例如：

* Parser 不得消费不完整帧；
* Database 中不存在已经逻辑过期但仍可读取的 Key；
* Connection 的 fd 只由 Connection 所有；
* 输出缓冲区中的数据顺序与命令执行顺序一致。

## 10.9 边界条件

明确列出学生容易漏掉的输入。

## 10.10 常见错误

指出典型错误及其表现，例如：

* 把 Null Bulk String 当成 Empty Bulk String；
* `recv == 0` 仍继续读；
* 把 `EAGAIN` 当成连接错误；
* 使用 `atoi` 导致溢出未检测；
* TTL 测试依赖真实时间；
* HGETALL 使用无序容器造成测试随机失败。

## 10.11 测试方法

列出：

* 当前任务测试命令；
* 单独运行某个测试；
* 使用 `redis-cli` 验证；
* 使用 Sanitizer；
* 如何查看日志；
* 如何构造半包。

## 10.12 验收标准

必须是可操作的检查项，不能写模糊描述。

## 10.13 评分标准

给出建议评分权重，例如：

```text
正确性 60%
边界条件 15%
资源安全 10%
代码结构 10%
测试补充 5%
```

不同任务可以调整。

## 10.14 禁止行为

例如：

* 修改测试绕过失败；
* 硬编码测试输入；
* 绕过既有接口重写整个项目；
* 用阻塞 I/O 代替要求的非阻塞 I/O；
* 使用 sleep 掩盖同步问题；
* 静默忽略错误。

## 10.15 完成后的系统能力

说明完成本任务后，项目新增了什么实际能力。

---

# 十一、Starter Code 设计原则

学生版本不是把所有 `.cpp` 清空。

必须保留：

* 清晰的架构；
* 固定接口；
* 类型定义；
* 必要注释；
* 构建文件；
* 测试基础设施；
* 前置任务已经完成的代码；
* 当前任务的可编译 Stub；
* 合理 TODO；
* 足够的上下文。

不得保留：

* 当前任务完整答案；
* 可直接复制的参考算法；
* 注释掉的 Solution；
* 二进制形式的参考答案；
* 通过 Git 历史直接看到的 Solution，除非 Instructor 仓库和学生仓库明确分离。

每个阶段开始版本必须能够完整编译。

可以让当前任务测试失败，但不能因为链接错误、缺少符号或无关模块损坏而无法运行。

---

# 十二、测试体系

至少分为四层。

## 12.1 单元测试

针对：

* ByteBuffer；
* RESP Encoder；
* RESP Parser；
* Database；
* Expiration；
* AOF；
* 配置解析。

## 12.2 集成测试

针对：

* Parser + Session；
* Session + Command；
* Command + Database；
* TTL；
* AOF Replay。

## 12.3 黑盒测试

真实启动服务器，通过 TCP 测试：

* 单命令；
* Pipeline；
* 半包；
* 粘包；
* 多客户端；
* 重启；
* AOF；
* 优雅退出。

## 12.4 隐藏测试设计

创建：

```text
tests/hidden_spec/
```

其中不一定保存真实隐藏测试代码，但必须为教师记录：

* 隐藏测试目标；
* 输入类别；
* 预期行为；
* 防止何种错误实现；
* 对应评分点。

学生文档可以提示测试范围，但不得给出隐藏测试具体数据。

---

# 十三、测试质量要求

不得只测试正常路径。

必须包含：

* 空输入；
* 最大值；
* 最小值；
* 溢出；
* 非法长度；
* 不完整协议；
* 多命令；
* 二进制数据；
* 资源耗尽边界；
* 连接断开；
* 文件错误；
* TTL 边界；
* Wrong Type；
* 命令参数数量错误；
* 命令大小写；
* 超大输入；
* 递归深度限制。

测试必须确定性运行。

随机测试必须：

* 固定 Seed；
* 失败时打印 Seed；
* 输出最小必要上下文。

可以对 RESP Parser 增加轻量 Fuzz 或属性测试，但不能依赖在线下载工具。

---

# 十四、Redis CLI 兼容验证

提供明确命令，例如：

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

对于与 Redis 官方行为不一致的地方：

* 不得假装完全兼容；
* 必须在 `docs/COMMANDS.md` 中明确记录；
* 测试以项目规范为准；
* 差异应尽量少且有理由。

---

# 十五、教学检查点

为每个任务提供可恢复的检查点。

推荐：

```text
task0-start
task0-solution
task1-start
task1-solution
...
task8-start
task8-solution
```

每个检查点必须验证：

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

对于 `taskN-start`：

* 前置任务测试必须通过；
* 当前任务测试应失败或部分失败；
* 后续任务测试可以不启用。

对于 `taskN-solution`：

* 从 Task 0 到 Task N 的测试必须全部通过。

---

# 十六、文档交付

最终至少提供：

## 学生文档

```text
README.md
docs/COURSE_GUIDE.md
docs/ARCHITECTURE.md
docs/PROTOCOL.md
docs/COMMANDS.md
tasks/task0/README.md
...
tasks/task8/README.md
```

## 教师文档

```text
docs/INSTRUCTOR_GUIDE.md
docs/REFERENCE_IMPLEMENTATION_REPORT.md
docs/GRADING_GUIDE.md
tests/hidden_spec/
```

教师文档需包括：

* 每个任务的完整知识点；
* 建议评分；
* 隐藏测试；
* 常见错误；
* 参考实现关键决策；
* 各任务预计难度；
* 任务依赖关系；
* 如何生成 Starter；
* 如何恢复 Solution；
* 如何发布单独任务版本。

---

# 十七、README 必须包含

主 README 至少包括：

* 项目简介；
* 学习目标；
* 架构概览；
* 已支持命令；
* 不支持功能；
* 环境要求；
* 构建方式；
* 启动方式；
* 测试方式；
* redis-cli 使用方式；
* 任务顺序；
* 目录说明；
* 常见问题；
* 学术诚信说明。

不要把 README 写成营销文案。

---

# 十八、代码质量验收

最终运行以下检查。

## 编译

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j

cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
```

## 测试

```bash
ctest --test-dir build-debug --output-on-failure
```

## Sanitizer

```bash
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMINIREDIS_ENABLE_ASAN=ON \
  -DMINIREDIS_ENABLE_UBSAN=ON

cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

## 格式检查

使用项目固定版本或配置的：

```text
clang-format
```

如果提供 `clang-tidy`，不得让其成为无法离线安装的强制依赖。

## 资源检查

检查：

* fd 泄漏；
* 内存泄漏；
* 重复 close；
* Use After Free；
* 越界；
* 整数溢出；
* 异常退出；
* 临时文件泄漏。

---

# 十九、性能与资源限制

本项目不要求达到 Redis 性能，但必须避免明显错误设计。

至少提供一个简单基准测试：

* 单连接 PING；
* Pipeline SET；
* Pipeline GET；
* 小 Value；
* 大 Value；
* 多连接。

记录：

* 测试环境；
* 请求数量；
* Value 大小；
* Pipeline 深度；
* 吞吐量；
* 延迟；
* 内存使用。

性能优化不得牺牲正确性和教学清晰度。

必须设置：

* 最大请求长度；
* 最大 Bulk String；
* 最大 Array 长度；
* 最大输入缓冲区；
* 最大输出缓冲区；
* 最大客户端数或明确当前不限制；
* 最大嵌套深度。

超过限制时必须返回错误或关闭连接，行为需写入规范。

---

# 二十、需要特别审查的问题

在最终验收中逐项回答：

1. Parser 是否真正支持半包和粘包？
2. Null Bulk String 与 Empty Bulk String 是否区分？
3. Key 和 Value 是否二进制安全？
4. 一个连接是否可以处理多个 Pipeline 命令？
5. 部分写是否会丢数据或重复发送？
6. 客户端断开后 Connection 是否安全销毁？
7. fd 复用是否可能访问旧 Connection？
8. Wrong Type 是否统一返回错误？
9. `INCR` 是否检查 `int64_t` 溢出？
10. TTL 是否使用可注入时钟？
11. TTL 更新后旧过期记录是否会误删新 Value？
12. SET 是否按规范清除或更新 TTL？
13. AOF 重放时是否再次写 AOF？
14. AOF 截断尾部如何处理？
15. AOF 写失败是否被静默忽略？
16. SIGINT/SIGTERM 是否能够优雅退出？
17. 每个 Student Checkpoint 是否可以编译？
18. Task Explain 中列出的文件是否准确？
19. 学生是否能仅根据任务书完成任务？
20. Starter 是否泄露 Solution？
21. 测试是否存在依赖真实时间的脆弱用例？
22. HGETALL 是否具有确定性顺序？
23. 命令参数错误是否统一？
24. 是否存在硬编码测试输入？
25. 是否存在无意义抽象或过度工程化？

---

# 二十一、执行原则

必须遵守：

* 先完成正确实现，再拆教学任务；
* 先保证正确性，再考虑性能；
* 每个阶段都实际编译和测试；
* 不用伪代码代替实现；
* 不用文档描述代替可运行代码；
* 不因任务复杂而缩减关键测试；
* 不为了模仿 Redis 而无限扩展范围；
* 不把协议、网络、命令、存储写进一个大类；
* 不创建无法测试的时间和 I/O 逻辑；
* 不静默吞掉错误；
* 不留下未经说明的已知失败测试；
* 不在学生任务中要求修改大量未说明文件；
* 不让后续任务破坏前面任务接口；
* 不用两套手工维护、逐渐分叉的源码。

当需求存在歧义时，选择：

1. 更容易验证；
2. 更适合教学；
3. 更接近 Redis 常见语义；
4. 更少隐藏魔法；
5. 更清晰的模块边界。

并在文档中记录假设，不要停下来反复请求确认。

---

# 二十二、最终输出要求

完成全部工作后，最终报告必须包含：

## 1. 实现状态

列出：

* 已完成模块；
* 支持命令；
* 非目标；
* 已知限制。

## 2. 架构摘要

简要说明：

* RESP；
* Session；
* Command；
* Database；
* Expiration；
* Event Loop；
* AOF。

## 3. 教学任务摘要

以表格列出：

```text
Task
主题
主要文件
学习目标
公开测试
隐藏测试重点
```

## 4. 文件变更

列出新增和主要修改文件。

## 5. 测试结果

列出实际执行的命令和结果。

不得只写“测试通过”，必须给出测试数量或测试组结果。

## 6. Student 版本生成方式

给出准确命令。

## 7. Instructor Solution 恢复方式

给出准确命令。

## 8. 审计结果

列出：

* Sanitizer；
* 协议测试；
* 网络测试；
* AOF 恢复；
* 优雅退出；
* 检查点构建。

## 9. 剩余问题

如实列出没有完成或存在风险的内容，不得伪造完成状态。

---

现在开始执行。

第一步先审计仓库并编写实现计划和架构文档。

完成完整参考实现并通过验收前，不得开始拆除学生代码。
