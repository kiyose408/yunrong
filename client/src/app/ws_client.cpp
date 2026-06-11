// app/ws_client.cpp — QWebSocket 最小封装实现

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

void WsClient::onConnected()
{
    qInfo() << "WebSocket connected";
    emit connected();
}

void WsClient::onDisconnected()
{
    qInfo() << "WebSocket disconnected";
    emit disconnected();
}

void WsClient::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "WebSocket error:" << m_socket.errorString();
}

void WsClient::onTextMessage(const QString& text)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "WS received non-JSON:" << text;
        return;
    }
    qInfo() << "WS recv:" << text;
    emit messageReceived(doc.object());
}
