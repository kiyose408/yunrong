// app/ws_client.h — QWebSocket 最小封装

#pragma once

#include <QObject>
#include <QWebSocket>
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

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTextMessage(const QString& text);

private:
    QWebSocket m_socket;
};
