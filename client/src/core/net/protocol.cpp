// net/protocol.cpp — 消息帧编解码实现

#include "net/protocol.h"
#include <QJsonDocument>
#include <QDateTime>

Protocol::Protocol()
    : m_seq(0)
{
}

QString Protocol::encode(const QString& type, const QJsonObject& payload)
{
    QJsonObject frame;
    frame["ver"] = m_version;
    frame["type"] = type;
    frame["seq"] = static_cast<int>(m_seq++);
    frame["ts"]  = QDateTime::currentMSecsSinceEpoch();
    frame["payload"] = payload;

    QJsonDocument doc(frame);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString Protocol::encodeMsg(const QJsonObject& msgPayload)
{
    return encode("msg", msgPayload);
}

QString Protocol::encodePing()
{
    return encode("ping", QJsonObject());
}

Protocol::Frame Protocol::decode(const QString& json) const
{
    Frame f;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        f.error = QString("JSON parse error: %1").arg(err.errorString());
        return f;
    }

    if (!doc.isObject()) {
        f.error = "frame is not a JSON object";
        return f;
    }

    QJsonObject obj = doc.object();

    // 必需字段检查
    if (!obj.contains("type") || !obj["type"].isString()) {
        f.error = "missing or invalid 'type' field";
        return f;
    }

    f.valid   = true;
    f.type    = obj["type"].toString();
    f.seq     = static_cast<uint32_t>(obj["seq"].toInt(0));
    f.payload = obj["payload"].toObject();
    return f;
}
