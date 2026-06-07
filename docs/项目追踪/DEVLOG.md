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

### T003 — 日志可以输出到文件和控制台

| 日期 | 状态 | 工时 |
|------|:----:|------|
| — | ⬜ 待开始 | 1h |

---

### T004 — 配置可以从 JSON 文件加载

| 日期 | 状态 | 工时 |
|------|:----:|------|
| — | ⬜ 待开始 | 1.5h |

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
