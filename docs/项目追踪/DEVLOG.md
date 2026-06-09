# 开发日志

> 每项任务完成后记录：状态、工时、产出、遇到的问题及解决方案。

---

## Phase 1：工程骨架 + Mock Server

### T001 — CMake 项目能 configure ✅

| 日期 | 状态 | 工时 |
|------|:----:|------|
| 2025-07 | ✅ 完成 | 1h |

**产出**：`client/CMakeLists.txt` `client/CMakePresets.json` `client/cmake/Platform.cmake`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | `CMAKE_MAKE_PROGRAM is not set` | preset 指定 `"Ninja"`，环境无 Ninja | 去掉 `generator` 字段，CMake 自动选 Visual Studio |
| 2 | FetchContent 无法 clone GitHub | 国内网络不通 | nlohmann_json → Gitee 镜像，spdlog → 用户 fork |
| 3 | spdlog Gitee 公共镜像 404 | `mirrors/spdlog` 不存在 | 改用 `https://gitee.com/kiyose408/spdlog.git` |

---

### T002 — 空白窗口可启动 ✅

| 日期 | 状态 | 工时 |
|------|:----:|------|
| 2025-07 | ✅ 完成 | 1h |

**产出**：`client/src/app/main.cpp` `client/src/app/CMakeLists.txt` `client/CMakeLists.txt`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| — | 无 | — | — |

**验收记录**：Qt Creator 编译通过，启动显示标题栏 "Yunrong" 的空白窗口，关闭正常退出。

---

### T003 — 日志可以输出到文件和控制台 ✅

| 日期 | 状态 | 工时 |
|------|:----:|------|
| 2025-07 | ✅ 完成 | 1h |

**产出**：`client/src/app/logger.h` `client/src/app/logger.cpp` `client/src/app/main.cpp`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | `spdlog::default_logger_raw` 编译失败 | spdlog v1.14.1 无此 API | 改为 `default_logger()` → 仍失败（需完整头文件） |
| 2 | `spdlog::default_logger` 编译失败 | logger.h 仅前向声明，宏展开缺 `#include <spdlog/spdlog.h>` | 加回完整头文件 |
| 3 | 日志文件未生成 | `init()` 在 `QApplication` 前调用，`QFile` 可能未就绪；相对路径写到了不确定位置 | `init()` 移到 `QApplication` 之后；路径改为 `applicationDirPath() + "/yunrong.log"`；增加文件打开失败的错误日志 |

**技术决策 — log 方案选型**：

| | spdlog | Qt qInstallMessageHandler |
|---|---|---|
| 捕获范围 | 仅显式调用 | 显式调用 + Qt 框架内部警告 |
| 性能 | 异步百万级/秒 | 同步，数千条/秒 |
| 依赖 | 第三方库 | Qt 自带，零依赖 |
| 本项目适配 | 会丢失 QWebSocket 等模块的内部错误诊断 | 自动拦截所有 Qt 警告 |

**结论**：选 Qt 原生方案。QWebSocket / QNetworkAccessManager 的连接失败、TLS 错误等通过 `qWarning` 输出，`qInstallMessageHandler` 自动捕获到日志文件——这是 spdlog 做不到的。spdlog 保留为项目依赖（Phase 2 异步日志或高性能场景可能用到），但日常日志主路径走 Qt。

---

### T004 — 配置可以从 JSON 文件加载 ✅

| 日期 | 状态 | 工时 |
|------|:----:|------|
| 2025-07 | ✅ 完成 | 1.5h |

**产出**：`client/config/default.json` `client/src/app/config_mgr.h` `client/src/app/config_mgr.cpp`

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | 配置文件找不到 | exe 在 build 深层目录，相对路径 `config/default.json` 无法定位 | `applicationDirPath() + "/../../../../config/default.json"` 回溯到项目根 |
| 2 | `configure_file` + generator expression 失败 | `$<TARGET_FILE_DIR>` 不与 `configure_file` 兼容 | 放弃 CMake 复制方案，改为代码中相对路径回溯 |
| 3 | `qInfo("str")` vs `qInfo() << "str"` 混淆 | 前者是 printf 风格，后者是流式，`LOG_INFO("str")` 宏展开后不能接 `<<` | 统一使用 `LOG_INFO() << "msg"` 流式写法 |

**实现要点**：
- `ConfigManager` 使用 nlohmann/json 的 JSON Pointer 风格路径取值（`/server/host`）
- 通过 `QString::split('/')` 逐级遍历 JSON 树，任意层级缺失时返回 fallback
- `default.json` 不存在时不崩溃，使用硬编码默认值并输出 WARN 日志

---

（后续 Phase 1 任务按实际进展逐项追加）

---

## Phase 2：客户端核心层

（待 Phase 1 完成后启用）

---

## Phase 3：Go 后端

（待启用）

---

## Phase 4：业务逻辑层

（待启用）

---

## Phase 5：GUI

（待启用）

---

## Phase 6：集成联调 + 打包

（待启用）
