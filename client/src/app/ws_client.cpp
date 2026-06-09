// app/ws_client.cpp — QWebSocket 最小封装实现

#include "ws_client.h"
#include <QDebug>

WsClient::WsClient(QObject* parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &WsClient::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &WsClient::onDisconnected);
    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WsClient::onError);
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
