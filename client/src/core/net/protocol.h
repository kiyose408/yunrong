// net/protocol.h — 消息帧编解码

#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <cstdint>

class Protocol
{
public:
    Protocol();

    // 编码：type + payload → 完整 JSON 帧字符串
    QString encode(const QString& type, const QJsonObject& payload);
    QString encodeMsg(const QJsonObject& msgPayload);
    QString encodePing();

    // 解码：JSON 帧字符串 → type + payload
    struct Frame {
        bool        valid = false;
        QString     type;
        uint32_t    seq  = 0;
        QJsonObject payload;
        QString     error;   // 非空表示解码失败
    };
    Frame decode(const QString& json) const;

    // seq 管理
    uint32_t nextSeq() const { return m_seq; }
    void     resetSeq() { m_seq = 0; }

private:
    uint32_t m_seq = 0;
    uint8_t  m_version = 1;
};
