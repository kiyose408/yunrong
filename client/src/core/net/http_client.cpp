// net/http_client.cpp — QNetworkAccessManager 最小封装实现

#include "net/http_client.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QDebug>

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
{
}

void HttpClient::setBaseUrl(const QString& base)
{
    m_baseUrl = QUrl(base);
}

void HttpClient::postJson(const QString& path, const QJsonObject& body)
{
    QUrl url = m_baseUrl.resolved(path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(body);
    QNetworkReply* reply = m_nam.post(req, doc.toJson(QJsonDocument::Compact));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "HTTP POST failed:" << reply->url().toString()
                       << "-" << reply->errorString();
            emit requestFailed(reply->errorString());
            return;
        }

        QByteArray raw = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "HTTP response not JSON:" << raw;
            emit requestFailed("invalid JSON response");
            return;
        }

        emit responseReceived(status, doc.object());
    });
}
