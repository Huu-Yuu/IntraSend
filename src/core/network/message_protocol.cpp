#include "message_protocol.h"
#include <QJsonDocument>
#include <QDateTime>
#include <QDataStream>
#include <QIODevice>
namespace LocalNetworkApp {

QByteArray MessageProtocol::serializeMessage(const NetworkMessage &message)
{
    // 创建消息内容
    QJsonObject messageObj;
    messageObj["type"] = static_cast<int>(message.type);
    messageObj["messageId"] = message.messageId.toString();
    messageObj["senderId"] = message.senderId.toString();
    messageObj["timestamp"] = message.timestamp.toString(Qt::ISODate);
    messageObj["content"] = message.content;

    // 序列化消息内容
    QJsonDocument doc(messageObj);
    QByteArray contentData = doc.toJson(QJsonDocument::Compact);

    // 创建消息头
    MessageHeader header;
    header.magic = MAGIC_NUMBER;
    header.version = PROTOCOL_VERSION;
    header.contentSize = static_cast<quint32>(contentData.size());

    // 序列化消息头
    QByteArray headerData;
    QDataStream headerStream(&headerData, QIODevice::WriteOnly);
    headerStream.setByteOrder(QDataStream::BigEndian);
    headerStream << header.magic << header.version << header.contentSize;

    // 组合消息头和消息内容
    return headerData + contentData;
}

MessageProtocol::NetworkMessage MessageProtocol::deserializeMessage(const QByteArray &data)
{
    NetworkMessage message;

    // 检查数据大小是否足够
    if (data.size() < sizeof(MessageHeader)) {
        // 数据不完整，返回空消息
        message.type = NetworkMessageType::Heartbeat;
        message.messageId = QUuid::createUuid();
        message.senderId = QUuid();
        message.timestamp = QDateTime::currentDateTime();
        return message;
    }

    // 解析消息头
    MessageHeader header;
    QDataStream headerStream(data.left(sizeof(MessageHeader)));
    headerStream.setByteOrder(QDataStream::BigEndian);
    headerStream >> header.magic >> header.version >> header.contentSize;

    // 检查魔术数字和版本
    if (header.magic != MAGIC_NUMBER || header.version != PROTOCOL_VERSION) {
        // 无效消息，返回空消息
        message.type = NetworkMessageType::Heartbeat;
        message.messageId = QUuid::createUuid();
        message.senderId = QUuid();
        message.timestamp = QDateTime::currentDateTime();
        return message;
    }

    // 检查数据大小是否足够
    if (data.size() < sizeof(MessageHeader) + header.contentSize) {
        // 数据不完整，返回空消息
        message.type = NetworkMessageType::Heartbeat;
        message.messageId = QUuid::createUuid();
        message.senderId = QUuid();
        message.timestamp = QDateTime::currentDateTime();
        return message;
    }

    // 解析消息内容
    QByteArray contentData = data.mid(sizeof(MessageHeader), header.contentSize);
    QJsonDocument doc = QJsonDocument::fromJson(contentData);
    QJsonObject messageObj = doc.object();

    // 提取消息字段
    message.type = static_cast<NetworkMessageType>(messageObj["type"].toInt());
    message.messageId = QUuid(messageObj["messageId"].toString());
    message.senderId = QUuid(messageObj["senderId"].toString());
    message.timestamp = QDateTime::fromString(messageObj["timestamp"].toString(), Qt::ISODate);
    message.content = messageObj["content"].toObject();

    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createUserStatusMessage(QUuid senderId, const QJsonObject &statusContent)
{
    NetworkMessage message;
    message.type = NetworkMessageType::UserStatus;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = statusContent;
    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createChatMessage(QUuid senderId, const QJsonObject &messageContent)
{
    NetworkMessage message;
    message.type = NetworkMessageType::ChatMessage;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = messageContent;
    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createFileTransferRequestMessage(QUuid senderId, const QJsonObject &requestContent)
{
    NetworkMessage message;
    message.type = NetworkMessageType::FileTransferRequest;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = requestContent;
    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createFileTransferResponseMessage(QUuid senderId, const QJsonObject &responseContent)
{
    NetworkMessage message;
    message.type = NetworkMessageType::FileTransferResponse;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = responseContent;
    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createFileDataMessage(QUuid senderId, const QJsonObject &fileDataContent)
{
    NetworkMessage message;
    message.type = NetworkMessageType::FileData;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = fileDataContent;
    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createUserDiscoveryMessage(QUuid senderId, const QJsonObject &discoveryContent)
{
    NetworkMessage message;
    message.type = NetworkMessageType::UserDiscovery;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = discoveryContent;
    return message;
}

MessageProtocol::NetworkMessage MessageProtocol::createHeartbeatMessage(QUuid senderId)
{
    NetworkMessage message;
    message.type = NetworkMessageType::Heartbeat;
    message.messageId = QUuid::createUuid();
    message.senderId = senderId;
    message.timestamp = QDateTime::currentDateTime();
    message.content = QJsonObject();
    return message;
}

} // namespace LocalNetworkApp
