// infra/logger.cpp — Qt 全局消息处理器，输出到文件 + 控制台

#include "infra/logger.h"
#include <QDateTime>
#include <iostream>

Logger& Logger::instance()
{
    static Logger s;
    return s;
}

Logger::~Logger()
{
    if (m_file.isOpen())
        m_file.close();
}

void Logger::init(const QString& filePath)
{
    m_file.setFileName(filePath);
    bool ok = m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    qInstallMessageHandler(Logger::messageHandler);

    if (ok)
        qInfo() << "Logger initialized:" << filePath;
    else
        qWarning() << "Logger: failed to open" << filePath << "-" << m_file.errorString();
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& /*ctx*/, const QString& msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO";  break;
    case QtWarningMsg:  level = "WARN";  break;
    case QtCriticalMsg: level = "ERROR"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    QString line = QString("[%1] [%2] %3").arg(timestamp, level, msg);

    // 控制台
    std::cout << line.toStdString() << std::endl;

    // 文件
    Logger& self = instance();
    self.writeLine(line);

    if (type == QtFatalMsg)
        std::abort();
}

void Logger::writeLine(const QString& line)
{
    if (!m_file.isOpen()) return;
    QTextStream stream(&m_file);
    stream << line << "\n";
    stream.flush();
}
