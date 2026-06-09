// src/app/main.cpp — 应用入口，Phase 1 最小骨架

#include <QApplication>
#include <QWidget>
#include <QDir>
#include "logger.h"
#include "config_mgr.h"
#include "ws_client.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("YunRong");

    // 日志
    QString logPath = QApplication::applicationDirPath() + "/yunrong.log";
    Logger::instance().init(logPath);
    LOG_INFO() << "Yunrong starting...";

    // 配置
    ConfigManager cfg;
    QString configPath = QApplication::applicationDirPath()
                         + "/../../../../config/default.json";
    cfg.loadFromFile(configPath);

    LOG_INFO() << "Server:" << cfg.serverHost() << ":" << cfg.serverPort()
               << "(TLS:" << (cfg.serverTls() ? "on" : "off") << ")";

    // WebSocket 连接（Mock Server 未建，预期连接失败）
    QString wsUrl = QString("ws://%1:%2/ws").arg(cfg.serverHost()).arg(cfg.serverPort());
    WsClient wsClient;
    wsClient.open(QUrl(wsUrl));

    QWidget window;
    window.setWindowTitle("YunRong");
    window.resize(400, 300);
    window.show();

    int ret = app.exec();
    LOG_INFO() << "Yunrong exiting";
    return ret;
}
