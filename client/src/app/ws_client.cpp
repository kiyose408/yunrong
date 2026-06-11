// app/ws_client.cpp — QWebSocket + 心跳 + 断线重连

#include "ws_client.h"
#include <QJsonDocument>
#include <QDebug>
#include <algorithm>

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
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WsClient::onReconnectTick);
}

WsClient::~WsClient()
{
    close();
}

void WsClient::open(const QUrl& url)
{
    m_url = url;
    m_manualClose = false;
    m_timedOut = false;
    m_reconnectAttempt = 0;
    qInfo() << "WebSocket connecting to" << url.toString();
    m_socket.open(url);
}

void WsClient::close()
{
    m_manualClose = true;
    m_heartbeatTimer.stop();
    m_reconnectTimer.stop();
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
    m_reconnectAttempt = 0;
    m_reconnectTimer.stop();
    resetActivity();
    m_heartbeatTimer.start(kPingIntervalSec * 1000);
    emit connected();
}

void WsClient::onDisconnected()
{
    m_heartbeatTimer.stop();
    qInfo() << "WebSocket disconnected";
    emit disconnected();

    if (m_manualClose) return;

    // 非主动断开 → 启动重连
    if (m_reconnectAttempt >= kMaxReconnect) {
        qWarning() << "Max reconnect attempts (" << kMaxReconnect << ") reached, giving up";
        emit maxReconnectReached();
        return;
    }

    int delayMs = reconnectDelayMs();
    qInfo() << "Reconnecting in" << delayMs << "ms (attempt"
            << (m_reconnectAttempt + 1) << "of" << kMaxReconnect << ")";
    m_reconnectTimer.start(delayMs);
}

void WsClient::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << m_socket.errorString();
}

void WsClient::onTextMessage(const QString& text)
{
    resetActivity();

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
        qWarning() << "Heartbeat timeout:" << elapsed << "s, connection lost";
        m_socket.close();   // 触发 onDisconnected → 自动重连
        emit heartbeatTimeout();
        return;
    }

    QJsonObject ping;
    ping["type"] = QStringLiteral("ping");
    sendJson(ping);
}

void WsClient::onReconnectTick()
{
    m_reconnectTimer.stop();
    m_reconnectAttempt++;
    qInfo() << "Reconnect attempt" << m_reconnectAttempt;
    m_socket.open(m_url);
}

// ---- private ----

void WsClient::resetActivity()
{
    m_lastActivity.start();
}

int WsClient::reconnectDelayMs() const
{
    // 指数退避: 1s, 2s, 4s, 8s, 16s, 32s, 60s, 60s...
    int ms = 1000 * (1 << m_reconnectAttempt);
    return std::min(ms, kMaxBackoffMs);
}
