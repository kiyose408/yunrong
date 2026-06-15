// app/logger.h — Qt 全局消息处理器封装

#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>

class Logger
{
public:
    static Logger& instance();

    void init(const QString& filePath);

private:
    Logger() = default;
    ~Logger();

    static void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);
    void writeLine(const QString& line);

    QFile m_file;
};

// 便捷宏——走 qDebug/qInfo/qWarning/qCritical，自动被 messageHandler 拦截
#define LOG_DEBUG    qDebug
#define LOG_INFO     qInfo
#define LOG_WARN     qWarning
#define LOG_ERROR    qCritical
