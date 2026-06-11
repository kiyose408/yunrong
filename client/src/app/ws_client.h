// app/ws_client.h — QWebSocket + 心跳 + 断线重连

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
    void maxReconnectReached();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTextMessage(const QString& text);
    void onHeartbeatTick();
    void onReconnectTick();

private:
    void resetActivity();
    int  reconnectDelayMs() const;

    QWebSocket     m_socket;
    QTimer         m_heartbeatTimer;
    QTimer         m_reconnectTimer;
    QElapsedTimer  m_lastActivity;
    QUrl           m_url;
    int            m_reconnectAttempt = 0;
    bool           m_timedOut         = false;
    bool           m_manualClose      = false;

    static constexpr int kPingIntervalSec = 30;
    static constexpr int kTimeoutSec      = 90;
    static constexpr int kMaxReconnect    = 10;
    static constexpr int kMaxBackoffMs    = 60000;
};
