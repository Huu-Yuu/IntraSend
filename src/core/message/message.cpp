#include "message.h"

namespace LocalNetworkApp {

Message::Message(QUuid senderId, QUuid receiverId, const QString &content, MessageType type) :
    messageId(QUuid::createUuid()),
    senderId(senderId),
    receiverId(receiverId),
    content(content),
    type(type),
    timestamp(QDateTime::currentDateTime()),
    read(false)
{
}

Message::Message(const QJsonObject &json) :
    messageId(QUuid(json["messageId"].toString())),
    senderId(QUuid(json["senderId"].toString())),
    receiverId(QUuid(json["receiverId"].toString())),
    content(json["content"].toString()),
    type(static_cast<MessageType>(json["type"].toInt())),
    timestamp(QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate)),
    read(json["read"].toBool())
{
}

QUuid Message::getMessageId() const
{
    return messageId;
}

QUuid Message::getSenderId() const
{
    return senderId;
}

QUuid Message::getReceiverId() const
{
    return receiverId;
}

QString Message::getContent() const
{
    return content;
}

MessageType Message::getType() const
{
    return type;
}

QDateTime Message::getTimestamp() const
{
    return timestamp;
}

void Message::setRead(bool read)
{
    this->read = read;
}

bool Message::isRead() const
{
    return read;
}

QJsonObject Message::toJson() const
{
    QJsonObject json;
    json["messageId"] = messageId.toString();
    json["senderId"] = senderId.toString();
    json["receiverId"] = receiverId.toString();
    json["content"] = content;
    json["type"] = static_cast<int>(type);
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["read"] = read;
    return json;
}

} // namespace LocalNetworkApp