# 任务划分文档（T001–TXXX）

> 单人开发，每任务 1–4 小时，完成后可独立验收。依赖关系明确，禁止大规模变动。

---

## 任务总览

```
Phase 1 (T001–T020)  工程骨架 + Mock Server        [2 周]
Phase 2 (T021–T055)  客户端核心层                   [6 周]
Phase 3 (T056–T078)  Go 后端骨架                   [4 周]
Phase 4 (T079–T098)  业务逻辑层                     [4 周]
Phase 5 (T099–T130)  GUI                           [7 周]
Phase 6 (T131–T145)  集成联调 + 跨平台打包          [2 周]
```

---

## 任务通用格式

```
TXXX ─ 标题（预估工时）

依赖：TXXX, TXXX
产出：path/to/file.cpp, path/to/file.h
验收：
  □ 条件 1
  □ 条件 2
说明：补充信息（可选）
```

---

## Phase 1：工程骨架 + Mock Server（T001–T020）

> 出口标准：`yunrong_client` 可编译启动空白窗口；Mock Server 可启动并响应 HTTP/WS。

### 第 1 周：C++ 客户端骨架

---

**T001 — 项目目录与 CMake 顶层结构（1h）**

```
依赖：无
产出：client/CMakeLists.txt, client/CMakePresets.json, client/cmake/Platform.cmake
验收：
  □ cmake --preset debug -S client -B client/build/debug 配置通过（允许 Qt 未安装时失败）
  □ CMakePresets.json 包含 debug / release / ci-debug 三个 preset
  □ Platform.cmake 区分 WIN32 / UNIX 平台宏
说明：仅 CMake 配置文件，不含任何 .cpp。FetchContent 声明 nlohmann_json、spdlog。
```

---

**T002 — 核心库 CMake 骨架（0.5h）**

```
依赖：T001
产出：client/src/core/CMakeLists.txt
验收：
  □ add_library(yunrong_core STATIC) 声明存在
  □ target_link_libraries 包含 Qt6::Core Qt6::Network Qt6::WebSockets Qt6::Sql nlohmann_json spdlog
  □ target_sources 列出 net/data/service/infra 下所有 .cpp 占位路径（文件尚未存在）
  □ include(../../cmake/Platform.cmake) 在末尾
```

---

**T003 — GUI + App CMake 骨架（0.5h）**

```
依赖：T002
产出：client/src/gui/CMakeLists.txt, client/src/app/CMakeLists.txt
验收：
  □ gui/CMakeLists: add_library(yunrong_gui STATIC)，link yunrong_core + Qt6::Widgets
  □ app/CMakeLists: add_executable(yunrong_client main.cpp)，link yunrong_gui
```

---

**T004 — infra/logger 实现（1.5h）**

```
依赖：T002
产出：client/src/core/infra/logger.h, client/src/core/infra/logger.cpp
验收：
  □ initLogger() 创建 spdlog 的 console sink + file sink
  □ LOG_INFO / LOG_WARN / LOG_ERROR 宏可用
  □ setLogLevel() 可运行时切换级别
  □ getLogger() 返回全局 logger 实例
  □ 编译通过（单独编译此 .cpp + spdlog 头文件）
说明：完整实现，不是 stub。这是 Phase 1 少数需要完整实现的基础模块。
```

---

**T005 — infra/config_mgr 实现（1.5h）**

```
依赖：T002
产出：client/src/core/infra/config_mgr.h, client/src/core/infra/config_mgr.cpp
验收：
  □ Meyer's Singleton（static local）
  □ loadFromFile(path) 解析 JSON，文件不存在时返回 false + 日志警告
  □ getString / getInt / getBool 含 fallback 默认值
  □ serverHost() / serverPort() / useTls() 便捷方法
  □ 编译通过
```

---

**T006 — model 数据模型头文件（1h）**

```
依赖：T002
产出：
  client/src/core/model/message.h
  client/src/core/model/conversation.h
  client/src/core/model/contact.h
  client/src/core/model/task.h
  client/src/core/model/file_transfer.h
验收：
  □ 每个 .h 有 #pragma once + namespace core::model
  □ Message 含 msgId/convId/senderId/contentType/contentBody/status/serverSeq/timestamp/edited/revoked
  □ Conversation 含 id/type/title/lastMsgPreview/unreadCount/lastReadSeq/isPinned/isMuted
  □ Contact 含 id/username/displayName/avatarUrl/departmentId/status
  □ Task 含 notifyId/taskType/title/body/priority/status/fromUserId/toUserId/actionUrl
  □ FileTransfer 含 filePath/fileHash/fileSize/chunkSize/totalChunks/receivedChunks/progress/bytesPerSec
  □ 纯头文件，无 .cpp
说明：数据结构与 04-数据库设计文档 对齐。
```

---

**T007 — infra/thread_pool stub（0.5h）**

```
依赖：T002
产出：client/src/core/infra/thread_pool.h, client/src/core/infra/thread_pool.cpp
验收：
  □ ThreadPool 类声明（构造/析构/enqueue 模板/stop_）
  □ .cpp 中构造和析构为空实现
  □ 编译通过
说明：Phase 2 第 7 周完整实现，当前仅占位编译。
```

---

**T008 — infra/msg_bus stub + infra/crypto stub（0.5h）**

```
依赖：T002
产出：
  client/src/core/infra/msg_bus.h, client/src/core/infra/msg_bus.cpp
  client/src/core/infra/crypto.h, client/src/core/infra/crypto.cpp
验收：
  □ MsgBus<T> 模板类声明（push/tryPop/size）+ 空 .cpp
  □ crypto.h 声明 sha256Hex / sha256File + 空 .cpp
  □ 编译通过
```

---

**T009 — net 模块全部 stub（1h）**

```
依赖：T002, T006
产出：
  client/src/core/net/protocol.h, client/src/core/net/protocol.cpp
  client/src/core/net/ws_client.h, client/src/core/net/ws_client.cpp
  client/src/core/net/http_client.h, client/src/core/net/http_client.cpp
  client/src/core/net/connection_mgr.h, client/src/core/net/connection_mgr.cpp
  client/src/core/net/heartbeat.h, client/src/core/net/heartbeat.cpp
  client/src/core/net/reconnect_strategy.h, client/src/core/net/reconnect_strategy.cpp
  client/src/core/net/rate_limiter.h, client/src/core/net/rate_limiter.cpp
验收：
  □ 每个 .h 有 #pragma once + namespace core::net
  □ Protocol 类声明：encodeAuth / encodeMessage / decode / sendAck 方法签名
  □ WsClient 类声明：connect / disconnect / send / 信号 onMessageReceived
  □ HttpClient 类声明：get / post / upload / 信号 onResponse
  □ ConnectionMgr 类声明：sendMessage / uploadFile / 连接状态枚举
  □ Heartbeat 类声明：start / stop / 信号 timeout
  □ ReconnectStrategy 类声明：nextDelay / reset
  □ RateLimiter 类声明：tryConsume
  □ 每个 .cpp 只有 #include + 空实现或 return {};
  □ 编译通过
```

---

**T010 — data 模块全部 stub（0.5h）**

```
依赖：T002, T006
产出：
  client/src/core/data/i_data_store.h
  client/src/core/data/sqlite_store.h, client/src/core/data/sqlite_store.cpp
  client/src/core/data/sync_mgr.h, client/src/core/data/sync_mgr.cpp
  client/src/core/data/cache_mgr.h, client/src/core/data/cache_mgr.cpp
验收：
  □ IDataStore 纯虚接口（getMessages / saveMessage / getConversations 等）
  □ SqliteStore : IDataStore，.cpp 空实现
  □ SyncMgr 类声明（requestSync / applySyncData）
  □ CacheMgr 类声明（get / put / evict）
  □ 编译通过
```

---

**T011 — service 模块全部 stub（0.5h）**

```
依赖：T002, T006
产出：
  client/src/core/service/im_service.h, client/src/core/service/im_service.cpp
  client/src/core/service/task_service.h, client/src/core/service/task_service.cpp
  client/src/core/service/file_service.h, client/src/core/service/file_service.cpp
  client/src/core/service/report_service.h, client/src/core/service/report_service.cpp
验收：
  □ IMService 类声明：sendTextMessage / onMessageReceived 信号
  □ TaskService 类声明：getTasks / handleTask
  □ FileService 类声明：uploadFile / downloadFile / 信号 transferProgress
  □ ReportService 类声明：getMessageStats / getFileStats
  □ 编译通过
```

---

**T012 — app/main.cpp 空白 QApplication 窗口（1h）**

```
依赖：T003, T004, T005
产出：client/src/app/main.cpp
验收：
  □ QApplication 启动（appName="YunRong", appVersion 来自 APP_VERSION 宏）
  □ 调用 initLogger()
  □ 调用 ConfigManager::loadFromFile()
  □ 创建 QMainWindow 空白窗口，标题 "云融 YunRong v0.1"
  □ 窗口默认大小 1200×800
  □ 编译通过 + 运行显示空白窗口
说明：这是 Phase 1 第一个可运行的里程碑。
```

---

**T013 — tests 骨架（0.5h）**

```
依赖：T002
产出：client/tests/CMakeLists.txt, client/tests/unit/test_protocol.cpp
验收：
  □ CMakeLists 引用 GTest::gtest GTest::gmock Qt6::Test
  □ add_test 注册到 CTest
  □ test_protocol.cpp 包含一个空 TEST 用例（EXPECT_TRUE(true)）
  □ cmake --preset ci-debug 可配置，ctest 可运行通过
```

---

### 第 2 周：Go 后端 + Mock Server

---

**T014 — Go 模块初始化（0.5h）**

```
依赖：无（server/ 独立项目）
产出：server/go.mod, server/cmd/server/main.go
验收：
  □ go.mod module github.com/yunrong/server, go 1.22
  □ main.go 内 func main() 打印 "yunrong server starting..."
  □ go run ./cmd/server 可执行
```

---

**T015 — Gin 路由骨架（1h）**

```
依赖：T014
产出：server/internal/handler/routes.go, server/cmd/server/main.go（更新）
验收：
  □ go get github.com/gin-gonic/gin 依赖就位
  □ routes.go 注册 /api/v1/health → {"code":0, "message":"ok"}
  □ main.go 启动 Gin 监听 :8080
  □ curl localhost:8080/api/v1/health 返回 200
```

---

**T016 — PostgreSQL 连接池 + 自动迁移（2h）**

```
依赖：T015
产出：server/internal/store/postgres.go, server/internal/store/migrations/001_initial_schema.sql
验收：
  □ go get github.com/jackc/pgx/v5 依赖就位
  □ NewPostgresPool(dsn) 返回 *pgxpool.Pool，MaxConns=20
  □ RunMigrations 读取 migrations/*.sql 文件，按文件名排序执行
  □ main.go 启动时自动建表（users/departments/conversations/conversation_members/messages/files/tasks/sync_watermarks）
  □ 重复启动不报错（IF NOT EXISTS）
  □ 不依赖 embed（使用 os.ReadDir 读取 migrations/ 目录）
说明：SQL 文件参考 04-数据库设计文档 §3.2。当前使用单表设计（无分区）。
```

---

**T017 — Mock Server：HTTP 基础（2h）**

```
依赖：T002（引用客户端 model 和 protocol 的接口理解，但不链接）
产出：mock/CMakeLists.txt, mock/src/main.cpp, mock/src/mock_server.h
验收：
  □ 独立 CMake 项目，find_package Qt6 Core Network WebSockets
  □ MockServer 类：start(httpPort, wsPort) / stop()
  □ main.cpp 启动 QCoreApplication + MockServer
  □ 内置硬编码测试用户数据（id=1001 "张三", id=1002 "李四"）
  □ HTTP POST /api/v1/auth/login → 返回 {"code":0, "data":{"access_token":"mock_token_xxx","user_id":1001}}
  □ curl 可验证
```

---

**T018 — Mock Server：WebSocket 消息收发（2h）**

```
依赖：T017
产出：mock/src/mock_server.cpp（更新）
验收：
  □ QWebSocketServer 监听独立端口
  □ 客户端连接后，Mock Server 主动发送一条 "欢迎使用云融 Mock Server" 文本消息
  □ 客户端发送 JSON 帧 {"type":"msg","seq":1,"payload":{"text":"hello"}}
    → Mock Server 回复 {"type":"ack","seq":1} + 转发消息给所有连接的客户端
  □ 两个客户端同时连接可互发消息（模拟群聊广播）
  □ 支持 ping/pong（收到 {"type":"ping"} 回复 {"type":"pong"}）
```

---

**T019 — Mock Server：场景加载（1h）**

```
依赖：T018
产出：mock/scenarios/normal_chat.json, mock/src/mock_server.cpp（更新）
验收：
  □ normal_chat.json 定义 users/conversations/messages 数据集
  □ MockServer::loadScenario(path) 读取 JSON
  □ 模拟登录返回对应 scenario 中的 user
  □ 会话列表 API 返回 scenario 中定义的 conversations
```

---

**T020 — 首次端到端验证（1h）**

```
依赖：T012, T019
产出：无新文件（验证任务）
验收：
  □ 启动 Mock Server（mock_server --scenario normal_chat.json）
  □ 启动客户端（yunrong_client），控制台日志显示 Logger 初始化成功
  □ 客户端配置指向 Mock Server 的 HTTP/WS 端口
  □ （手动验证，不要求代码实现）客户端可调用 Mock Server 登录 API 拿到 token
说明：此为 Phase 1 出口检查点，确认骨架链路全通。实际登录从 GUI 发起在 Phase 5 实现。
```

---

## Phase 2：客户端核心层（T021–T055）

> 出口标准：yunrong_core 静态库完整可用，网络/数据/基础设施全部通过单元测试，覆盖率 ≥ 70%。

### 第 3–4 周：Protocol + WebSocket

---

**T021 — Protocol::encode / decode JSON 文本帧（3h）**

```
依赖：T006, T009
产出：client/src/core/net/protocol.cpp（完整实现）
验收：
  □ encodeAuth(token, seq) → JSON 字符串，含 ver/type/seq/ts/payload
  □ encodeMessage(convId, contentBody, seq) → JSON 字符串
  □ encodeAck(ackSeq) → JSON 字符串
  □ encodeRead(convId, readToSeq) → JSON 字符串
  □ encodeSync(sinceSeq, limit) → JSON 字符串
  □ decode(jsonStr) → 结构体 DecodedFrame {type, seq, ts, payload}
  □ 非法 JSON → 返回 std::nullopt 或抛异常
  □ 单元测试覆盖所有 encode/decode 配对
```

---

**T022 — Protocol::encode / decode 二进制帧（1.5h）**

```
依赖：T021
产出：client/src/core/net/protocol.cpp（更新）
验收：
  □ encodeThumbnail(data) → QByteArray，格式 [TotalLen 4B|Type 0x01|Payload]
  □ decodeBinaryFrame(bytes) → {type, payload}
  □ TotalLen 大端序
  □ 单元测试：编解码往返一致
```

---

**T023 — WsClient 基础连接/断开/收发（3h）**

```
依赖：T021
产出：client/src/core/net/ws_client.cpp（完整实现）
验收：
  □ connect(url, token) → QWebSocket 建立连接，URL 包含 ?token=xxx
  □ disconnect() → 关闭连接
  □ send(jsonStr) → 发送文本帧
  □ 信号 messageReceived(QString) 收到服务端文本帧时触发
  □ 信号 connected() / disconnected()
  □ 信号 error(QString)
  □ 自动处理 TLS（wss:// 开头的 URL）
  □ 单元测试：使用 Mock Server（T018）验证连接 + 收发
```

---

**T024 — Heartbeat 心跳检测（1.5h）**

```
依赖：T023
产出：client/src/core/net/heartbeat.cpp（完整实现）
验收：
  □ 构造函数注入 WsClient&，监听其 messageReceived 信号
  □ 服务端发送 {"type":"ping"} → 自动回复 {"type":"pong"}
  □ 90 秒未收到任何帧（含 ping）→ 触发信号 timeout()
  □ 不主动发 ping（心跳由服务端主导，与 03 号协议文档一致）
  □ 单元测试：Mock Server 控制 ping 发送间隔，验证超时触发
```

---

**T025 — ReconnectStrategy 指数退避（1.5h）**

```
依赖：无（纯算法类）
产出：client/src/core/net/reconnect_strategy.cpp（完整实现）
验收：
  □ nextDelay() 返回序列：1s, 2s, 4s, 8s, 16s, 30s, 60s（第 7 次及之后固定 60s）
  □ maxRetries() = 10，超过后返回 -1 表示放弃
  □ reset() 重置到初始状态
  □ 单元测试：验证序列正确
```

---

**T026 — HttpClient 基础（2h）**

```
依赖：T002
产出：client/src/core/net/http_client.cpp（完整实现）
验收：
  □ get(url, headers) → 异步，通过信号返回 response(body, statusCode)
  □ post(url, jsonBody, headers) → 同上
  □ 自动注入 Authorization: Bearer <token>（通过 setToken 设置）
  □ 超时 10s → 信号 error(timeout)
  □ 单元测试：Mock Server HTTP 端点验证 GET/POST
```

---

**T027 — ConnectionMgr 外观（2h）**

```
依赖：T023, T026, T025
产出：client/src/core/net/connection_mgr.cpp（完整实现）
验收：
  □ 持有 WsClient 和 HttpClient 实例
  □ setServerConfig(host, port, tls)
  □ connectToServer(token) → 建立 WS 连接 + 配置 HTTP base URL
  □ 连接断开时自动调用 ReconnectStrategy 重连
  □ 重连成功后自动发送 auth 帧
  □ 信号 connectionStateChanged(enum State)
  □ 单元测试：Mock Server 模拟断线→重连流程
```

---

**T028 — Protocol + WS 全路径集成测试（1.5h）**

```
依赖：T021–T027
产出：client/tests/integration/test_ws_protocol.cpp
验收：
  □ 测试用例：客户端登录 → 发送消息 → 收到 ACK → 状态变为 Delivered
  □ 测试用例：服务端推送消息 → 客户端解码 → 去重（重复 seq 丢弃）
  □ 测试用例：心跳超时 → 触发重连 → 重连成功 → 收到 sync_data
```

---

### 第 5 周：HTTP 客户端完善 + RateLimiter

---

**T029 — RateLimiter 令牌桶（1.5h）**

```
依赖：无
产出：client/src/core/net/rate_limiter.cpp（完整实现）
验收：
  □ 构造 RateLimiter(bytesPerSec)
  □ tryConsume(n) → bool（是否有足够令牌）
  □ 内部令牌桶算法：每秒补充 tokens，上限为 burst size（= bytesPerSec）
  □ 线程安全（std::mutex）
  □ 单元测试：快速消耗→拒绝→等待补充→恢复
```

---

**T030 — HttpClient 完善：重试 + 并发控制 + 文件上传（2.5h）**

```
依赖：T026, T029
产出：client/src/core/net/http_client.cpp（更新）
验收：
  □ 网络超时自动重试（最多 3 次），仅幂等请求（GET/PUT）
  □ 同一时刻最多 6 个并发请求（内部队列）
  □ uploadFile(path, url) → POST multipart/form-data，每块 1MB
  □ downloadFile(url, destPath) → GET + Range 分块下载，断点续传
  □ 信号 uploadProgress / downloadProgress(id, progress, speed)
  □ 单元测试：Mock Server 模拟分块上传/下载 + 中途断连恢复
```

---

### 第 6 周：数据层

---

**T031 — SqliteStore 初始化 + 建表（2h）**

```
依赖：T006, T002
产出：client/src/core/data/sqlite_store.cpp（实现 init + 建表）
验收：
  □ init(dbPath) → 打开 SQLite，执行 PRAGMA（WAL 模式/foreign_keys/cache_size）
  □ 自动建表：conversations/messages/contacts/files/tasks/config/drafts
  □ 表结构与 04-数据库设计文档 §2 对齐
  □ 重复 init 不报错（IF NOT EXISTS）
  □ 单元测试：init → 检查 sqlite_master 中表是否存在
```

---

**T032 — SqliteStore 消息 CRUD（3h）**

```
依赖：T031
产出：client/src/core/data/sqlite_store.cpp（更新）
验收：
  □ saveMessage(msg) → INSERT INTO messages
  □ getMessages(convId, sinceSeq, limit) → SELECT ... WHERE conv_id=? AND server_seq>? ORDER BY server_seq LIMIT ?
  □ getLastServerSeq() → SELECT MAX(server_seq) FROM messages
  □ searchMessages(keyword) → SELECT ... WHERE content_body LIKE '%keyword%'
  □ updateMessageStatus(msgId, status)
  □ revokeMessage(msgId) → UPDATE SET revoked=1
  □ 单元测试：CRUD 全路径 + 边界（空表/大量数据）
```

---

**T033 — SqliteStore 会话 CRUD（1.5h）**

```
依赖：T031
产出：client/src/core/data/sqlite_store.cpp（更新）
验收：
  □ upsertConversation(conv) → INSERT OR REPLACE
  □ getConversations(sortBy) → SELECT ... ORDER BY is_pinned DESC, last_msg_time DESC
  □ updateUnreadCount(convId, count)
  □ updateLastReadSeq(convId, seq)
  □ 单元测试
```

---

**T034 — SqliteStore 联系人/文件/配置（1.5h）**

```
依赖：T031
产出：client/src/core/data/sqlite_store.cpp（更新）
验收：
  □ saveContacts(list) → 批量 INSERT OR REPLACE
  □ getContacts() → SELECT ... ORDER BY department_id, display_name
  □ searchContacts(keyword) → LIKE 搜索
  □ saveFileTransfer / getFileTransfers / updateFileTransferProgress
  □ getConfig / setConfig（键值对）
  □ 单元测试
```

---

**T035 — CacheMgr 内存缓存（2h）**

```
依赖：T006
产出：client/src/core/data/cache_mgr.cpp（完整实现）
验收：
  □ getCachedConversations() → unordered_map<convId, Conversation>
  □ updateConversationCache(conv) → 更新或新增
  □ getOnlineStatus(userId) → OnlineStatus，未知返回 Offline
  □ setOnlineStatus(userId, status)
  □ LRU 淘汰：会话缓存上限 50 条，超出淘汰最久未访问
  □ 线程安全（shared_mutex）
  □ 单元测试
```

---

**T036 — SyncMgr 同步引擎（3h）**

```
依赖：T032, T021
产出：client/src/core/data/sync_mgr.cpp（完整实现）
验收：
  □ requestSync(sinceSeq) → 发送 {"type":"sync","payload":{"since_seq":X,"limit":200}}
  □ applySyncData(json) → 解析 sync_data 帧，批量写入 messages + conversations
  □ 处理 has_more → 自动继续请求下一批
  □ applyContactsDelta(json) → 增量更新联系人缓存
  □ 信号 syncCompleted()
  □ 单元测试：Mock Server 返回多批 sync_data，验证全量正确落库
```

---

### 第 7 周：基础设施完整实现

---

**T037 — ThreadPool 完整实现（2h）**

```
依赖：T007
产出：client/src/core/infra/thread_pool.cpp（完整实现）
验收：
  □ 构造时创建 N 个 worker 线程，等待任务
  □ enqueue(F&&, Args...) → 返回 std::future<result>
  □ 析构时安全关闭（stop_=true, notify_all, join 所有线程）
  □ 任务队列有上限（默认 256），超出时 enqueue 阻塞
  □ 单元测试：投递 100 个任务 → 验证全部执行 + 结果正确 + 线程复用
```

---

**T038 — MsgBus 无锁队列（2h）**

```
依赖：T008
产出：client/src/core/infra/msg_bus.cpp（更新，或仅头文件模板实现）
验收：
  □ 模板类 MsgBus<T>
  □ push(T) → 非阻塞写入
  □ tryPop(T&) → 非阻塞读取，返回 bool
  □ size() → 近似大小
  □ 线程安全：至少使用 std::mutex（Phase 2 阶段接受有锁版本；无锁版本 v2 优化）
  □ 单元测试：2 写线程 + 1 读线程，10000 条消息无丢失无重复
```

---

**T039 — Crypto 完整实现（1.5h）**

```
依赖：T008
产出：client/src/core/infra/crypto.cpp（完整实现）
验收：
  □ sha256Hex(std::string) → 使用 QCryptographicHash::Sha256，返回 hex 字符串
  □ sha256File(path) → 分块读取文件（每块 1MB），累计哈希
  □ 单元测试：已知字符串/文件的 SHA-256 预期值对比
```

---

### 第 8 周：核心层集成 + 测试覆盖补充

---

**T040 — 核心层全模块联调（3h）**

```
依赖：T027, T036, T037, T038, T039
产出：client/tests/integration/test_core_pipeline.cpp
验收：
  □ 测试用例：启动 Mock Server → ConnectionMgr 连接 → 发送消息 → 
    收到 ACK → SqliteStore 落库 → CacheMgr 更新会话缓存
  □ 测试用例：Mock Server 推送消息 → Protocol 解码 → SyncMgr 应用 → 
    SqliteStore 写入 → 通知上层（通过信号验证）
  □ 测试用例：模拟断网 → ReconnectStrategy 重连 → SyncMgr 拉取增量
```

---

**T041 — 补充单元测试至 70% 覆盖率（3h）**

```
依赖：T040
产出：client/tests/unit/*.cpp（补充）
验收：
  □ gtest 过滤器运行全部测试，无 FAIL
  □ 重点覆盖：Protocol（所有帧类型编解码）、ReconnectStrategy（所有状态转移）、
    SqliteStore（空表/边界/并发）、RateLimiter（令牌消耗/补充）
  □ 集成测试覆盖核心数据流路径
说明：不强制工具测量覆盖率百分比，以"关键路径 + 边界条件"覆盖为准。
```

---

**T042 — Phase 2 出口检查（1h）**

```
依赖：T041
产出：无新文件
验收：
  □ cmake --preset ci-debug && cmake --build && ctest 全部通过
  □ 所有 TODO 标记已解决或转为 Issue（Gitee）
  □ 与 03-通信协议文档 逐帧类型对照，确认全部实现或明确标注"Phase 4 实现"
```

---

## Phase 3：Go 后端骨架（T043–T060）

> 出口标准：客户端 + Go 后端完整消息流打通。每任务 1–3h。

---

**T043 — JWT 签发与验证（2h）**

```
依赖：T015（server/ 项目就位）
产出：server/internal/handler/auth.go（完整实现）
验收：
  □ POST /api/v1/auth/login → 验证用户名密码 → 返回 access_token + refresh_token + user_id
  □ access_token 有效期 2h，refresh_token 有效期 7d
  □ authMiddleware → 从 Authorization: Bearer <token> 提取并验证 JWT
  □ 验证失败返回 401
  □ 单元测试：go test 覆盖登录成功/密码错误/token 过期
```

---

**T044 — Token 刷新 + 登出（1h）**

```
依赖：T043
产出：server/internal/handler/auth.go（更新）
验收：
  □ POST /api/v1/auth/refresh → 验证 refresh_token → 返回新的 token pair
  □ POST /api/v1/auth/logout → 将 token 加入黑名单（内存即可，或标记 revoked）
```

---

**T045 — 联系人 API（1.5h）**

```
依赖：T043
产出：server/internal/handler/contacts.go（完整实现）
验收：
  □ GET /api/v1/contacts → 分页返回联系人列表（含在线状态）
  □ GET /api/v1/contacts/search?q=keyword → LIKE 搜索
  □ GET /api/v1/departments → 树形部门结构
```

---

**T046 — 会话 API（2h）**

```
依赖：T043
产出：server/internal/handler/conversations.go（完整实现）
验收：
  □ GET /api/v1/conversations → 当前用户的会话列表（按 last_msg_time 倒序）
  □ POST /api/v1/conversations → 创建新会话（私聊或群聊）
  □ GET /api/v1/conversations/:id/messages?since_seq=X&limit=200 → 游标分页
  □ GET /api/v1/conversations/:id/messages/search?q=keyword → 全文搜索
```

---

**T047 — WebSocket Hub 完善（3h）**

```
依赖：T046
产出：server/internal/ws/hub.go（完整重写）
验收：
  □ Hub 内部二级路由：map[convID] → map[userID] → []*Client
  □ SendToConversation(convID, msg) → 只投递给该会话成员
  □ SendToUser(userID, msg) → 投递给该用户的所有在线设备
  □ Register / Unregister 并发安全
  □ readPump：解析 JSON 帧 → 根据 type 分发到对应 handler
  □ writePump：从 Send chan 读取 → WriteMessage
  □ 单元测试：go test 模拟多 client 连接/断开/消息路由
```

---

**T048 — WebSocket 消息处理（3h）**

```
依赖：T047
产出：server/internal/ws/handler.go（新文件）
验收：
  □ 收到 {"type":"msg"} → 校验 seq → 存储到 PG → 回复 ACK → 路由到目标会话
  □ 收到 {"type":"msg_read"} → 更新 conversation_members.last_read_seq → 通知对方
  □ 收到 {"type":"msg_revoke"} → 标记消息 revoked → 广播给会话成员
  □ 收到 {"type":"msg_edit"} → 更新消息 content_body + edited_at → 广播
  □ 收到 {"type":"sync"} → 查询 since_seq 后的消息 → 返回 sync_data
  □ 去重：收到的 seq ≤ last_recv_seq 视为重复，直接回复 ACK 但丢弃
```

---

**T049 — 任务 API（1.5h）**

```
依赖：T043
产出：server/internal/handler/tasks.go（完整实现）
验收：
  □ GET /api/v1/tasks → 按状态筛选（pending/done/rejected）
  □ POST /api/v1/tasks/:id/handle → 处理任务（通过/驳回），推送 notify 给发起方
  □ 任务状态变更通过 WS 实时推送
```

---

**T050 — 文件 API（2.5h）**

```
依赖：T043
产出：server/internal/handler/files.go（完整实现）
验收：
  □ POST /api/v1/files/check → 根据 SHA-256 检查文件是否已存在（秒传判断）
  □ POST /api/v1/files/upload/:upload_id/chunks/:index → 接收分块，写入临时文件
  □ POST /api/v1/files/upload/:upload_id/complete → 校验 SHA-256 → 移动至存储目录 → 返回 URL
  □ GET /api/v1/files/download?url=... → Range 分块下载
```

---

**T051 — 后端全 API 集成测试（3h）**

```
依赖：T043–T050
产出：server/internal/handler/*_test.go（新增/补充）
验收：
  □ go test ./... 全部通过
  □ 测试覆盖：登录→Token→创建会话→WS 收发消息→文件上传/下载→任务处理
  □ 使用 testcontainers-go 或 Docker 启动 PG 实例做集成测试（可选；至少用 mock DB）
```

---

**T052 — Phase 3 出口：前后端首次对接（2h）**

```
依赖：T042, T051
产出：无新文件（验证任务）
验收：
  □ 客户端（yunrong_core）连接 Go 后端
  □ 登录 → 拿到 token → WS 连接 → 发送消息 → 收到 ACK → 消息落库
  □ 两个客户端实例互发消息可收到
  □ Phase 3 出口检查清单通过
```

---

## Phase 4：业务逻辑层（T053–T070）

> 出口标准：IM / 任务 / 文件三大业务闭环，客户端可独立演示核心功能。

---

**T053 — IMService 消息发送全链路（3h）**

```
依赖：T027, T032, T036
产出：client/src/core/service/im_service.cpp（完整实现）
验收：
  □ sendTextMessage(convId, text) → Worker Pool 编码 → ConnectionMgr 发送 → 
    收到 ACK → 更新消息状态为 Delivered → 通知 GUI（信号）
  □ sendImageMessage(convId, imagePath) → 先上传文件 → 拿到 URL → 发消息
  □ 发送失败 → 消息状态 Failed → 支持手动重发
  □ 离线消息排队：发送时网络断开 → 存入 offline_queue → 恢复后自动重发
```

---

**T054 — IMService 消息接收 + 路由（2.5h）**

```
依赖：T053
产出：client/src/core/service/im_service.cpp（更新）
验收：
  □ 收到 WS 推送消息 → Worker Pool 批量解码 → 去重（server_seq）→ SqliteStore 落库 → 
    更新 CacheMgr 会话摘要 → 信号 newMessage(msg)
  □ 收到 msg_read → 更新本地消息状态为 Read
  □ 收到 msg_revoke → 本地消息标记 revoked → 信号 messageRevoked(msgId)
  □ 收到 msg_edit → 更新本地消息 content_body + edited 标记
```

---

**T055 — IMService 会话管理（2h）**

```
依赖：T053, T054
产出：client/src/core/service/im_service.cpp（更新）
验收：
  □ 获取会话列表（优先 CacheMgr，未命中查 SqliteStore）
  □ 未读计数管理：新消息到达时 +1，用户打开会话时清零 + 发送 msg_read
  □ 会话置顶/免打扰（更新本地 + 同步远端）
  □ 创建新会话
```

---

**T056 — FileService 上传编排（2.5h）**

```
依赖：T030
产出：client/src/core/service/file_service.cpp（完整实现）
验收：
  □ uploadFile(filePath) → SHA-256 → 秒传检查 → 分块上传 → 完成后返回 URL
  □ 信号 uploadProgress(id, progress, speed) 实时更新
  □ 暂停/恢复/取消
  □ 断点续传：记录 TransferState → 重启后从 nextChunk 继续
```

---

**T057 — FileService 下载编排（1.5h）**

```
依赖：T056
产出：client/src/core/service/file_service.cpp（更新）
验收：
  □ downloadFile(url, destPath) → Range 分块下载 → SHA-256 校验
  □ 信号 downloadProgress(id, progress, speed)
  □ 暂停/恢复/取消 + 断点续传
```

---

**T058 — TaskService（1.5h）**

```
依赖：T032
产出：client/src/core/service/task_service.cpp（完整实现）
验收：
  □ 从服务端拉取任务列表 → 本地缓存
  □ 按状态筛选：待处理/已处理/我发起的
  □ 处理操作：通过/驳回 → POST /api/v1/tasks/:id/handle
  □ 收到 WS notify 新任务 → 弹窗提醒（通过信号通知 GUI）
```

---

**T059 — ReportService 骨架（1h）**

```
依赖：T032
产出：client/src/core/service/report_service.cpp（骨架实现）
验收：
  □ getMessageStats(days) → 按天聚合消息数（查询本地 SQLite）
  □ getFileStats(days) → 按天聚合传输量
  □ 返回数据结构供 GUI 绘制图表
```

---

**T060 — Phase 4 集成测试 + 出口检查（2h）**

```
依赖：T053–T059
产出：client/tests/integration/test_business_logic.cpp
验收：
  □ 测试：IM 发送→ACK→落库→状态更新全链路
  □ 测试：文件上传→秒传命中→跳过上传
  □ 测试：任务推送→处理→状态同步
  □ Phase 4 出口清单通过
```

---

## Phase 5：GUI（T061–T092）

> 出口标准：完整桌面客户端，全部界面可交互。每任务 1–3h。

---

**T061 — MainWindow 三栏布局（2h）**

```
依赖：T003
产出：client/src/gui/main_window.h, client/src/gui/main_window.cpp
验收：
  □ 左侧导航栏（64px 固定宽度，图标竖排：消息/任务/文件/联系人/设置）
  □ 中间内容区（QStackedWidget 切换视图）
  □ 右侧信息面板（可选显示/隐藏，默认 280px）
  □ 窗口最小尺寸 800×600，默认 1200×800
```

---

**T062 — 全局样式 QSS（1.5h）**

```
依赖：T061
产出：client/resources/style.qss, client/src/app/main.cpp（更新加载 QSS）
验收：
  □ 统一的配色方案（浅色主题）
  □ 导航栏背景色、选中态高亮
  □ 会话列表 hover/selected 状态
  □ QSS 通过 Qt 资源文件 (.qrc) 加载
```

---

**T063 — 登录窗口（2h）**

```
依赖：T062
产出：client/src/gui/login_window.h, client/src/gui/login_window.cpp
验收：
  □ 用户名 + 密码输入框 + 登录按钮
  □ "记住密码"复选框（存入 ConfigManager）
  □ 登录成功 → 保存 token → 切换到主窗口
  □ 登录失败 → 显示错误提示
  □ 自动登录：启动时检查是否有有效 token → 跳过登录界面
```

---

**T064 — ConversationListModel + Delegate（3h）**

```
依赖：T006, T055
产出：
  client/src/gui/models/conversation_list_model.h/.cpp
  client/src/gui/delegates/conversation_delegate.h/.cpp
验收：
  □ Model 实现 QAbstractListModel，数据源来自 IMService 的会话列表
  □ Delegate 绘制：头像（圆形）+ 会话名 + 最后消息预览 + 时间 + 未读角标
  □ 未读角标：红色圆形，数字 >99 显示 "99+"
  □ 置顶会话有视觉区分（浅色背景）
  □ 免打扰会话显示静音图标
```

---

**T065 — 会话列表视图（1.5h）**

```
依赖：T064, T061
产出：client/src/gui/views/conversation_list_view.h/.cpp
验收：
  □ QListView + ConversationListModel + ConversationDelegate
  □ 点击会话 → 切换右侧聊天视图
  □ 右键菜单：置顶/取消置顶、免打扰、删除会话
  □ 实时排序：新消息到达时置顶
```

---

**T066 — MessageListModel + MessageDelegate（4h）**

```
依赖：T006, T054
产出：
  client/src/gui/models/message_list_model.h/.cpp
  client/src/gui/delegates/message_delegate.h/.cpp
验收：
  □ Model 实现 QAbstractListModel，数据源来自 IMService
  □ Delegate 自适应高度（根据文本内容计算）
  □ 文本消息气泡：圆角矩形，自己的靠右（蓝色），对方的靠左（灰色）
  □ 图片消息：缩略图 + 点击查看原图
  □ 文件消息：文件图标 + 文件名 + 大小 + 下载按钮
  □ 系统消息：居中灰色文字（"xxx 撤回了一条消息"）
  □ 消息状态指示：发送中（旋转图标）、已送达（单勾）、已读（双勾蓝）、失败（红色感叹号）
  □ 撤回消息显示 "你撤回了一条消息" / "对方撤回了一条消息"
  □ 编辑过的消息显示 "(已编辑)" 标记
  □ 支持文本选中 + 复制
  □ 右键菜单：复制、引用、撤回（自己的消息且 2 分钟内）
```

---

**T067 — 聊天输入框（2h）**

```
依赖：T066
产出：client/src/gui/widgets/chat_input.h/.cpp
验收：
  □ QTextEdit 多行输入，高度自适应（最小 1 行，最大 5 行）
  □ Enter 发送，Shift+Enter 换行
  □ 发送按钮（右侧）
  □ 表情选择按钮（骨架，Phase 5 不实现表情面板）
  □ 文件/图片发送按钮 → 系统文件对话框 → 触发 FileService
  □ 输入中检测：有内容时发送按钮亮起，为空时灰色
```

---

**T068 — 聊天视图整合（2h）**

```
依赖：T065, T066, T067
产出：client/src/gui/views/chat_view.h/.cpp
验收：
  □ 顶部：会话标题 + 对方在线状态
  □ 中部：消息列表（MessageListView），自动滚动到底部
  □ 底部：聊天输入框
  □ 加载历史消息：滚动到顶部时自动拉取更早的消息（分页）
  □ @提及：输入 @ 弹出联系人选择器（骨架）
```

---

**T069 — ContactTreeModel + Delegate（2h）**

```
依赖：T006
产出：
  client/src/gui/models/contact_tree_model.h/.cpp
  client/src/gui/delegates/contact_delegate.h/.cpp
验收：
  □ Model 实现 QAbstractItemModel，树形结构（部门→人员）
  □ 懒加载：展开部门时才加载子节点
  □ Delegate 绘制：头像 + 姓名 + 部门/职位 + 在线状态圆点
  □ 搜索框：输入关键字实时过滤（QSortFilterProxyModel）
```

---

**T070 — 联系人视图（1h）**

```
依赖：T069, T061
产出：client/src/gui/views/contact_view.h/.cpp
验收：
  □ QTreeView + ContactTreeModel + ContactDelegate
  □ 搜索框在顶部
  □ 双击联系人 → 创建/打开私聊会话 → 切换到聊天视图
```

---

**T071 — TaskListModel + Delegate（2h）**

```
依赖：T006, T058
产出：
  client/src/gui/models/task_list_model.h/.cpp
  client/src/gui/delegates/task_delegate.h/.cpp
验收：
  □ Model 数据源来自 TaskService，支持按状态筛选
  □ Delegate 绘制：任务标题 + 发起人 + 时间 + 优先级标签（红/黄/灰）
  □ 状态筛选按钮：待处理 / 已处理 / 我发起的
  □ 点击任务 → 展开详情（正文 + 操作按钮）
  □ 操作按钮：通过 / 驳回（调用 TaskService::handleTask）
```

---

**T072 — 任务视图（1h）**

```
依赖：T071, T061
产出：client/src/gui/views/task_view.h/.cpp
验收：
  □ QListView + TaskListModel + TaskDelegate
  □ 顶部状态筛选栏
  □ 收到新任务 → 列表顶部插入 + Toast 通知
```

---

**T073 — FilePanel 文件传输面板（2.5h）**

```
依赖：T006, T056, T057
产出：
  client/src/gui/views/file_panel.h/.cpp
  client/src/gui/widgets/speed_chart.h/.cpp
验收：
  □ 上传/下载任务列表（QTableView）：文件名、大小、进度条、速度、
    状态（传输中/暂停/完成/失败）、操作按钮（暂停/恢复/取消/重试）
  □ FileService 的进度信号实时更新进度条和速度
  □ QChartView 绘制实时传输速度曲线（上传 + 下载双线）
  □ 传输历史：已完成的任务保留在列表中，支持清理
```

---

**T074 — 设置窗口（1.5h）**

```
依赖：T005
产出：client/src/gui/views/settings_window.h/.cpp
验收：
  □ 通用：语言（骨架，仅中文）、开机启动
  □ 网络：服务器地址 + 端口配置
  □ 通知：消息通知开关、声音开关
  □ 存储：本地数据路径 + 缓存大小显示 + 清除缓存按钮
  □ 关于：版本号 + Git commit hash
  □ 配置保存到 ConfigManager → 持久化到 JSON 文件
```

---

**T075 — 系统托盘 + Toast 通知（2h）**

```
依赖：T061, T054
产出：
  client/src/gui/system_tray.h/.cpp
  client/src/gui/widgets/toast_notify.h/.cpp
验收：
  □ QSystemTrayIcon：托盘图标 + 右键菜单（显示主窗口/退出）
  □ 收到新消息 → 托盘图标闪烁 + 未读计数角标
  □ Toast 通知：右上角滑入动画，3s 后自动消失
  □ Toast 内容：发送者头像 + 名称 + 消息摘要
  □ 点击 Toast → 打开对应会话
  □ 平台适配：Win 用原生 Toast API（可选），Linux 用 D-Bus Notifications（可选）
```

---

**T076 — 数据统计面板（1.5h）**

```
依赖：T059
产出：client/src/gui/views/dashboard_view.h/.cpp
验收：
  □ 日期范围选择器（本周/本月/自定义）
  □ 消息量趋势曲线（QChartView + QLineSeries）
  □ 活跃时段热力图（简化版：按小时柱状图）
  □ 传输量统计（上传/下载柱状图）  
  □ 数据来源：ReportService
```

---

**T077 — 导航切换 + 全局状态集成（2h）**

```
依赖：T065, T068, T070, T072, T073, T074, T076
产出：client/src/gui/main_window.cpp（更新）
验收：
  □ 点击导航栏图标 → QStackedWidget 切换对应视图
  □ 未读消息总数显示在导航栏"消息"图标上
  □ 待处理任务数显示在"任务"图标上
  □ 正在传输的文件数显示在"文件"图标上
  □ 快捷键：Ctrl+1~5 切换视图，Ctrl+F 搜索
```

---

**T078 — GUI 全流程走查 + 打磨（3h）**

```
依赖：T077
产出：无新文件（修复 + 打磨）
验收：
  □ 登录→会话列表→发消息→收消息→已读→撤回→编辑 全流程无崩溃
  □ 文件上传→进度显示→下载→SHA-256 校验 全流程
  □ 任务推送→列表→处理→状态更新 全流程
  □ 断网→重连→离线消息同步→未读计数恢复 全流程
  □ 窗口缩放/最大化/最小化 无布局错乱
  □ 内存：空闲 <150MB，加载 10 万条消息 <500MB
```

---

## Phase 6：集成联调 + 跨平台打包（T079–T093）

> 出口标准：Windows + Linux 双平台可编译打包，v1.0 可发布。

---

**T079 — 全验收场景逐条走查（3h）**

```
依赖：T078, T052
产出：无新文件
验收：
  □ AC-01：两个客户端互发文本消息，500ms 内收到，状态正确流转
  □ AC-02：飞行模式下发消息 → 恢复网络 → 自动重发成功，无重复
  □ AC-03：上传 500MB 文件 → 中途断网 → 恢复后断点续传 → SHA-256 校验通过
  □ AC-04：24h 持续收发消息 → 内存无泄漏（TODO：实际可能只做 1h 压测）
  □ AC-05：同时对 10 个会话发送 → GUI 无卡顿
  □ AC-06：Windows + Linux 分别编译运行
```

---

**T080 — Bug 修复 + 缓冲（5h）**

```
依赖：T079
产出：按需
验收：
  □ T079 走查中发现的 bug 全部修复
  □ 无已知崩溃路径
```

---

**T081 — Windows 打包（3h）**

```
依赖：T080
产出：dist/yunrong-setup.exe 或 .zip
验收：
  □ windeployqt 自动收集 Qt DLL
  □ 依赖 DLL 完整（Qt6 + OpenSSL + SQLite + VC Runtime）
  □ 安装包可在未装 Qt 的 Windows 10 上启动
  □ 打包脚本：scripts/package_win.bat
```

---

**T082 — Linux 打包（3h）**

```
依赖：T080
产出：dist/YunRong-0.1.0-x86_64.AppImage
验收：
  □ linuxdeployqt 生成 AppImage
  □ 在未装 Qt 的 Ubuntu 20.04 上可启动
  □ 打包脚本：scripts/package_linux.sh
```

---

**T083 — CHANGELOG + 发布文档（1h）**

```
依赖：T081, T082
产出：CHANGELOG.md
验收：
  □ 记录 v1.0 新增功能
  □ 已知问题列表
  □ 部署说明（指向 09-部署与运维文档）
```

---

**T084 — v1.0 发布（0.5h）**

```
依赖：T083
产出：git tag v1.0
验收：
  □ dev → main squash merge
  □ git tag -a v1.0 -m "首次正式发布"
  □ Gitee Release 页面添加 CHANGELOG + 安装包附件
```

---

## 附录 A：任务统计

| Phase  | 任务数    | 预估总工时     | 周数     |
|:------:|:------:|:---------:|:------:|
| 1      | 20     | 30h       | 2      |
| 2      | 22     | 45h       | 6      |
| 3      | 10     | 21h       | 4      |
| 4      | 8      | 16h       | 4      |
| 5      | 18     | 32h       | 7      |
| 6      | 6      | 15h       | 2      |
| **合计** | **84** | **~160h** | **25** |

## 附录 B：任务编号速查

```
Phase 1: T001–T020   工程骨架 + Mock Server
Phase 2: T021–T042   客户端核心层
Phase 3: T043–T052   Go 后端骨架  
Phase 4: T053–T060   业务逻辑层
Phase 5: T061–T078   GUI
Phase 6: T079–T084   集成联调 + 打包
```
