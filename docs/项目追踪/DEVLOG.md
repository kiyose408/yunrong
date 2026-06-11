# 开发日�?
> 每项任务完成后记录：状态、工时、产出、遇到的问题及解决方案�?
---

## Phase 1：工程骨�?+ Mock Server

### T001 �?CMake 项目�?configure �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`client/CMakeLists.txt` `client/CMakePresets.json` `client/cmake/Platform.cmake`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | `CMAKE_MAKE_PROGRAM is not set` | preset 指定 `"Ninja"`，环境无 Ninja | 去掉 `generator` 字段，CMake 自动�?Visual Studio |
| 2 | FetchContent 无法 clone GitHub | 国内网络不�?| nlohmann_json �?Gitee 镜像，spdlog �?用户 fork |
| 3 | spdlog Gitee 公共镜像 404 | `mirrors/spdlog` 不存�?| 改用 `https://gitee.com/kiyose408/spdlog.git` |

---

### T002 �?空白窗口可启�?�?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`client/src/app/main.cpp` `client/src/app/CMakeLists.txt` `client/CMakeLists.txt`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| �?| �?| �?| �?|

**验收记录**：Qt Creator 编译通过，启动显示标题栏 "Yunrong" 的空白窗口，关闭正常退出�?
---

### T003 �?日志可以输出到文件和控制�?�?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`client/src/app/logger.h` `client/src/app/logger.cpp` `client/src/app/main.cpp`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | `spdlog::default_logger_raw` 编译失败 | spdlog v1.14.1 无此 API | 改为 `default_logger()` �?仍失败（需完整头文件） |
| 2 | `spdlog::default_logger` 编译失败 | logger.h 仅前向声明，宏展开�?`#include <spdlog/spdlog.h>` | 加回完整头文�?|
| 3 | 日志文件未生�?| `init()` �?`QApplication` 前调用，`QFile` 可能未就绪；相对路径写到了不确定位置 | `init()` 移到 `QApplication` 之后；路径改�?`applicationDirPath() + "/yunrong.log"`；增加文件打开失败的错误日�?|

**技术决�?�?log 方案选型**�?
| | spdlog | Qt qInstallMessageHandler |
|---|---|---|
| 捕获范围 | 仅显式调�?| 显式调用 + Qt 框架内部警告 |
| 性能 | 异步百万�?�?| 同步，数千条/�?|
| 依赖 | 第三方库 | Qt 自带，零依赖 |
| 本项目适配 | 会丢�?QWebSocket 等模块的内部错误诊断 | 自动拦截所�?Qt 警告 |

**结论**：�?Qt 原生方案。QWebSocket / QNetworkAccessManager 的连接失败、TLS 错误等通过 `qWarning` 输出，`qInstallMessageHandler` 自动捕获到日志文件——这�?spdlog 做不到的。spdlog 保留为项目依赖（Phase 2 异步日志或高性能场景可能用到），但日常日志主路径�?Qt�?
---

### T004 �?配置可以�?JSON 文件加载 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1.5h |

**产出**：`client/config/default.json` `client/src/app/config_mgr.h` `client/src/app/config_mgr.cpp`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | 配置文件找不�?| exe �?build 深层目录，相对路�?`config/default.json` 无法定位 | `applicationDirPath() + "/../../../../config/default.json"` 回溯到项目根 |
| 2 | `configure_file` + generator expression 失败 | `$<TARGET_FILE_DIR>` 不与 `configure_file` 兼容 | 放弃 CMake 复制方案，改为代码中相对路径回溯 |
| 3 | `qInfo("str")` vs `qInfo() << "str"` 混淆 | 前者是 printf 风格，后者是流式，`LOG_INFO("str")` 宏展开后不能接 `<<` | 统一使用 `LOG_INFO() << "msg"` 流式写法 |

**实现要点**�?- `ConfigManager` 使用 nlohmann/json �?JSON Pointer 风格路径取值（`/server/host`�?- 通过 `QString::split('/')` 逐级遍历 JSON 树，任意层级缺失时返�?fallback
- `default.json` 不存在时不崩溃，使用硬编码默认值并输出 WARN 日志

---

### T005 �?WebSocket 能连接和断开 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`client/src/app/ws_client.h` `client/src/app/ws_client.cpp`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | 6 �?`LNK2001` 未解析符�?| Qt MOC 未处�?`ws_client.h` 中的 `Q_OBJECT` �?| 顶层 `CMakeLists.txt` 显式开�?`CMAKE_AUTOMOC ON`；`ws_client.h` 加入 `add_executable` 源列�?|

**验收记录**：Mock Server 未建，连�?`ws://127.0.0.1:8080/ws` 失败，日志输�?`WebSocket error: "Invalid socket descriptor"`，程序正常退出不崩溃�?
---

### T006 �?WebSocket 收发 JSON 消息 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`client/src/app/ws_client.h` `client/src/app/ws_client.cpp`（增�?`sendJson` / `onTextMessage` / `messageReceived`�?
| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| �?| �?| �?| �?|

**验收记录**：无 Mock Server 情况下发�?`{"type":"ping"}`，日志输�?`WS send: {"type":"ping"}`，连接被拒后正常退出�?
---

### T007 �?心跳：连接断开能检测到 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 0.5h |

**产出**：`client/src/app/ws_client.h` `client/src/app/ws_client.cpp`（增�?`m_heartbeatTimer` / `m_lastActivity` / `onHeartbeatTick()`�?
| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| �?| �?| �?| �?|

**实现要点**：`onConnected` 启动 30s 定时器；`onTextMessage` 刷新活动时间；`onHeartbeatTick` 发�?ping + 检�?90s 超时。Mock Server 就位后完整验证�?
---

### T008 �?断线自动重连 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 0.5h |

**产出**：`client/src/app/ws_client.h` `client/src/app/ws_client.cpp`（增�?`m_reconnectTimer` / `onReconnectTick()` / `reconnectDelayMs()`�?
| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| �?| �?| �?| �?|

**实现要点**：`onDisconnected` 中区分手动关�?vs 意外断开，后者启动指数退避重连；`reconnectDelayMs()` = `min(1000×2^attempt, 60000)`；`onConnected` 清零计数器�?
---

### T009 �?Mock Server HTTP /health �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 0.5h |

**产出**：`server/go.mod` `server/cmd/server/main.go`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| �?| �?| �?| �?|

**验证方式**：`go run ./cmd/server` �?另开终端 `curl http://localhost:8080/health` �?`{"status":"ok"}`，Ctrl+C 正常退出�?
---

### T010 �?Mock Server 用户登录 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`server/cmd/server/main.go`（增�?`loginHandler` / `makeJWT` 标准库手�?JWT�?
| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| �?| �?| �?| �?|

**验证结果**：`admin/123456` �?200 + valid JWT；`admin/wrong` �?401。JWT 可用 jwt.io 解码验证�?
---

### T011 �?Mock Server WebSocket 回声 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1h |

**产出**：`server/cmd/server/main.go`（增�?`wsHandler` + gorilla/websocket upgrader�?
| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | `go mod tidy` 无法下载 gorilla/websocket | Go 默认 proxy.golang.org 被墙 | `go env -w GOPROXY=https://goproxy.cn,direct` 切国内代�?|

**验证结果**：浏览器 `new WebSocket("ws://localhost:8080/ws")` �?ping→pong、任�?JSON→echo。客户端断开时服务端日志输出 `ws client disconnected`�?
---

### T012 �?Mock Server 单聊消息转发 �?
| 日期 | 状�?| 工时 |
|------|:----:|------|
| 2026-06 | �?完成 | 1.5h |

**产出**：`server/cmd/server/main.go`（增�?`Hub` / `Client` / `parseTokenUserID` / `sendToUser`�?
| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | 端口被旧进程占用 `bind: already in use` | 前一任务启动�?go run 未退�?| `netstat -ano | findstr 8080` 找到 PID �?`taskkill` |
| 2 | 消息走了 default echo 而非 msg 路由 | 浏览器标签页混用了不�?token 的旧连接 | 重启服务�?+ 两个全新标签页分别用 admin/zhangsan token 重连 |

**验证结果**：admin (1001) �?`{"type":"msg","to":1002,"body":"hello"}` �?zhangsan (1002) 收到 `{"type":"msg","from":1001,"body":"hello"}`。服务端日志：`msg routed: 1001 �?1002`�?
---

## 🎉 Phase 1 完成

| T# | 模块 | 能力 |
|:--:|------|------|
| T001–T004 | 客户端骨�?| CMake + 空白窗口 + 日志 + 配置 |
| T005–T008 | WebSocket | 连接/断开 + JSON 收发 + 30s 心跳 + 指数退避重�?|
| T009–T012 | Mock Server | /health + JWT 登录 + WS 回声 + Hub 单聊路由 |

**Phase 1 出口**：客户端 + Mock Server 均已可编译运行，WS 单聊消息流走通�?
---

## Phase 2：客户端核心�?
（待 Phase 1 完成后启用）

---

## Phase 3：Go 后端

（待启用�?
---

## Phase 4：业务逻辑�?
（待启用�?
---

## Phase 5：GUI

（待启用�?
---

## Phase 6：集成联�?+ 打包

（待启用�?
