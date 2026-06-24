// src/app/main.cpp — 应用入口

#include <QApplication>
#include <QWidget>
#include <QJsonObject>
#include "infra/logger.h"
#include "infra/config_mgr.h"
#include "net/http_client.h"
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

    QString baseUrl = QString("http://%1:%2").arg(cfg.serverHost()).arg(cfg.serverPort());
    LOG_INFO() << "Server:" << baseUrl << "(TLS:" << (cfg.serverTls() ? "on" : "off") << ")";

    // HTTP 登录
    HttpClient http;
    http.setBaseUrl(baseUrl);

    QObject::connect(&http, &HttpClient::responseReceived, [](int status, const QJsonObject& data) {
        qInfo() << "Login response:" << status;
        if (data.contains("data")) {
            QJsonObject d = data["data"].toObject();
            qInfo() << "  user_id:" << d["user_id"].toInt();
            qInfo() << "  token:" << d["access_token"].toString();
        }
    });
    QObject::connect(&http, &HttpClient::requestFailed, [](const QString& error) {
        qWarning() << "Login failed:" << error;
    });

    QJsonObject loginBody;
    loginBody["username"] = QStringLiteral("admin");
    loginBody["password"] = QStringLiteral("123456");
    http.postJson("/api/v1/auth/login", loginBody);

    // WebSocket 连接
    QString wsUrl = QString("ws://%1:%2/ws").arg(cfg.serverHost()).arg(cfg.serverPort());
    WsClient wsClient;

    QObject::connect(&wsClient, &WsClient::messageReceived, [](const QJsonObject& msg) {
        qInfo() << "Message received - type:" << msg.value("type").toString();
    });
    QObject::connect(&wsClient, &WsClient::heartbeatTimeout, []() {
        qWarning() << "Connection lost (heartbeat timeout)";
    });
    QObject::connect(&wsClient, &WsClient::maxReconnectReached, []() {
        qWarning() << "Giving up after max reconnect attempts";
    });

    wsClient.open(QUrl(wsUrl));

    QWidget window;
    window.setWindowTitle("YunRong");
    window.resize(400, 300);
    window.show();

    int ret = app.exec();
    LOG_INFO() << "Yunrong exiting";
    return ret;
}
