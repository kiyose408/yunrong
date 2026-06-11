# 云融（YunRong）— 企业协作终端

跨平台企业办公协作桌面客户端，以即时通讯为核心，融合任务通知与文件传输。

> **当前阶段**：Phase 1 完成，Phase 2 进行中。详见 [开发路线](#开发路线)。

## 技术栈

| 层 | 技术 |
|----|------|
| 桌面 GUI | Qt 6 Widgets (C++17) |
| 客户端网络 | QWebSocket + QNetworkAccessManager |
| 客户端存储 | SQLite 3 |
| 后端服务 | Go (gorilla/websocket) |
| 后端存储 | PostgreSQL 14+ |
| 构建 | CMake 3.20 + Go modules |

## 仓库结构

```
yunrong/
├── client/                # C++ Qt 桌面客户端
│   ├── CMakeLists.txt
│   ├── CMakePresets.json
│   ├── cmake/
│   ├── config/
│   └── src/app/           # 应用入口 + 日志/配置/WS 客户端
├── server/                # Go Mock Server
│   └── cmd/server/
├── docs/
│   ├── 设计文档/          # 全部设计文档（00–09）
│   └── 项目追踪/          # ROADMAP / TASKS / DEVLOG / ENGINEERING
├── .gitignore
└── README.md
```

## 设计文档索引

按阅读顺序排列：

| 编号 | 文档 | 内容 |
|:----:|------|------|
| 00 | [设计思路](docs/设计文档/00-设计思路文档.md) | 技术选型理由、架构哲学、设计原则 |
| 01 | [需求分析](docs/设计文档/01-需求分析文档.md) | 功能需求、非功能需求、验收标准 |
| 02 | [系统架构设计](docs/设计文档/02-系统架构设计文档.md) | 四层架构、线程模型、数据流、ADR |
| 03 | [通信协议详细设计](docs/设计文档/03-通信协议详细设计文档.md) | JSON 帧协议、二进制帧、ACK 机制、心跳 |
| 04 | [数据库设计](docs/设计文档/04-数据库设计文档.md) | SQLite 本地 + PostgreSQL 远程双存储 |
| 05 | [模块详细设计](docs/设计文档/05-模块详细设计文档.md) | 类接口、时序图、线程交互 |
| 06 | [接口设计](docs/设计文档/06-接口设计文档.md) | REST API + WebSocket API 完整定义 |
| 07 | [GUI 设计](docs/设计文档/07-GUI设计文档.md) | 窗口布局、控件清单、数据绑定 |
| 08 | [测试计划](docs/设计文档/08-测试计划文档.md) | 测试金字塔、用例、Mock Server |
| 09 | [部署与运维](docs/设计文档/09-部署与运维文档.md) | 构建指南、环境依赖、打包 |

## 开发路线

| Phase | 内容 | 状态 |
|:-----:|------|:----:|
| 1 | 工程骨架 + Mock Server | ✅ v0.1 |
| 2 | 客户端核心层（net / data / infra） | 🟡 进行中 |
| 3 | Go 后端骨架 | ⬜ |
| 4 | 业务逻辑层 | ⬜ |
| 5 | GUI | ⬜ |
| 6 | 集成联调 + 跨平台打包 | ⬜ |

### Phase 1 成果（T001–T012）

| 模块 | 能力 |
|------|------|
| 客户端骨架 | CMake + 空白窗口 + 日志双输出 + JSON 配置加载 |
| WebSocket 客户端 | 连接/断开 + JSON 收发 + 30s 心跳 + 指数退避重连（1s→60s） |
| Mock Server | /health + JWT 登录 + WebSocket 回声 + Hub 单聊路由 |

## 快速开始

### 客户端

```bash
cd client
cmake --preset debug
cmake --build --preset debug
```

Qt Creator 中打开 `client/CMakeLists.txt`，选择 debug preset 构建运行。

### Mock Server

```bash
cd server
go run ./cmd/server
```

启动后访问 `http://localhost:8080/health`，或用浏览器控制台连接 WebSocket。

## 工程规范

分支策略、commit 格式、合并自查清单见 [ENGINEERING.md](docs/项目追踪/ENGINEERING.md)。
