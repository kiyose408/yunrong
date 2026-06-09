// app/config_mgr.cpp — JSON 配置文件加载实现

#include "config_mgr.h"
#include <QFile>
#include <QDebug>

bool ConfigManager::loadFromFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Config file not found:" << path << "- using defaults";
        m_data = nlohmann::json::object();
        return false;
    }

    QByteArray bytes = file.readAll();
    try {
        m_data = nlohmann::json::parse(bytes.toStdString());
        qInfo() << "Config loaded from" << path;
        return true;
    } catch (const nlohmann::json::exception& e) {
        qWarning() << "Config parse error:" << e.what() << "- using defaults";
        m_data = nlohmann::json::object();
        return false;
    }
}

QString ConfigManager::serverHost() const { return getString("/server/host", "127.0.0.1"); }
int     ConfigManager::serverPort() const { return getInt("/server/port", 8080); }
bool    ConfigManager::serverTls()  const { return getBool("/server/tls", false); }

// ---- private helpers ----

QString ConfigManager::getString(const char* key, const QString& fallback) const
{
    auto ptr = m_data;
    for (const auto& part : QString(key).split('/', Qt::SkipEmptyParts)) {
        if (!ptr.contains(part.toStdString())) return fallback;
        ptr = ptr[part.toStdString()];
    }
    return ptr.is_string() ? QString::fromStdString(ptr.get<std::string>()) : fallback;
}

int ConfigManager::getInt(const char* key, int fallback) const
{
    auto ptr = m_data;
    for (const auto& part : QString(key).split('/', Qt::SkipEmptyParts)) {
        if (!ptr.contains(part.toStdString())) return fallback;
        ptr = ptr[part.toStdString()];
    }
    return ptr.is_number_integer() ? ptr.get<int>() : fallback;
}

bool ConfigManager::getBool(const char* key, bool fallback) const
{
    auto ptr = m_data;
    for (const auto& part : QString(key).split('/', Qt::SkipEmptyParts)) {
        if (!ptr.contains(part.toStdString())) return fallback;
        ptr = ptr[part.toStdString()];
    }
    return ptr.is_boolean() ? ptr.get<bool>() : fallback;
}
