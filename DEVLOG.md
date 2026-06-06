# 开发日志

> 记录每项任务的完成状态、遇到的问题及解决方案。按 Phase 分组，按 T 编号排序。

---

## Phase 1：工程骨架 + Mock Server

### T001 — CMake 顶层项目结构

| 项目 | 内容 |
|---|---|
| **日期** | 2025-07 |
| **状态** | ✅ 完成 |
| **工时** | 1h |
| **产出** | `client/CMakeLists.txt` `client/CMakePresets.json` `client/cmake/Platform.cmake` |

**问题记录**

| # | 问题 | 原因 | 解决 |
|---|---|---|---|
| 1 | `cmake --preset debug` 报 `CMAKE_MAKE_PROGRAM is not set` | `generator: "Ninja"` 但沙箱环境未安装 Ninja | 去掉 preset 中的 `generator` 字段，让 CMake 自动选择默认生成器（Windows 下自动选 Visual Studio） |
| 2 | FetchContent `nlohmann_json` clone 失败 `Failed to connect to github.com port 443` | 国内网络无法直连 GitHub | 切换为 Gitee 镜像 `https://gitee.com/mirrors/nlohmann-json.git` |
| 3 | FetchContent `spdlog` clone 失败 `404 not found` | Gitee 公共镜像 `mirrors/spdlog` 不存在 | 用户自行 fork 到 `https://gitee.com/kiyose408/spdlog.git`，更新 `GIT_REPOSITORY` |

---

### T002 — 核心库 CMake 骨架

| 项目 | 内容 |
|---|---|
| **日期** | — |
| **状态** | ⬜ 待开始 |
| **工时** | 0.5h |

**问题记录**

| # | 问题 | 原因 | 解决 |
|---|---|---|---|

---

### T003 — GUI + App CMake 骨架

| 项目 | 内容 |
|---|---|
| **日期** | — |
| **状态** | ⬜ 待开始 |
| **工时** | 0.5h |

**问题记录**

| # | 问题 | 原因 | 解决 |
|---|---|---|---|

---

## Phase 2：客户端核心层

（待 Phase 1 完成后启用）

---

## Phase 3：Go 后端骨架

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
