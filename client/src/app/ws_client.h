// app/ws_client.h — QWebSocket 最小封装

#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QUrl>

class WsClient : public QObject
{
    Q_OBJECT
public:
    explicit WsClient(QObject* parent = nullptr);
    ~WsClient();

    void open(const QUrl& url);
    void close();
    void sendJson(const QJsonObject& obj);

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject& msg);
    void heartbeatTimeout();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTextMessage(const QString& text);
    void onHeartbeatTick();

private:
    void resetActivity();

    QWebSocket     m_socket;
    QTimer         m_heartbeatTimer;
    QElapsedTimer  m_lastActivity;
    bool           m_timedOut = false;

    static constexpr int kPingIntervalSec = 30;
    static constexpr int kTimeoutSec      = 90;
};
