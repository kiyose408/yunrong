// net/http_client.h — QNetworkAccessManager 最小封装

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QUrl>

class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject* parent = nullptr);

    void setBaseUrl(const QString& base);
    void postJson(const QString& path, const QJsonObject& body);

signals:
    void responseReceived(int statusCode, const QJsonObject& data);
    void requestFailed(const QString& error);

private:
    QNetworkAccessManager m_nam;
    QUrl m_baseUrl;
};
