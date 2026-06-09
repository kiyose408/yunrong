// src/app/main.cpp — 应用入口，Phase 1 最小骨架

#include <QApplication>
#include <QWidget>
#include <QDir>
#include "logger.h"
#include "config_mgr.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("YunRong");

    // 日志
    QString logPath = QApplication::applicationDirPath() + "/yunrong.log";
    Logger::instance().init(logPath);
    LOG_INFO() << "Yunrong starting...";

    // 配置：从 build 目录向上回溯 4 级到项目根 config/
    // exe 在 build/Desktop_.../src/app/ → ../../../../config/default.json
    ConfigManager cfg;
    QString configPath = QApplication::applicationDirPath()
                         + "/../../../../config/default.json";
    cfg.loadFromFile(configPath);

    LOG_INFO() << "Server:" << cfg.serverHost() << ":" << cfg.serverPort()
               << "(TLS:" << (cfg.serverTls() ? "on" : "off") << ")";

    QWidget window;
    window.setWindowTitle("YunRong");
    window.resize(400, 300);
    window.show();

    int ret = app.exec();
    LOG_INFO() << "Yunrong exiting";
    return ret;
}
