// src/app/main.cpp — 应用入口，Phase 1 最小骨架

#include <QApplication>
#include <QWidget>
#include <QDir>
#include "logger.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("YunRong");

    // 日志文件写到程序所在目录
    QString logPath = QApplication::applicationDirPath() + "/yunrong.log";
    Logger::instance().init(logPath);
    LOG_INFO("Yunrong starting...");

    QWidget window;
    window.setWindowTitle("YunRong");
    window.resize(400, 300);
    window.show();

    int ret = app.exec();
    LOG_INFO("Yunrong exiting");
    return ret;
}
