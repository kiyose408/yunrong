# 07 — 云融（YunRong）· GUI 设计文档

> **前置文档**：[02-系统架构设计文档](02-系统架构设计文档.md)、[05-模块详细设计文档](05-模块详细设计文档.md)
> **设计思路**：[00-设计思路文档](00-设计思路文档.md)
>
> **版本**：v0.1
> **状态**：已发布
> **最后更新**：2025-01

---

## 1. 设计概述

### 1.1 设计原则

| 原则 | 说明 |
|------|------|
| **非阻塞 UI** | 任何可能超过 50ms 的操作不得在主线程执行 |
| **即时反馈** | 用户操作后 100ms 内必须有视觉反馈（骨架屏/动画/临时状态） |
| **渐进展示** | 默认展示最常用信息，详细内容按需展开 |
| **平台一致** | 遵循各平台 HIG（Windows 11 Fluent / Linux GNOME Adwaita），而非强行统一 |
| **键盘可达** | 所有功能可通过键盘操作（Tab 导航 + 快捷键） |
| **响应式** | 窗口宽度 ≥ 800px 时三栏，< 800px 时自动折叠为单栏 + 导航 |

### 1.2 设计尺度

| 参数 | 值 | 说明 |
|------|-----|------|
| 默认窗口尺寸 | 1200 × 800 px | 最小 800 × 600 |
| 导航栏宽度 | 64 px | 固定，图标竖排 |
| 侧边栏宽度 | 280 px (默认) | 可拖拽调整 (200–400 px) |
| 信息面板宽度 | 320 px (默认) | 可折叠 |
| 字体 | 系统默认 (Segoe UI / Noto Sans) | 基础大小 14px |
| 圆角半径 | 8px (控件) / 12px (气泡) | 统一圆角体系 |
| 间距单位 | 4px 基础网格 | 所有间距为 4 的倍数 |

---

## 2. 主窗口布局

### 2.1 三段式布局

```
┌──────┬──────────────────────────┬─────────────┐
│ Nav  │     Content Area          │  Info Panel │
│ Bar  │                          │  (可折叠)    │
│ 64px │                          │  320px      │
│      │                          │             │
│  ●   │  ┌────────────────────┐  │  联系人详情   │
│ 消   │  │                    │  │  文件预览     │
│ 息   │  │                    │  │  搜索面板     │
│      │  │                    │  │             │
│  ●   │  │   当前视图内容       │  │             │
│ 任   │  │                    │  │             │
│ 务   │  │                    │  │             │
│      │  │                    │  │             │
│  ●   │  │                    │  │             │
│ 文   │  └────────────────────┘  │             │
│ 件   │                          │             │
│      │                          │             │
│  ●   │                          │             │
│ 联   │                          │             │
│ 系   │                          │             │
│ 人   │                          │             │
│      │                          │             │
│  ●   │                          │             │
│ 设   │                          │             │
│ 置   │                          │             │
└──────┴──────────────────────────┴─────────────┘
```

### 2.2 导航栏设计

```
┌──────┐
│      │  ← 用户头像 (32×32, 圆形)
│  👤  │
│      │
│  💬  │  ← 消息 (当前选中: 高亮左侧蓝色竖条)
│  ┃   │
│      │
│  ✅  │  ← 任务 (有未读数: 红色角标 "3")
│  ③   │
│      │
│  📁  │  ← 文件
│      │
│  👥  │  ← 联系人
│      │
│  ⚙   │  ← 设置
│      │
│      │
│      │  ← 弹性空间
│      │
│  ⚡   │  ← 网络状态指示 (绿/黄/红)
│      │
└──────┘
```

导航项数据结构：

```cpp
struct NavItem {
    QString icon;           // SVG 图标路径
    QString label;          // 工具提示文本
    Page page;              // 对应页面枚举
    int unreadCount = 0;    // 未读角标数
    bool isActive = false;
};
```

### 2.3 响应式断点

| 窗口宽度 | 布局 |
|:--------:|------|
| ≥ 1200 px | 三栏：导航 + 侧边栏 + 内容 + 信息面板 |
| 900–1199 px | 两栏：导航 + 侧边栏 + 内容（信息面板折叠） |
| 700–899 px | 两栏：导航 + 内容（侧边栏折叠为弹出菜单） |
| < 700 px | 单栏：内容全屏 + 底部导航条 |

```cpp
void MainWindow::resizeEvent(QResizeEvent* event) {
    int w = event->size().width();
    if (w >= 1200 && layoutMode_ != LayoutMode::ThreePanel) {
        setLayoutMode(LayoutMode::ThreePanel);
    } else if (w >= 900 && w < 1200 && layoutMode_ != LayoutMode::TwoPanel) {
        setLayoutMode(LayoutMode::TwoPanel);
    } else if (w >= 700 && w < 900 && layoutMode_ != LayoutMode::Compact) {
        setLayoutMode(LayoutMode::Compact);
    } else if (w < 700 && layoutMode_ != LayoutMode::Single) {
        setLayoutMode(LayoutMode::Single);
    }
    QMainWindow::resizeEvent(event);
}
```

---

## 3. 核心视图设计

### 3.1 聊天视图 (ChatView)

```
┌──────────────────────────────────────────┐
│  会话标题栏                               │
│  ┌────────────────────────────────────┐  │
│  │ 👤 李四                            │  │
│  │    在线 · 研发部                    │  │
│  │                          🔍 📞 ⋯  │  │
│  └────────────────────────────────────┘  │
│                                          │
│  消息列表 (QListView)                     │
│  ┌────────────────────────────────────┐  │
│  │         ┌──────────────────┐       │  │
│  │  09:30  │ 系统消息: 你们已加 │       │  │
│  │         └──────────────────┘       │  │
│  │                                    │  │
│  │  ┌─────────────────────┐           │  │
│  │  │ 你好，明天的会议几点？│  对方     │  │
│  │  └─────────────────────┘           │  │
│  │                                    │  │
│  │               ┌──────────────────┐ │  │
│  │     我        │ 下午 3 点，3 号   │ │  │
│  │               │ 会议室           │ │  │
│  │               └──────────────────┘ │  │
│  │                          ✓ 已读    │  │
│  │                                    │  │
│  │  ┌─────────────────────┐           │  │
│  │  │ 好的，我会准时参加    │  对方     │  │
│  │  └─────────────────────┘           │  │
│  └────────────────────────────────────┘  │
│                                          │
│  输入区域                                 │
│  ┌────────────────────────────────────┐  │
│  │ 工具栏: 😊 📎 🖼️ 📁  ...          │  │
│  ├────────────────────────────────────┤  │
│  │                                    │  │
│  │  输入消息...                        │  │
│  │                                    │  │
│  ├────────────────────────────────────┤  │
│  │                          发送  ↵   │  │
│  └────────────────────────────────────┘  │
└──────────────────────────────────────────┘
```

#### 消息气泡规格

```
我方气泡 (右对齐, 绿色 #95EC69):
  ┌──────────────────────┐
  │ 消息文本              │
  │ 最大宽度 400px        │
  │                      │
  │              ✓ 09:30 │  ← 状态图标 + 时间
  └──────────────────────┘
  
  圆角: 左上12px, 左下12px, 右上12px, 右下4px
  内边距: 10px 14px
  字体: 14px, 行高 1.5

对方气泡 (左对齐, 白色 #FFFFFF):
  ┌──────────────────────┐
  │ 消息文本              │
  │ 最大宽度 400px        │
  │                      │
  │ 09:30                │
  └──────────────────────┘

  圆角: 左上12px, 右上12px, 右下12px, 左下4px
  内边距: 10px 14px
  阴影: 0 1px 3px rgba(0,0,0,0.08)
```

#### 文件卡片规格

```
┌─────────────────────────────────────┐
│ 📄 Q3季度报告.pdf                    │
│     1.0 MB                         │
│ ┌─────────────────────────────┐    │
│ │████████████████░░░░░░░░░░░░░│ 45%│
│ └─────────────────────────────┘    │
│                          ⬇ 下载    │
└─────────────────────────────────────┘
```

### 3.2 会话列表 (ConversationList)

```
┌──────────────────────────────────┐
│ 🔍 搜索会话或消息...              │
├──────────────────────────────────┤
│                                  │
│ ┌──────────────────────────────┐ │
│ │ 👤 李四                09:30 │ │
│ │    好的，我会准时参加         │ │
│ │                        ③     │ │  ← 未读角标
│ └──────────────────────────────┘ │
│                                  │
│ ┌──────────────────────────────┐ │
│ │ 👥 项目讨论组           09:15 │ │
│ │    张三: 文档已更新到共享      │ │
│ │    [免打扰] 🔕                │ │
│ └──────────────────────────────┘ │
│                                  │
│ ┌──────────────────────────────┐ │
│ │ 👤 王五                昨天   │ │
│ │    [图片]                    │ │
│ │                         ✓    │ │  ← 已读
│ └──────────────────────────────┘ │
│                                  │
│  ─── 置顶 ────────────────────── │
│                                  │
│ ┌──────────────────────────────┐ │
│ │ 📌 全员公告群           周一   │ │
│ │    [2条公告] 新年放假通知     │ │
│ └──────────────────────────────┘ │
└──────────────────────────────────┘
```

每行高度：68px，含头像(40×40) + 名称(14px bold) + 摘要(12px) + 时间(11px)。

### 3.3 联系人视图 (ContactTree)

```
┌──────────────────────────────────┐
│ 🔍 搜索联系人...                  │
├──────────────────────────────────┤
│                                  │
│  ⭐ 星标联系人                    │
│  ├─ 👤 李四    研发部    🟢 在线  │
│  └─ 👤 赵六    产品部    🟡 离开  │
│                                  │
│  🏢 总公司 (1500人)               │
│  ├─ 📁 研发部 (80人)              │
│  │  ├─ 👤 张三    高级工程师 🟢   │
│  │  ├─ 👤 李四    高级工程师 🟢   │
│  │  └─ 👤 孙七    实习生     ⚫   │
│  ├─ 📁 产品部 (45人)              │
│  │  ├─ 👤 赵六    产品经理   🟡   │
│  │  └─ 👤 周八    UI设计    🟢   │
│  └─ 📁 市场部 (30人)              │
│     └─ ...                       │
│                                  │
└──────────────────────────────────┘
```

树节点高度 40px，缩进 20px/级。部门节点前有展开/折叠箭头。

状态指示灯：
- 🟢 在线 (绿色 #4CAF50)
- 🟡 离开 (黄色 #FF9800)
- 🔴 忙碌 (红色 #F44336)
- ⚫ 离线 (灰色 #9E9E9E)

### 3.4 任务视图 (TaskList)

```
┌──────────────────────────────────┐
│  [待处理] [已处理] [我发起的]      │
├──────────────────────────────────┤
│                                  │
│  ┌──────────────────────────────┐│
│  │ 🔴 高优先级                   ││
│  │ 请假审批                      ││
│  │ 张三申请年假 3 天              ││
│  │ 2025-01-20 → 2025-01-22      ││
│  │ 1 小时前                     ││
│  │          [驳回]  [同意]       ││
│  └──────────────────────────────┘│
│                                  │
│  ┌──────────────────────────────┐│
│  │ 🟡 普通优先级                 ││
│  │ 代码评审                      ││
│  │ 李四邀请你评审 PR #234         ││
│  │ 3 小时前                     ││
│  │          [拒绝]  [接受]       ││
│  └──────────────────────────────┘│
│                                  │
│  ┌──────────────────────────────┐│
│  │ 📢 系统公告                   ││
│  │ 服务器维护通知                 ││
│  │ 本周六 02:00-06:00 维护       ││
│  │ 2 天前 · 已读                ││
│  └──────────────────────────────┘│
└──────────────────────────────────┘
```

### 3.5 文件传输面板 (FilePanel)

```
┌──────────────────────────────────┐
│  传输中 (2) · 已完成 (48)         │
├──────────────────────────────────┤
│                                  │
│  上传                            │
│  ┌──────────────────────────────┐│
│  │ 📄 Q3季度报告.pdf             ││
│  │    1.0 MB                    ││
│  │ ████████████░░░░░░ 78%       ││
│  │    812 KB/s           ⏸ 取消 ││
│  └──────────────────────────────┘│
│                                  │
│  下载                            │
│  ┌──────────────────────────────┐│
│  │ 🎨 设计稿_v3.fig              ││
│  │    24.5 MB                   ││
│  │ ████░░░░░░░░░░░░░░ 22%       ││
│  │    1.2 MB/s           ⏸ 取消 ││
│  └──────────────────────────────┘│
│                                  │
│  ─── 已完成 ──────────────────── │
│  ├─ 📸 screenshot.png   2.1 MB  │
│  ├─ 📄 需求文档.docx    156 KB  │
│  └─ 🎵 录音.amr          48 KB  │
└──────────────────────────────────┘
```

### 3.6 数据统计面板 (Dashboard)

```
┌──────────────────────────────────┐
│  消息统计   本周 ◀ ▶              │
├──────────────────────────────────┤
│                                  │
│  总发送: 1,523    总接收: 2,105  │
│                                  │
│  📈 消息趋势 (7天)                │
│  ┌────────────────────────────┐  │
│  │     ·                      │  │
│  │    / \    ·                │  │
│  │   /   \  / \      ·       │  │
│  │  /     \/   \    / \      │  │
│  │ /            \  /   \     │  │
│  │·              \/     ·    │  │
│  └────────────────────────────┘  │
│  ── 发送  ── 接收                │
│                                  │
│  📊 会话活跃度                    │
│  ┌────────────────────────────┐  │
│  │ 项目讨论组 ████████████ 850 │  │
│  │ 李四       ██████ 300     │  │
│  │ 王五       ████ 200       │  │
│  │ 全员公告群  ███ 150        │  │
│  └────────────────────────────┘  │
│                                  │
│  📁 文件传输                      │
│  上传: 45 个 · 500 MB            │
│  下载: 23 个 · 100 MB            │
│                                  │
│  [导出 CSV] [导出 PDF] [导出 JSON]│
└──────────────────────────────────┘
```

---

## 4. 自定义控件规格

### 4.1 MessageBubble (消息气泡)

```cpp
// gui/widgets/message_bubble.h
class MessageBubble : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool isMine READ isMine WRITE setIsMine)

public:
    explicit MessageBubble(QWidget* parent = nullptr);

    void setMessage(const model::Message& msg);
    void setStatus(MessageStatus status);  // Sending/Sent/Delivered/Read/Failed

    bool isMine() const;
    void setIsMine(bool mine);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QString text_;
    QString imageUrl_;
    int contentType_ = 0;
    MessageStatus status_ = MessageStatus::Sent;
    bool isMine_ = false;
    bool isSelected_ = false;

    void paintTextBubble(QPainter* p, const QRect& rect);
    void paintImageBubble(QPainter* p, const QRect& rect);
    void paintFileCard(QPainter* p, const QRect& rect);
    QRect bubbleRect() const;
    QColor bubbleColor() const;
};
```

绘制流程：

```
paintEvent:
  1. 计算气泡矩形 (bubbleRect)
     - 我方: 右对齐, 右侧距边 12px
     - 对方: 左对齐, 左侧距边 48px (头像占位)
  2. 绘制气泡背景
     - QPainterPath 圆角矩形
     - 我方: 填充 #95EC69
     - 对方: 填充 #FFFFFF, 描边 #E0E0E0
  3. 根据 contentType 绘制内容
     - 文本: QPainter::drawText (支持多行、Emoji)
     - 图片: QPainter::drawPixmap (带圆角裁剪)
     - 文件: 文件卡片 (图标 + 名称 + 大小 + 进度条)
  4. 绘制时间和状态标记
     - 我方: 气泡右下角
     - 对方: 气泡左下角
```

### 4.2 AvatarWidget (头像)

```cpp
class AvatarWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int size READ size WRITE setSize)

public:
    explicit AvatarWidget(QWidget* parent = nullptr);

    void setImage(const QPixmap& pixmap);
    void setImageUrl(const QString& url);        // 异步加载
    void setPlaceholder(const QString& initials); // 无头像时显示首字母
    void setOnlineStatus(OnlineStatus status);

    int size() const;
    void setSize(int size);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap image_;
    QString initials_;
    OnlineStatus status_ = OnlineStatus::Offline;
    int size_ = 40;

    void paintCircle(QPainter* p);
    void paintInitials(QPainter* p);
    void paintStatusDot(QPainter* p);
};
```

规格：
- 圆形裁剪（`QPainterPath::addEllipse` + `setClipPath`）
- 默认尺寸：列表 40px，详情 64px，导航栏 32px
- 状态角标：头像右下角 10px 圆点 + 2px 白色描边

### 4.3 SpeedChart (实时速度曲线)

```cpp
class SpeedChart : public QWidget {
    Q_OBJECT

public:
    explicit SpeedChart(QWidget* parent = nullptr);

    void addDataPoint(double upSpeed, double downSpeed); // KB/s
    void setHistoryDuration(int seconds);                 // 默认 60s
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct DataPoint {
        qint64 timestamp;
        double upSpeed;
        double downSpeed;
    };
    std::deque<DataPoint> history_; // 保留最近 ~600 个采样点 (≈60s @10Hz)，超限时 pop_front
    QColor upColor_   = QColor("#4CAF50");
    QColor downColor_ = QColor("#2196F3");

    void paintGrid(QPainter* p, const QRect& rect);
    void paintCurves(QPainter* p, const QRect& rect);
    void paintLegend(QPainter* p, const QRect& rect);
};
```

### 4.4 ToastNotify (通知弹窗)

```
┌────────────────────────────────┐
│ 🔔  新的审批任务                │
│     张三邀请你审批「请假申请」    │
│                          ╳     │
└────────────────────────────────┘
```

```cpp
class ToastNotify : public QWidget {
    Q_OBJECT

public:
    enum class Level { Info, Warning, Error, Success };

    static void show(QWidget* parent, const QString& title,
                     const QString& body, Level level = Level::Info,
                     int durationMs = 5000);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;   // 触发滑入动画

private:
    QString title_;
    QString body_;
    Level level_ = Level::Info;

    void startSlideIn();   // 从右上角滑入
    void startFadeOut();   // 淡出
};
```

动画规格：
- 滑入：`QPropertyAnimation` 作用于 `pos`，从 `(windowWidth, 20)` 到 `(windowWidth - width - 20, 20)`，持续 300ms，QEasingCurve::OutCubic
- 淡出：`QPropertyAnimation` 作用于 `windowOpacity`，从 1.0 到 0.0，持续 200ms

### 4.5 StatusIndicator (在线状态)

```cpp
class StatusIndicator : public QWidget {
    Q_OBJECT

public:
    enum class State { Online, Away, Busy, Offline };

    void setState(State state);
    State state() const;

    // 支持闪烁动画（新消息提醒）
    void startBlink(int intervalMs = 500);
    void stopBlink();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    State state_ = State::Offline;
    bool blinkOn_ = false;
    QTimer* blinkTimer_ = nullptr;
};
```

---

## 5. 样式系统

### 5.1 设计令牌 (Design Tokens)

```cpp
// gui/style/tokens.h
namespace gui::style {

// ---- 颜色 ----
namespace Color {
    inline constexpr auto Primary      = "#1677FF";  // 主色调
    inline constexpr auto PrimaryLight = "#E6F4FF";  // 主色浅底
    inline constexpr auto Success      = "#52C41A";
    inline constexpr auto Warning      = "#FAAD14";
    inline constexpr auto Error        = "#FF4D4F";
    inline constexpr auto TextPrimary  = "#1F1F1F";
    inline constexpr auto TextSecondary= "#8C8C8C";
    inline constexpr auto TextDisabled = "#BFBFBF";
    inline constexpr auto BgPrimary    = "#FFFFFF";
    inline constexpr auto BgSecondary  = "#F5F5F5";
    inline constexpr auto BgTertiary   = "#FAFAFA";
    inline constexpr auto Border       = "#E8E8E8";
    inline constexpr auto BubbleMine   = "#95EC69";
    inline constexpr auto BubbleOther  = "#FFFFFF";
}

// ---- 字体 ----
namespace Font {
    inline constexpr int SizeXS   = 11;
    inline constexpr int SizeSM   = 12;
    inline constexpr int SizeBase = 14;
    inline constexpr int SizeLG   = 16;
    inline constexpr int SizeXL   = 20;
    inline constexpr int SizeTitle= 24;
}

// ---- 间距 ----
namespace Spacing {
    inline constexpr int XS  = 4;
    inline constexpr int SM  = 8;
    inline constexpr int MD  = 12;
    inline constexpr int LG  = 16;
    inline constexpr int XL  = 24;
    inline constexpr int XXL = 32;
}

// ---- 圆角 ----
namespace Radius {
    inline constexpr int SM = 4;
    inline constexpr int MD = 8;
    inline constexpr int LG = 12;
    inline constexpr int Full = 9999;
}

// ---- 阴影 ----
namespace Shadow {
    inline constexpr auto Card = "0 1px 3px rgba(0,0,0,0.08)";
    inline constexpr auto Dropdown = "0 4px 12px rgba(0,0,0,0.12)";
    inline constexpr auto Modal = "0 8px 24px rgba(0,0,0,0.16)";
}

} // namespace gui::style
```

### 5.2 全局样式表

```cpp
// gui/style/global.qss (编译为 Qt 资源)
void applyGlobalStyle(QApplication* app) {
    app->setStyleSheet(R"(
        /* ---- 全局 ---- */
        * {
            font-family: -apple-system, "Segoe UI", "Noto Sans", sans-serif;
            font-size: 14px;
            color: #1F1F1F;
        }

        /* ---- 导航栏 ---- */
        #NavBar {
            background-color: #2C2C2C;
            min-width: 64px;
            max-width: 64px;
        }
        #NavBar QPushButton {
            border: none;
            border-radius: 8px;
            padding: 8px;
            color: #A0A0A0;
        }
        #NavBar QPushButton:hover {
            background-color: #3C3C3C;
            color: #FFFFFF;
        }
        #NavBar QPushButton[active="true"] {
            color: #FFFFFF;
        }
        #NavBar QPushButton[active="true"]::before {
            /* 左侧蓝色指示条 */
            background-color: #1677FF;
        }

        /* ---- 侧边栏 ---- */
        #SideBar {
            background-color: #F5F5F5;
            border-right: 1px solid #E8E8E8;
        }

        /* ---- 会话列表 ---- */
        #ConversationList QListView {
            background-color: #F5F5F5;
            border: none;
        }
        #ConversationList QListView::item {
            padding: 12px;
            border-bottom: 1px solid #F0F0F0;
        }
        #ConversationList QListView::item:selected {
            background-color: #E6F4FF;
        }
        #ConversationList QListView::item:hover {
            background-color: #ECECEC;
        }

        /* ---- 输入框 ---- */
        #MessageInput {
            border: 1px solid #E8E8E8;
            border-radius: 8px;
            padding: 10px;
            background-color: #FFFFFF;
        }
        #MessageInput:focus {
            border-color: #1677FF;
        }

        /* ---- 滚动条 ---- */
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
        }
        QScrollBar::handle:vertical {
            background: #D0D0D0;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #A0A0A0;
        }

        /* ---- 工具提示 ---- */
        QToolTip {
            background-color: #2C2C2C;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 12px;
        }
    )");
}
```

### 5.3 暗色主题

通过 `ConfigManager` 读取 `ui.theme` 切换全局样式表。暗色模式下的颜色映射：

| 令牌 | 亮色 | 暗色 |
|------|------|------|
| BgPrimary | `#FFFFFF` | `#1E1E1E` |
| BgSecondary | `#F5F5F5` | `#2D2D2D` |
| BgTertiary | `#FAFAFA` | `#252525` |
| TextPrimary | `#1F1F1F` | `#E0E0E0` |
| TextSecondary | `#8C8C8C` | `#9E9E9E` |
| Border | `#E8E8E8` | `#3A3A3A` |
| BubbleOther | `#FFFFFF` | `#3A3A3A` |

---

## 6. 交互设计

### 6.1 键盘快捷键

| 快捷键 | 功能 | 备注 |
|--------|------|------|
| `Ctrl+N` | 新建会话 | |
| `Ctrl+F` | 搜索消息/联系人 | 根据当前视图切换搜索对象 |
| `Ctrl+Enter` | 发送消息 | |
| `Enter` | 发送消息（可配置） | |
| `Ctrl+Tab` | 切换到下一个会话 | |
| `Ctrl+Shift+Tab` | 切换到上一个会话 | |
| `Ctrl+1..5` | 切换到导航页 1-5 | 消息/任务/文件/联系人/设置 |
| `Ctrl+R` | 引用回复选中消息 | |
| `Ctrl+C` | 复制选中消息文本 | |
| `Escape` | 关闭弹窗/取消选择/清空搜索 | |
| `Alt+Left` | 返回上一个视图 | |
| `Ctrl+Shift+A` | 上传文件 | |
| `Ctrl+D` | 截图发送 | 可选功能 |
| `Ctrl++/-` | 缩放字体 | |

### 6.2 通知交互

```
收到新消息 → 判断当前状态:
  ├── 聊天窗口在前台 + 当前会话
  │     → 直接追加到消息列表 + 标记已读
  │
  ├── 聊天窗口在前台 + 其他会话
  │     → 会话列表未读角标 +1 + 系统托盘角标更新
  │
  ├── 应用在后台（最小化/隐藏）
  │     → 系统通知弹窗 (Windows Toast / Linux D-Bus)
  │     → 系统托盘闪烁 + 角标更新
  │     → 任务栏图标高亮 (Windows) / 紧急标记 (Linux)
  │
  └── 免打扰模式
       → 静默记录，仅角标更新，无弹窗无声音
```

### 6.3 文件拖拽上传

```
拖拽文件到聊天窗口:
  1. 检测 dragEnterEvent → 判断文件类型和大小
  2. 显示拖拽指示器 (半透明蓝色虚线框)
  3. dropEvent → 获取文件路径列表
  4. 弹出确认对话框 (文件名 + 大小 + 发送按钮)
  5. 确认后 → FileService::uploadFile()
  6. 上传完成后 → 自动发送文件消息
```

### 6.4 消息右键菜单

```
右键消息气泡 → QMenu:
  ├── 📋 复制文本
  ├── 💬 引用回复
  ├── 📌 置顶消息 (仅群聊)
  ├── 🔗 复制消息链接
  ├── ──────────────
  ├── ❌ 撤回 (仅自己的消息, 2 分钟内)
  └── 🗑️ 删除 (仅本地)
```

---

## 7. 平台适配

### 7.1 系统托盘

```cpp
// platform/tray_impl.h
class IPlatformTray {
public:
    virtual ~IPlatformTray() = default;
    virtual void showMessage(const QString& title, const QString& body) = 0;
    virtual void setBadge(int count) = 0;
    virtual void setBlinking(bool blink) = 0;
};

// Windows 实现
class WinTray : public IPlatformTray {
    // 使用 QSystemTrayIcon + Windows ITaskbarList3 接口
    // 任务栏角标: ITaskbarList3::SetOverlayIcon()
};

// Linux 实现
class LinuxTray : public IPlatformTray {
    // 使用 QSystemTrayIcon + D-Bus org.freedesktop.Notifications
    // 部分 DE (KDE) 支持角标，GNOME 不支持
};
```

### 7.2 窗口行为

| 行为 | Windows | Linux |
|------|---------|-------|
| 最小化到托盘 | ✅ (点击关闭按钮最小化) | ✅ |
| 任务栏进度条 | `ITaskbarList3::SetProgressValue` | Unity Launcher API |
| 窗口阴影 | DWM 原生 | 自绘 (QGraphicsDropShadowEffect) |
| 窗口圆角 | Windows 11 原生 | 自绘 (QPainterPath + setMask) |
| 文件对话框 | QFileDialog (原生) | QFileDialog (GTK 风格) |

### 7.3 高 DPI 适配

```cpp
// main.cpp
int main(int argc, char* argv[]) {
    // Qt 6 默认启用 High DPI
    // 手动设置策略 (Qt 5 兼容)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    // 设置缩放取整策略
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    // ...
}
```

所有图标使用 SVG 格式，`QIcon::fromTheme()` 优先使用系统图标。

---

## 8. GUI 与核心层的连接

### 8.1 信号槽连接图

```
core::service::IMService (工作线程, QObject)
    │
    ├── signal: newMessageReceived(Message)
    │        │ QueuedConnection
    │        ▼
    │   gui::MainWindow (GUI 线程)
    │        │
    │        ├──► gui::ChatView::appendMessage()
    │        ├──► gui::ConversationList::updateLastMessage()
    │        └──► gui::TrayIcon::notify()

core::net::ConnectionManager (Main Thread, Qt Event Loop)
    │  异步 IO 收发在主线程；重计算（编解码/加解密）
    │  通过 QueuedConnection 投递给 Worker Pool
    │
    ├── signal: onStateChanged(ConnectionState)
    │        │ QueuedConnection
    │        ▼
    │   gui::StatusBar::updateConnectionIcon()

core::infra::WorkerPool (N workers)
    │  接收 Main Thread 投递的 RawMessage
    │  执行 Protocol 编解码、校验、解密
    │  完成后通过 Signal 回传 DecodedMsg 给 Main Thread
    │
    ├── signal: messageDecoded(DecodedMsg)
    │        │ QueuedConnection → Main Thread → GUI / DB

core::service::FileService (Worker Thread)
    │
    ├── signal: transferProgressChanged(id, progress, speed)
    │        │ QueuedConnection
    │        ▼
    │   gui::FilePanel::updateProgress()
    │   gui::SpeedChart::addDataPoint()
```

### 8.2 线程安全桥接

```cpp
// 在工作线程中安全调用 GUI 更新
template<typename Func>
void runOnGuiThread(Func&& func) {
    QMetaObject::invokeMethod(QApplication::instance(),
        std::forward<Func>(func), Qt::QueuedConnection);
}

// 使用示例（任何工作线程）
runOnGuiThread([this, msg = std::move(message)] {
    chatView_->appendMessage(msg);
});
```

---

## 9. 性能考虑

### 9.1 消息列表虚拟化

`QListView` + 自定义 `QAbstractListModel` 天然支持视图虚拟化——只渲染可见区域的消息气泡。对于 10000 条消息的会话，实际渲染的只有 ~20 个气泡。

### 9.2 图片懒加载

```cpp
class LazyImageLoader : public QObject {
    // 维护一个优先级队列
    // 可见区域内的图片优先加载
    // 滚出可见区域的图片取消加载
    // 内存中最多缓存 50 张解码后的缩略图
};
```

### 9.3 绘制优化

- 消息气泡背景预渲染为 `QPixmap` 缓存，避免每次 paintEvent 重复计算圆角路径
- 使用 `QWidget::setAttribute(Qt::WA_OpaquePaintEvent)` 减少不必要的背景擦除
- `QListView::setUniformItemSizes(true)` 允许 Qt 做更多布局优化（但由于消息气泡高度可变，设为 false 更准确）

---

## 附录 A — 控件清单

| 控件 | 基类 | 是否自定义绘制 | 所在模块 |
|------|------|:---:|------|
| MainWindow | QMainWindow | 否 | gui/ |
| NavBar | QWidget | 是 | gui/widgets/ |
| ConversationList | QListView | 否 (自定义 Delegate) | gui/views/ |
| ChatView | QWidget (组合) | 否 | gui/views/ |
| MessageBubble | QWidget | 是 | gui/widgets/ |
| AvatarWidget | QWidget | 是 | gui/widgets/ |
| ContactTree | QTreeView | 否 (自定义 Model) | gui/views/ |
| TaskList | QListView | 否 (自定义 Delegate) | gui/views/ |
| FilePanel | QWidget (组合) | 否 | gui/views/ |
| SpeedChart | QWidget | 是 | gui/widgets/ |
| Dashboard | QWidget (组合) | 否 | gui/views/ |
| ToastNotify | QWidget | 是 | gui/widgets/ |
| StatusIndicator | QWidget | 是 | gui/widgets/ |
| MessageInput | QTextEdit | 否 (子类化) | gui/widgets/ |

---

## 附录 B — 文档修订记录

| 版本 | 日期 | 作者 | 变更说明 |
|------|------|------|----------|
| v0.1 | 2025-01 | — | 初稿：7 个视图 + 5 个自定义控件 + 样式系统 + 交互 + 平台适配 |
