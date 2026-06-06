# 任务划分文档

> 增量验证驱动。每项任务从上一项任务的**已验证状态**出发，产出可运行、可观察的增量。
> **禁止**：提前创建后续任务才需要的文件或抽象。**允许**：任务中发现设计问题时回过来修正设计文档。

---

## Phase 1：工程骨架（T001–T012）

> 出口：`yunrong_client` 空白窗口可启动并输出日志；Mock Server 响应 `/health` 并转发单聊消息。

---

**T001 — CMake 项目可 configure（1h）** ✅ 已完成

```
依赖：无
产出：client/CMakeLists.txt  client/CMakePresets.json  client/cmake/Platform.cmake
验收：
  □ Qt Creator 中打开 client/CMakeLists.txt，configure 通过
  □ FetchContent 成功下载 nlohmann_json 和 spdlog
  □ 不含任何 .cpp 或 .h 文件
```

---

**T002 — 空白窗口可启动（1h）**

```
依赖：T001
起点：CMake configure 通过，spdlog 和 nlohmann_json 可链接
产出：client/src/app/main.cpp  client/src/app/CMakeLists.txt  client/src/gui/CMakeLists.txt
      修改 client/CMakeLists.txt（增加 add_subdirectory）
验收：
  □ cmake --build 成功，无编译错误
  □ 启动后显示空白 Qt 窗口（标题 "YunRong"）
  □ 关闭窗口正常退出，无崩溃
说明：暂不创建 core 库。代码直接放在 app/ 中——先运行起来再考虑复用。
```

---

**T003 — 日志输出到文件和控制台（1h）**

```
依赖：T002
起点：空白窗口可启动
产出：client/src/app/logger.h  client/src/app/logger.cpp
验收：
  □ 启动程序，控制台出现 [INFO] Logger initialized
  □ 程序目录下生成 yunrong.log
  □ yunrong.log 内容含启动时间戳
  □ 关闭窗口时日志记录 "Application exiting"
```

---

**T004 — 配置从 JSON 文件加载（1.5h）**

```
依赖：T002
起点：空白窗口可启动
产出：client/src/app/config_mgr.h  client/src/app/config_mgr.cpp
      client/config/default.json
验收：
  □ 启动时读取 config/default.json
  □ 日志输出 server.host 和 server.port 的值
  □ default.json 不存在时不崩溃，日志警告 + 使用默认值
```

---

**T005 — WebSocket 可连接可断开（2h）**

```
依赖：T003, T004
起点：日志和配置可用
产出：client/src/app/ws_client.h  client/src/app/ws_client.cpp
验收：
  □ 启动后自动连接 ws://{host}:{port}/ws
  □ 连接成功时日志输出 "WebSocket connected"
  □ 连接失败（目标不存在/端口错误）时日志输出错误原因，不崩溃
  □ 关闭窗口时正常断开
说明：在 Mock Server 就绪前，可用 T012 的 Mock Server 验证，或接受连接失败作为合法输出。
```

---

**T006 — WebSocket 收发 JSON 文本（2h）**

```
依赖：T005
起点：WebSocket 可连接
产出：修改 ws_client.h/cpp（增加收发接口）
验收：
  □ 发送 {"type":"ping"}，日志输出 "Sent: {\"type\":\"ping\"}"
  □ 收到消息时日志输出完整 JSON 原文
  □ 收到非 JSON 文本时日志输出警告 "Non-JSON message ignored"
  □ 收发不崩溃
```

---

**T007 — 心跳检测断开（1.5h）**

```
依赖：T006
起点：WS 可收发
产出：修改 ws_client.h/cpp（增加定时器和超时检测）
验收：
  □ 每 30s 自动发送 ping
  □ 90s 内未收到任何消息，日志输出 "Heartbeat timeout"
  □ 超时后 ws_client 状态变为 Disconnected
```

---

**T008 — 断线自动重连（1.5h）**

```
依赖：T007
起点：心跳可检测断开
产出：修改 ws_client.h/cpp（增加重连逻辑）
验收：
  □ 断开后自动尝试重连
  □ 重连间隔指数增长：1s → 2s → 4s → 8s → 16s → 30s → 60s（封顶）
  □ 重连成功日志输出 "Reconnected"
  □ 连续 10 次失败后停止，日志输出 "Max reconnect attempts reached"
```

---

**T009 — Mock Server：HTTP 启动和健康检查（2h）**

```
依赖：无（独立 Go 项目，与客户端并行开发）
起点：server/ 目录不存在
产出：server/go.mod  server/cmd/server/main.go
验收：
  □ go run ./cmd/server 启动，日志显示监听端口
  □ curl http://localhost:8080/health → 200 {"status":"ok"}
  □ Ctrl+C 正常退出，日志显示 "shutting down"
```

---

**T010 — Mock Server：用户登录接口（1.5h）**

```
依赖：T009
起点：/health 可访问
产出：修改 main.go（增加 POST /api/v1/auth/login）
验收：
  □ POST {"username":"admin","password":"123456"} → 200 + access_token
  □ 用户名或密码错误 → 401
  □ 返回的 token 可用 jwt.io 解码，含 user_id 和 exp
```

---

**T011 — Mock Server：WebSocket 升级和回声（2h）**

```
依赖：T010
起点：HTTP 登录可用
产出：修改 main.go（增加 GET /ws 路由 + gorilla/websocket upgrader）
验收：
  □ 用 wscat 连接 ws://localhost:8080/ws?token=xxx
  □ 发送 {"type":"ping"} → 收到 {"type":"pong"}
  □ 发送任意 JSON → 收到 {"type":"echo","payload":<原消息>}
  □ 断开时服务端日志输出 "client disconnected"
```

---

**T012 — Mock Server：单聊消息转发（2h）**

```
依赖：T011
起点：WS 回声可用
产出：修改 main.go（增加 Hub + 按 userID 路由）
验收：
  □ 客户端 A（token=user1）和 B（token=user2）同时连接
  □ A 发送 {"type":"msg","to":"user2","body":"hello"}
  □ B 收到 {"type":"msg","from":"user1","body":"hello"}
  □ A 不收到自己发的消息
```

---

## Phase 2：客户端核心能力（T013–T030）

> 出口：客户端可通过 HTTP 登录、通过 WS 收发消息、消息本地 SQLite 落库、离线消息恢复后补拉。

---

### 第一轮：从 app/ 抽离已验证模块

---

**T013 — core 库：迁移 logger 和 config_mgr（1h）**

```
依赖：T003, T004
起点：logger 和 config_mgr 在 app/ 下稳定运行
产出：client/src/core/infra/logger.h/cpp  client/src/core/infra/config_mgr.h/cpp
      client/src/core/CMakeLists.txt（新建 yunrong_core 静态库）
验收：
  □ app/ 中不再有 logger.cpp 和 config_mgr.cpp
  □ app 链接 yunrong_core，编译通过
  □ 程序行为与迁移前一致（日志、配置无变化）
说明：只有被 app 验证过的模块才允许移入 core。未验证的代码不得放入 core。
```

---

**T014 — HTTP 客户端：能调 Mock Server 登录（2h）**

```
依赖：T010（Mock Server 登录可用），T013（core 库可用）
起点：Mock Server 可响应 /auth/login
产出：client/src/core/net/http_client.h/cpp
验收：
  □ 程序启动后调用 POST /api/v1/auth/login
  □ 日志输出服务端返回的 access_token
  □ 网络不通时日志输出错误，不崩溃
  □ token 过期时（Mock Server 可配短过期时间）日志输出 "Token expired"
```

---

### 第二轮：消息收发通路

---

**T015 — 消息帧编解码（1.5h）**

```
依赖：T006（已验证 WS 可收发 JSON）
起点：WS 可收发原始 JSON
产出：client/src/core/net/protocol.h/cpp
验收：
  □ encode(type="msg", payload) 输出合法 JSON 帧，含 ver/type/seq/ts
  □ decode(合法 JSON) 返回解析结果，type 字段正确
  □ decode(非法 JSON) 不崩溃，返回错误
  □ seq 在每次 encode 后自增
```

---

**T016 — WS 客户端接入 Protocol（1h）**

```
依赖：T008（重连可用），T015（Protocol 可用）
起点：WS 可收发原始 JSON，Protocol 可编解码
产出：修改 ws_client.h/cpp（收发改用 Protocol 编解码）
验收：
  □ 发送消息走 Protocol::encode → WS send
  □ 收到消息走 WS receive → Protocol::decode
  □ 无效帧不崩溃，日志输出 "Invalid frame received"
  □ 重连后 seq 从 0 重新开始
```

---

**T017 — 发送消息等待 ACK（2h）**

```
依赖：T012（Mock Server 可转发消息），T016（WS 接入 Protocol）
起点：Mock Server 可转发单聊，客户端 WS 可用 Protocol
产出：修改 ws_client.h/cpp（增加 pending 表 + ACK 超时重传）
验收：
  □ 发送 msg 后不立即标记为"已送达"，等待 ACK
  □ 5s 内收到 ACK → 日志 "Message delivered (seq=42)"
  □ 5s 超时 → 重发（最多 3 次）
  □ 3 次均失败 → 日志 "Message failed (seq=42)"
  □ Mock Server 需回复 ACK（修改 T012 的 Mock Server 增加 ACK 回复）
```

---

**T018 — 收到消息去重（1h）**

```
依赖：T017（发送有 ACK）
起点：发送端有 ACK 机制
产出：修改 ws_client.h/cpp（增加 last_recv_seq 去重）
验收：
  □ 收到 seq≤last_recv_seq 的消息 → 丢弃 + 日志 "Duplicate message seq=xxx"
  □ 收到 seq>last_recv_seq 的消息 → 正常处理 + 更新 last_recv_seq
  □ 用 Mock Server 模拟重复推送验证
```

---

### 第三轮：本地持久化

---

**T019 — SQLite 初始化（1.5h）**

```
依赖：T013（core 库可用）
起点：core 库可链接 Qt6::Sql
产出：client/src/core/data/sqlite_store.h/cpp
验收：
  □ 程序启动时自动创建 yunrong.db（若不存在）
  □ 自动执行建表 SQL（conversations / messages / contacts / file_transfers / config）
  □ 日志输出 "SQLite opened: yunrong.db"
  □ 第二次启动不重复建表（IF NOT EXISTS）
  □ .db 文件已在 .gitignore 中
```

---

**T020 — 消息写入 SQLite（1.5h）**

```
依赖：T018（收消息有去重），T019（SQLite 就绪）
起点：收消息可去重，SQLite 已建表
产出：修改 sqlite_store.h/cpp（增加 insertMessage）
验收：
  □ 收到一条消息后，调用 insertMessage 写入 messages 表
  □ 日志输出 "Message stored: msg_id=xxx"
  □ 重复 msg_id 写入时不崩溃（UNIQUE 约束）
  □ 停掉程序，用 sqlite3 命令行检查 yunrong.db 中有刚才的消息
```

---

**T021 — 消息按会话查询（1h）**

```
依赖：T020（消息可写入）
起点：messages 表有数据
产出：修改 sqlite_store.h/cpp（增加 queryMessages）
验收：
  □ queryMessages(conv_id, limit=20, before_seq=X) 返回 ≤20 条消息
  □ 结果按 timestamp DESC 排列
  □ 空会话返回空列表，不崩溃
```

---

### 第四轮：会话管理与同步

---

**T022 — 会话列表（1.5h）**

```
依赖：T021（消息可查询）
起点：可按会话查消息
产出：修改 sqlite_store.h/cpp（增加 conversations 表 CRUD）
      client/src/core/data/sync_mgr.h/cpp（新建）
验收：
  □ 收到新消息时自动更新对应会话的 last_msg_preview 和 last_msg_time
  □ 新会话自动插入 conversations 表
  □ sync_mgr 维护内存中的会话列表，按 last_msg_time 降序
  □ 日志输出 "Conversation list updated: N conversations"
```

---

**T023 — 未读计数（1h）**

```
依赖：T022（会话列表可用）
起点：会话列表可维护
产出：修改 sync_mgr.h/cpp（增加未读计数逻辑）
      conversations 表增加 unread_count 字段
验收：
  □ 非当前会话的新消息使 unread_count +1
  □ 当前会话的新消息不增加未读计数
  □ 切换到某会话时 unread_count 清零
  □ 日志输出 "Unread: conv=2001 count=5"
```

---

**T024 — 离线消息补拉（2h）**

```
依赖：T017（ACK 机制可用），T022（会话列表可用）
起点：发送/接收链路完整
产出：修改 ws_client.h/cpp（重连后发送 sync 帧）
      修改 Mock Server（增加 sync_data 响应）
验收：
  □ 客户端重连后自动发送 {"type":"sync","since_seq":<本地最大seq>}
  □ Mock Server 返回 since_seq 之后的所有消息
  □ 客户端收到后逐条写入 SQLite + 更新会话列表
  □ 日志输出 "Sync complete: N messages received"
```

---

**T025 — 已读回执（1.5h）**

```
依赖：T023（未读计数可用），T024（同步可用）
起点：未读计数和同步就绪
产出：修改 ws_client.h/cpp（增加 msg_read 帧发送）
      修改 Mock Server（转发 msg_read 给对方）
验收：
  □ 用户打开会话时发送 {"type":"msg_read","conv_id":X,"read_to_seq":N}
  □ 对方收到后日志输出 "Messages read: conv=X up_to_seq=N"
  □ Mock Server 记录 last_read_seq
```

---

### 第五轮：文件传输

---

**T026 — HTTP 文件上传（分块）（2.5h）**

```
依赖：T014（HTTP 客户端可用）
起点：HTTP POST 可用
产出：修改 http_client.h/cpp（增加 multipart + 分块上传接口）
      修改 Mock Server（增加 POST /api/v1/files/upload 路由，保存到临时目录）
验收：
  □ 选择一个 5MB 测试文件，分 5 个 1MB 块上传
  □ 每块上传后日志输出进度百分比
  □ 全部上传完成后 Mock Server 返回 {"url":"/files/xxx"}
  □ Mock Server 的临时目录中存在完整文件，SHA-256 与原文件一致
```

---

**T027 — HTTP 文件下载（Range）（2h）**

```
依赖：T026（上传可用）
起点：文件已上传到 Mock Server
产出：修改 http_client.h/cpp（增加 Range 下载接口）
验收：
  □ 下载已上传的文件，分 4 个并发 Range 下载
  □ 每完成一个 Range 日志输出进度
  □ 下载完成后 SHA-256 与原文件一致
  □ 中途取消下载不崩溃
```

---

**T028 — 断点续传（1.5h）**

```
依赖：T026, T027
起点：上传/下载通路可用
产出：client/src/core/data/file_transfer_state.h/cpp（新建）
验收：
  □ 上传/下载时每完成一个块就写入 TransferState 到 SQLite
  □ 程序崩溃后重启，读取 TransferState 从未完成的块继续
  □ 不重复传输已完成的块
  □ 传输完成后清理 TransferState
```

---

**T029 — 文件秒传（1h）**

```
依赖：T026（上传可用）
起点：文件可上传
产出：修改 http_client.h/cpp（上传前发 file_check 请求）
      修改 Mock Server（增加 POST /api/v1/files/check）
验收：
  □ 上传前先 POST /api/v1/files/check {"hash":"sha256:xxx"}
  □ Mock Server 返回 {"exists":true} → 跳过上传，日志 "File exists, skip upload"
  □ Mock Server 返回 {"exists":false} → 正常上传
```

---

### 第六轮：缓冲

---

**T030 — 客户端端到端走查 + DEVLOG 补录（2h）**

```
依赖：T001–T029 全部完成
验收：
  □ 按以下场景逐条走查：登录→连 WS→收发消息→看 SQLite 有数据→
    断网→消息发不出去→恢复网络→自动重连→补拉离线消息→文件上传→
    文件下载→文件秒传
  □ 每条场景日志输出符合预期
  □ 发现的 bug 记录到 DEVLOG 并修复
  □ DEVLOG Phase 1/2 补全
```

---

## Phase 3：Go 后端正式实现（T031–T048）

> 出口：Mock Server 被真实 Go 后端替代；所有 API 对接 PostgreSQL；WebSocket Hub 完整。

---

**T031 — Go 项目骨架 + PostgreSQL 连接池（1.5h）**

```
依赖：T009（Mock Server 结构可参考）
起点：server/ 下有 Mock Server（将其移到 mock/ 保留）
产出：server/cmd/server/main.go  server/internal/store/postgres.go
      server/internal/store/migrations/001_initial_schema.sql
验收：
  □ go run ./cmd/server 启动，成功连接 PostgreSQL
  □ 自动执行 migration 建表
  □ 日志输出 "Connected to PostgreSQL, migrations applied"
  □ /health 返回 200
```

---

**T032 — 用户注册和登录（JWT）（2h）**

```
依赖：T031（DB 可用）
起点：PostgreSQL 可用，users 表已建
产出：server/internal/handler/auth.go
验收：
  □ POST /api/v1/auth/register → 写入 users 表 → 返回 token
  □ POST /api/v1/auth/login → 验证密码 → 返回 token
  □ 密码用 bcrypt 哈希存储
  □ 错误密码 3 次后锁定账户 5 分钟（安全措施）
```

---

**T033 — 认证中间件（1h）**

```
依赖：T032（登录可用）
起点：JWT 签发可用
产出：server/internal/handler/middleware.go
验收：
  □ 带有效 token 的请求通过
  □ 无效/过期 token → 401
  □ 缺失 Authorization header → 401
  □ 通过中间件后 c.Get("user_id") 可获取用户 ID
```

---

**T034 — 联系人列表 API（1.5h）**

```
依赖：T033（认证可用）
起点：users 表有数据
产出：server/internal/handler/contacts.go
验收：
  □ GET /api/v1/contacts → 分页返回所有用户
  □ GET /api/v1/contacts?search=张三 → 模糊搜索
  □ 每页默认 50 条，支持 page 参数
```

---

**T035 — 部门树 API（1h）**

```
依赖：T033, T034（认证 + contacts 模式可参考）
起点：departments 表已建
产出：server/internal/handler/departments.go
验收：
  □ GET /api/v1/departments → 返回完整部门树（嵌套 JSON）
  □ 叶子部门无 children 字段
```

---

**T036 — WebSocket Hub（替代 Mock Server 的 Hub）（2h）**

```
依赖：T033（认证可用），T012（Mock Server Hub 逻辑可参考）
起点：JWT 中间件可用
产出：server/internal/ws/hub.go  server/internal/ws/client.go
验收：
  □ GET /ws?token=xxx → 升级为 WebSocket
  □ 无效 token → 拒绝升级
  □ 同一 userID 可从多设备同时连接
  □ 客户端断开时 Hub 自动清理
```

---

**T037 — 消息收发（WS 通路 + PostgreSQL 落库）（2.5h）**

```
依赖：T036（Hub 可用）
起点：WebSocket Hub 就绪
产出：server/internal/handler/messages.go（WS 消息处理）
验收：
  □ 客户端 A 发 msg → 服务端写入 messages 表 → 转发给 B
  □ 服务端回复 ACK
  □ B 不在线时消息持久化，B 上线后通过 sync 补拉
  □ 服务端为每条消息分配全局递增 server_seq
```

---

**T038 — 会话列表 API（1.5h）**

```
依赖：T037（消息可用）
起点：messages 表有数据，conversation_members 表已建
产出：server/internal/handler/conversations.go
验收：
  □ GET /api/v1/conversations → 返回当前用户的会话列表
  □ 每个会话含 last_msg_preview, last_msg_time, unread_count
  □ 按 last_msg_time 降序
```

---

**T039 — 消息历史 API（1h）**

```
依赖：T037, T038
起点：消息可收发，会话可用
产出：修改 conversations.go（增加消息拉取接口）
验收：
  □ GET /api/v1/conversations/:id/messages?before_seq=100&limit=20
  □ 返回 ≤20 条消息，按 server_seq DESC
  □ 传入 before_seq=0 返回最新 20 条
```

---

**T040 — 消息搜索 API（1.5h）**

```
依赖：T037（消息可用）
起点：PostgreSQL 全文搜索可用
产出：修改 conversations.go（增加搜索接口）
验收：
  □ GET /api/v1/conversations/:id/messages/search?q=会议
  □ 返回匹配的消息列表，高亮关键词
  □ 空结果返回空数组
```

---

**T041 — 离线同步 API（2h）**

```
依赖：T037（server_seq 可用），T024（客户端 sync 逻辑可用）
起点：全局 server_seq 就绪
产出：修改 messages.go（增加 sync_data 响应）
验收：
  □ 客户端发送 {"type":"sync","since_seq":42}
  □ 服务端返回 server_seq > 42 的所有消息（最多 200 条）
  □ has_more=true 时客户端可继续 sync
  □ 无新消息时返回空数组 + has_more=false
```

---

**T042 — 已读回执（服务端）（1h）**

```
依赖：T041（同步可用），T025（客户端 msg_read 可用）
起点：sync 机制就绪
产出：修改 messages.go（增加 msg_read 处理）
验收：
  □ 收到 msg_read → 更新 conversation_members.last_read_seq
  □ 转发 msg_read 给对方客户端
```

---

**T043 — 文件上传 API（2h）**

```
依赖：T033（认证可用）
起点：files 表已建
产出：server/internal/handler/files.go
验收：
  □ POST /api/v1/files/check → 返回 exists + upload_id
  □ POST /api/v1/files/upload/:upload_id/chunks/:idx → 保存到磁盘
  □ POST /api/v1/files/upload/:upload_id/complete → SHA-256 校验 + 标记完成
  □ 上传完成后返回可访问的 URL
```

---

**T044 — 文件下载 API（1h）**

```
依赖：T043（上传可用）
起点：文件已存储
产出：修改 files.go（增加下载接口）
验收：
  □ GET /api/v1/files/download?file_id=X → 返回文件流
  □ 支持 Range 头，返回 206 Partial Content
  □ 不存在文件返回 404
```

---

**T045 — 任务推送和列表 API（2h）**

```
依赖：T037（消息通路可用）
起点：tasks 表已建
产出：server/internal/handler/tasks.go
验收：
  □ POST /api/v1/tasks（管理员）→ 创建任务 → WS 推送给目标用户
  □ GET /api/v1/tasks → 返回当前用户的任务列表
  □ GET /api/v1/tasks?status=pending → 按状态筛选
  □ POST /api/v1/tasks/:id/handle → 处理任务 → WS 通知发起人
```

---

**T046 — 偏好设置 API（1h）**

```
依赖：T033（认证可用）
起点：user_preferences 表已建
产出：server/internal/handler/preferences.go
验收：
  □ GET /api/v1/preferences → 返回当前用户的偏好 JSON
  □ PUT /api/v1/preferences → 覆盖写入
  □ 不存在的键返回默认值
```

---

**T047 — 后端集成测试（2h）**

```
依赖：T031–T046
起点：全部 API 就绪
验收：
  □ 用 Go test 编写集成测试：注册→登录→发消息→查消息→上传文件→下载文件
  □ 全部测试通过
  □ go test ./... 覆盖率 ≥ 60%
```

---

**T048 — 后端缓冲 + DEVLOG（1.5h）**

```
依赖：T047
验收：
  □ 修复发现的 bug
  □ DEVLOG Phase 3 补全
  □ 设计文档与实际实现不一致的地方回写文档
```

---

## Phase 4：业务逻辑收敛（T049–T058）

> 出口：客户端三大业务（IM / 文件 / 任务）通过 service 层调用，不直接操作 net/ 和 data/。

---

**T049 — IMService：收敛消息收发逻辑（2h）**

```
依赖：T018（收发可用），T021（查询可用），T023（未读可用）
起点：app 中直接调用 ws_client 和 sqlite_store
产出：client/src/core/service/im_service.h/cpp
验收：
  □ sendTextMessage(conv_id, text) 内部调 ws_client + 更新 SQLite
  □ 收到消息时 im_service 回调 → 更新会话列表 + 未读计数
  □ app 中不再直接调 ws_client 的收发接口
  □ 已有功能（收发/去重/ACK/重传/已读）行为不变
```

---

**T050 — FileService：收敛文件传输逻辑（2h）**

```
依赖：T026–T029（文件传输可用）
起点：app 中直接调用 http_client 做文件传输
产出：client/src/core/service/file_service.h/cpp
验收：
  □ uploadFile(path) 内部调 http_client 分块上传 + 进度回调
  □ downloadFile(url, path) 内部调 http_client Range 下载 + 进度回调
  □ app 中不再直接调 http_client 的文件接口
  □ 已有功能（分块/续传/秒传）行为不变
```

---

**T051 — TaskService：收敛任务逻辑（1.5h）**

```
依赖：T045（任务 API 可用）
起点：任务 API 可调
产出：client/src/core/service/task_service.h/cpp
验收：
  □ 从服务端拉取任务列表
  □ 处理任务（通过/驳回）调 REST API
  □ 任务状态变更通知通过 im_service 接收
```

---

**T052 — SyncManager 完善：增量同步 + 冲突处理（2h）**

```
依赖：T024（离线补拉可用），T049（IMService 可用）
起点：sync 基本可用
产出：修改 sync_mgr.h/cpp（完善同步引擎）
验收：
  □ 上线后自动 sync 所有会话的增量消息
  □ 离线期间的操作队列（发送/已读/撤回）在重连后按序重放
  □ 服务端冲突（如同一消息被编辑两次）以 server_seq 大者为准
```

---

**T053 — 消息撤回（1.5h）**

```
依赖：T049（IMService 可用）
起点：IM 收发可用
产出：修改 im_service.h/cpp（增加 revokeMessage）
      conversations.go（服务端 msg_revoke 处理）
验收：
  □ 发送方在 2 分钟内可撤回
  □ 撤回后双方客户端消息气泡变为"已撤回"
  □ 超时撤回返回错误
  □ 服务端消息 content_body 标记 revoked=true
```

---

**T054 — 消息编辑（1h）**

```
依赖：T049（IMService 可用）
起点：IM 收发可用
产出：修改 im_service.h/cpp（增加 editMessage）
验收：
  □ 发送方可编辑自己的文本消息
  □ 编辑后消息气泡显示"(已编辑)"标记
  □ 图片/文件消息不可编辑
```

---

**T055 — 联系人缓存（1.5h）**

```
依赖：T034（联系人 API 可用），T019（SQLite 可用）
起点：服务端可返回联系人列表
产出：修改 sqlite_store.h/cpp（增加 contacts 表 CRUD）
      client/src/core/data/cache_mgr.h/cpp（新建联系人缓存）
验收：
  □ 首次登录后从服务端拉取全部联系人，写入本地 SQLite
  □ 后续启动优先读本地，后台静默更新
  □ 搜索联系人时优先搜本地（即时响应），后台补充服务端搜索结果
```

---

**T056 — 业务层集成走查（2h）**

```
依赖：T049–T055
验收：
  □ 走查 IM 全流程：登录→同步→收发→撤回→编辑→已读
  □ 走查文件全流程：上传→进度→下载→续传→秒传
  □ 走查任务全流程：接收通知→列表→处理→结果通知
  □ bug 修复 + DEVLOG 补全
```

---

**T057 — 抽离 model 头文件（0.5h）**

```
依赖：T056（业务层稳定）
起点：各模块的数据结构分散在 service/ 和 data/ 中
产出：client/src/core/model/message.h conversation.h contact.h task.h file_transfer.h
验收：
  □ 5 个头文件包含各自的数据结构定义（从 service/data 中提取）
  □ 编译通过
  □ 不修改任何业务逻辑
说明：model 是纯数据结构，从已验证的代码中提取——不是提前设计。
```

---

**T058 — 缓冲 + DEVLOG（1h）**

```
依赖：T057
验收：
  □ 修复走查中发现的 bug
  □ DEVLOG Phase 4 补全
```

---

## Phase 5：GUI（T059–T076）

> 出口：完整可交互的桌面客户端。每个视图独立任务、独立验收。

---

**T059 — 主窗口三栏布局（2h）**

```
依赖：T002（空白窗口），T022（会话列表数据可用）
起点：空白窗口可启动，后台数据层完整
产出：client/src/gui/main_window.h/cpp
验收：
  □ 左侧导航栏（64px 宽，图标竖排）
  □ 中间内容区（默认显示会话列表）
  □ 右侧信息面板（可选显示/隐藏）
  □ 窗口最小宽度 800px，可拖拽调整
```

---

**T060 — 导航栏切换视图（1.5h）**

```
依赖：T059（主窗口就绪）
起点：三栏布局可用
产出：修改 main_window.h/cpp（增加 QStackedWidget）
验收：
  □ 点击"消息"图标 → 内容区显示会话列表
  □ 点击"联系人"图标 → 内容区显示占位文本"联系人"
  □ 点击"任务"图标 → 内容区显示占位文本"任务"
  □ 点击"文件"图标 → 内容区显示占位文本"文件"
  □ 点击"设置"图标 → 内容区显示占位文本"设置"
  □ 当前选中图标高亮
```

---

**T061 — 登录窗口（2h）**

```
依赖：T014（HTTP 登录可用），T059（主窗口就绪）
起点：HTTP 登录通路可用
产出：client/src/gui/login_window.h/cpp
验收：
  □ 输入用户名+密码 → 点击登录 → 调 HTTP /auth/login
  □ 登录成功 → 关闭登录窗口 → 显示主窗口
  □ 登录失败 → 显示错误提示（红色文字）
  □ "记住密码"复选框 → 勾选后下次自动填入
  □ 回车键触发登录
```

---

**T062 — 会话列表视图（2.5h）**

```
依赖：T022（会话列表数据可用），T059（主窗口就绪）
起点：sync_mgr 维护会话列表
产出：client/src/gui/views/conversation_list.h/cpp
验收：
  □ QListView 展示会话列表
  □ 每项显示：头像（占位圆形）、会话名、最后消息预览、时间
  □ 有未读消息的会话显示红色角标（数字）
  □ 置顶会话显示在图钉图标
  □ 点击某会话 → 日志输出 "Selected conversation: id=xxx"
```

---

**T063 — 消息气泡控件（2.5h）**

```
依赖：T062（会话列表就绪）
起点：会话列表可展示
产出：client/src/gui/widgets/message_bubble.h/cpp
验收：
  □ 自己的消息靠右对齐，蓝色背景
  □ 对方的消息靠左对齐，灰色背景
  □ 文本消息自适应高度（支持多行）
  □ 显示发送时间（HH:mm）
  □ 显示消息状态图标（发送中/已送达/已读/失败）
```

---

**T064 — 聊天视图（2.5h）**

```
依赖：T063（消息气泡可用），T049（IMService 可用），T062（会话列表可用）
起点：消息气泡控件就绪，IM 收发可用
产出：client/src/gui/views/chat_view.h/cpp
验收：
  □ 点击会话列表中的会话 → 内容区切换为聊天视图
  □ 消息列表显示历史消息气泡（从 SQLite 加载最近 50 条）
  □ 底部输入框 + 发送按钮
  □ 输入文本 → 回车或点击发送 → 消息气泡出现 + 发送中状态
  □ 收到新消息 → 自动追加到列表底部 + 滚动到底
  □ 收到已读回执 → 消息状态更新为"已读"
```

---

**T065 — 联系人树（2h）**

```
依赖：T035（部门树 API 可用），T055（联系人缓存可用），T060（导航可用）
起点：联系人数据可用
产出：client/src/gui/views/contact_tree.h/cpp
验收：
  □ QTreeView 展示部门→人员树形结构
  □ 点击部门节点展开/折叠
  □ 顶部搜索框 → 输入关键词 → 过滤树中匹配的人员
  □ 搜索结果高亮显示
  □ 点击人员 → 日志输出 "Contact selected: id=xxx"
```

---

**T066 — 联系人详情 → 发起单聊（1.5h）**

```
依赖：T065（联系人树可用），T064（聊天视图可用）
起点：联系人可选，聊天视图就绪
产出：修改 contact_tree.h/cpp（增加双击处理）
验收：
  □ 双击联系人 → 创建/切换到该联系人的单聊会话
  □ 会话不存在时自动创建（调 REST API + 本地插入）
  □ 切换到聊天视图，显示该会话的消息历史
```

---

**T067 — 任务列表视图（2h）**

```
依赖：T051（TaskService 可用），T060（导航可用）
起点：任务数据可用
产出：client/src/gui/views/task_list.h/cpp
验收：
  □ QTableView 展示任务列表
  □ 列：标题、发起人、优先级、时间、状态
  □ 顶部筛选栏：待处理 / 已处理 / 我发起的
  □ 点击某任务 → 日志输出 "Task selected: id=xxx"
```

---

**T068 — 任务处理对话框（1.5h）**

```
依赖：T067（任务列表可用）
起点：任务可展示
产出：client/src/gui/widgets/task_dialog.h/cpp
验收：
  □ 双击任务列表中的任务 → 弹出处理对话框
  □ 显示任务详情（标题、正文、发起人、时间）
  □ "通过"按钮 → 调 REST API → 关闭对话框 → 任务状态变为"已处理"
  □ "驳回"按钮 → 同上
  □ 操作结果日志输出
```

---

**T069 — 文件传输面板（2h）**

```
依赖：T050（FileService 可用），T060（导航可用）
起点：文件传输逻辑可用
产出：client/src/gui/views/file_panel.h/cpp
验收：
  □ QTableView 展示传输任务列表
  □ 列：文件名、大小、进度条、速度、状态
  □ 支持"添加文件"按钮 → 打开文件选择对话框 → 开始上传
  □ 上传/下载进度实时更新
```

---

**T070 — 传输速度曲线（1.5h）**

```
依赖：T069（文件面板可用）
起点：文件面板可显示传输任务
产出：client/src/gui/widgets/speed_chart.h/cpp
验收：
  □ QChartView 实时绘制传输速度曲线（KB/s）
  □ 多条传输任务各自一条曲线，颜色不同
  □ X 轴为时间（最近 60 秒），自动滚动
  □ 鼠标悬停显示数值
```

---

**T071 — 设置窗口（1.5h）**

```
依赖：T004（配置可用），T046（偏好 API 可用），T060（导航可用）
起点：配置和偏好可读写
产出：client/src/gui/views/settings_window.h/cpp
验收：
  □ 通用设置：语言、主题（亮/暗切换预览）
  □ 网络设置：服务器地址、端口、TLS 开关
  □ 通知设置：桌面通知开关、声音开关
  □ 修改后点击保存 → 写入本地 config + 同步服务端偏好 API
```

---

**T072 — 系统托盘（1.5h）**

```
依赖：T023（未读计数可用），T064（聊天可用）
起点：IM 功能完整，未读计数可用
产出：修改 main_window.h/cpp（增加 QSystemTrayIcon）
验收：
  □ 最小化到托盘而不是关闭
  □ 托盘图标显示未读消息数角标（数字或红点）
  □ 新消息到达时托盘图标闪烁
  □ 右键菜单：显示主窗口 / 退出
  □ 双击托盘图标恢复主窗口
```

---

**T073 — Toast 通知弹窗（1.5h）**

```
依赖：T064（聊天可用），T072（托盘可用）
起点：消息接收可用
产出：client/src/gui/widgets/toast_notify.h/cpp
验收：
  □ 新消息到达且主窗口不在前台 → 右上角弹出 Toast
  □ Toast 显示发送者头像（占位）、名称、消息预览
  □ 3 秒后自动消失，带滑出动画
  □ 点击 Toast → 打开对应会话
```

---

**T074 — @提及高亮和提醒（1h）**

```
依赖：T064（聊天可用），T073（Toast 可用）
起点：聊天和通知可用
产出：修改 chat_view 和 toast_notify
验收：
  □ 收到含 @当前用户名 的消息 → 消息气泡高亮（黄色边框）
  □ 群聊免打扰模式下，@我的消息仍触发 Toast 通知
  □ 会话列表中 @我的会话显示 [有人@我] 标记
```

---

**T075 — GUI 集成走查（3h）**

```
依赖：T059–T074
验收：
  □ 走查完整交互流：
    启动→登录窗口→输入账号密码→登录成功→主窗口→
    会话列表→选会话→发消息→收到回复→气泡状态变化→
    切到联系人→搜索人员→双击发起单聊→
    切到任务→查看任务→处理任务→
    切到文件→上传文件→看进度→
    最小化到托盘→新消息→托盘闪烁+Toast→
    双击托盘→恢复窗口→切到设置→改主题→保存→
    关闭→退出
  □ 无崩溃路径
  □ 内存无持续增长（24 小时运行观察）
  □ bug 修复 + DEVLOG 补全
```

---

**T076 — 跨平台编译验证（2h）**

```
依赖：T075（GUI 走查通过）
起点：Windows 上功能完整
验收：
  □ 在 Linux（Ubuntu 22.04+）上 cmake --preset release 编译通过
  □ 启动后主窗口正常显示
  □ 基本收发消息可用
  □ Linux 平台差异问题记录到 DEVLOG
```

---

## Phase 6：集成联调 + 发布（T077–T084）

> 出口：可打包分发的 v1.0 版本。

---

**T077 — 客户端 + 后端全链路压力测试（2h）**

```
依赖：T075（客户端 OK），T047（后端 OK）
起点：两端独立测试通过
验收：
  □ 5 个客户端同时在线，每个每秒发 10 条消息，持续 5 分钟
  □ 消息到达率 ≥ 99%
  □ 服务端内存无泄漏
  □ 延迟 p99 < 500ms
```

---

**T078 — 24 小时稳定性测试（1h）**

```
依赖：T077
验收：
  □ 客户端 + 服务端同时运行 24 小时
  □ 每小时自动发送一条心跳消息
  □ 内存使用稳定（无持续增长）
  □ 数据库文件大小不异常膨胀
```

---

**T079 — 异常场景测试（2h）**

```
依赖：T077
验收：
  □ 拔网线 30 秒 → 客户端检测断开 → 自动重连 → 补拉消息
  □ 服务端崩溃重启 → 客户端重连 → 恢复正常
  □ 客户端崩溃重启 → 恢复未发送的消息 → 未完成的文件传输续传
  □ 发送超大文件（2GB） → 分块传输 → SHA-256 校验通过
  □ 发送消息后立即断网 → 重连后消息发出 → 不重复
```

---

**T080 — 端到端测试脚本（2h）**

```
依赖：T079（异常场景手动验证通过）
产出：tests/e2e/test_basic_chat.py（或 Go test）
验收：
  □ 脚本自动启动 Mock/真实服务端
  □ 自动启动 2 个客户端进程（headless 模式或 CLI 模式）
  □ 自动执行：登录 → 收发消息 → 验证 DB → 断连 → 重连 → 补拉
  □ CI 可运行（cmake --preset ci-debug && ctest）
```

---

**T081 — 文档终审（1.5h）**

```
依赖：全部任务
验收：
  □ 每份设计文档末尾修订记录补全到 v0.3
  □ 接口文档与实际 API 一致（逐条核对）
  □ README 更新快速开始步骤（可直接复制粘贴运行）
  □ DEVLOG 全 Phase 补全
```

---

**T082 — Windows 打包（2h）**

```
依赖：T080（测试通过）
起点：Release 构建可用
产出：packages/yunrong-1.0.0-win64.exe
验收：
  □ windeployqt 收集 Qt DLL
  □ NSIS/Inno Setup 生成安装包
  □ 在纯净 Windows 10 虚拟机上安装 → 启动 → 登录 → 收发消息
```

---

**T083 — Linux 打包（1.5h）**

```
依赖：T076（Linux 编译通过）
起点：Linux 上 Release 构建可用
产出：packages/yunrong-1.0.0-x86_64.AppImage
验收：
  □ linuxdeployqt 生成 AppImage
  □ 在 Ubuntu 22.04 上双击运行 → 登录 → 收发消息
```

---

**T084 — v1.0 发布（1h）**

```
依赖：T081, T082, T083
产出：Git tag v1.0 + Gitee Release 页面
验收：
  □ dev → main 合并
  □ git tag v1.0 + git push --tags
  □ Gitee Release 上传 Windows 安装包 + Linux AppImage
  □ CHANGELOG.md 列出 v1.0 功能清单
```

---

## 任务统计

| Phase     | 任务数    | 预估总工时    |
|:---------:|:------:|:--------:|
| 1 — 工程骨架  | 12     | 20h      |
| 2 — 客户端核心 | 18     | 32h      |
| 3 — Go 后端 | 18     | 28h      |
| 4 — 业务收敛  | 10     | 14h      |
| 5 — GUI   | 18     | 33h      |
| 6 — 集成发布  | 8      | 13h      |
| **合计**    | **84** | **140h** |

预估 140 小时 ÷ 每周 20 小时 = **7 周**。含缓冲约 **8–9 周**。

---

## 验收原则

1. 每个验收条件必须可被客观判定——看到什么输出、运行什么命令、检查什么结果
2. 不检查本任务不涉及的功能
3. 依赖项未就绪时不启动任务——先完成依赖，再做本体
4. 任务执行中发现设计问题时，先修正设计文档再继续编码
