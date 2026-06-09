// app/ws_client.h — QWebSocket 最小封装

#pragma once

#include <QObject>
#include <QWebSocket>
#include <QUrl>

class WsClient : public QObject
{
    Q_OBJECT
public:
    explicit WsClient(QObject* parent = nullptr);
    ~WsClient();

    void open(const QUrl& url);
    void close();

signals:
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket m_socket;
};
