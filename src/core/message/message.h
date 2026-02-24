#ifndef MESSAGE_H
#define MESSAGE_H

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include "../utils/enums.h"

namespace LocalNetworkApp {

class Message {
public:
    Message(QUuid senderId, QUuid receiverId, const QString &content, MessageType type = MessageType::Text);
    Message(const QJsonObject &json);
    ~Message() = default;

    // 获取消息ID
    QUuid getMessageId() const;

    // 获取发送者ID
    QUuid getSenderId() const;

    // 获取接收者ID
    QUuid getReceiverId() const;

    // 获取消息内容
    QString getContent() const;

    // 获取消息类型
    MessageType getType() const;

    // 获取消息时间戳
    QDateTime getTimestamp() const;

    // 设置消息已读状态
    void setRead(bool read);

    // 获取消息已读状态
    bool isRead() const;

    // 转换为JSON格式
    QJsonObject toJson() const;

private:
    QUuid messageId;    // 消息唯一标识符
    QUuid senderId;     // 发送者ID
    QUuid receiverId;   // 接收者ID
    QString content;    // 消息内容
    MessageType type;   // 消息类型
    QDateTime timestamp; // 消息时间戳
    bool read;          // 是否已读
};

} // namespace LocalNetworkApp

#endif // MESSAGE_H
