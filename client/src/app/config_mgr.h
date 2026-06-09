// app/config_mgr.h — JSON 配置文件加载

#pragma once

#include <QString>
#include <nlohmann/json.hpp>

class ConfigManager
{
public:
    bool loadFromFile(const QString& path);

    QString serverHost() const;
    int     serverPort() const;
    bool    serverTls() const;

private:
    nlohmann::json m_data;
    QString getString(const char* key, const QString& fallback) const;
    int     getInt(const char* key, int fallback) const;
    bool    getBool(const char* key, bool fallback) const;
};
