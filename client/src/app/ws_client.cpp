// app/ws_client.cpp — QWebSocket + 心跳实现

#include "ws_client.h"
#include <QJsonDocument>
#include <QDebug>

WsClient::WsClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &WsClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WsClient::onDisconnected);
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WsClient::onError);
    connect(&m_socket, &QWebSocket::textMessageReceived,
            this, &WsClient::onTextMessage);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &WsClient::onHeartbeatTick);
}

WsClient::~WsClient()
{
    close();
}

void WsClient::open(const QUrl& url)
{
    qInfo() << "WebSocket connecting to" << url.toString();
    m_socket.open(url);
}

void WsClient::close()
{
    m_heartbeatTimer.stop();
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        m_socket.close();
    }
}

void WsClient::sendJson(const QJsonObject& obj)
{
    QJsonDocument doc(obj);
    QString text = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    qInfo() << "WS send:" << text;
    m_socket.sendTextMessage(text);
}

// ---- slots ----

void WsClient::onConnected()
{
    qInfo() << "WebSocket connected";
    m_timedOut = false;
    resetActivity();
    m_heartbeatTimer.start(kPingIntervalSec * 1000);
    emit connected();
}

void WsClient::onDisconnected()
{
    m_heartbeatTimer.stop();
    qInfo() << "WebSocket disconnected";
    emit disconnected();
}

void WsClient::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << m_socket.errorString();
}

void WsClient::onTextMessage(const QString& text)
{
    resetActivity();   // 任何消息都算活动

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "WS received non-JSON:" << text;
        return;
    }
    qInfo() << "WS recv:" << text;
    emit messageReceived(doc.object());
}

void WsClient::onHeartbeatTick()
{
    if (m_timedOut) return;

    qint64 elapsed = m_lastActivity.elapsed() / 1000;
    if (elapsed > kTimeoutSec) {
        m_timedOut = true;
        m_heartbeatTimer.stop();
        qWarning() << "Heartbeat timeout: no message for" << elapsed
                   << "seconds, connection lost";
        m_socket.close();
        emit heartbeatTimeout();
        return;
    }

    QJsonObject ping;
    ping["type"] = QStringLiteral("ping");
    sendJson(ping);
}

// ---- private ----

void WsClient::resetActivity()
{
    m_lastActivity.start();
}
