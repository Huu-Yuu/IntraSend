#ifndef MESSAGE_PROTOCOL_H
#define MESSAGE_PROTOCOL_H

#include <QByteArray>
#include <QJsonObject>
#include <QUuid>
#include "../utils/enums.h"

namespace LocalNetworkApp {

class MessageProtocol {
public:
    MessageProtocol() = delete;
    ~MessageProtocol() = delete;

    // 网络消息基础结构
    struct NetworkMessage {
        NetworkMessageType type;
        QUuid messageId;
        QUuid senderId;
        QDateTime timestamp;
        QJsonObject content;
    };

    // 序列化网络消息
    static QByteArray serializeMessage(const NetworkMessage &message);

    // 反序列化网络消息
    static NetworkMessage deserializeMessage(const QByteArray &data);

    // 创建用户状态消息
    static NetworkMessage createUserStatusMessage(QUuid senderId, const QJsonObject &statusContent);

    // 创建聊天消息
    static NetworkMessage createChatMessage(QUuid senderId, const QJsonObject &messageContent);

    // 创建文件传输请求消息
    static NetworkMessage createFileTransferRequestMessage(QUuid senderId, const QJsonObject &requestContent);

    // 创建文件传输响应消息
    static NetworkMessage createFileTransferResponseMessage(QUuid senderId, const QJsonObject &responseContent);

    // 创建文件数据块消息
    static NetworkMessage createFileDataMessage(QUuid senderId, const QJsonObject &fileDataContent);

    // 创建用户发现消息
    static NetworkMessage createUserDiscoveryMessage(QUuid senderId, const QJsonObject &discoveryContent);

    // 创建心跳消息
    static NetworkMessage createHeartbeatMessage(QUuid senderId);

private:
    // 消息头结构
    struct MessageHeader {
        quint32 magic;          // 魔术数字，用于标识消息
        quint32 version;        // 协议版本
        quint32 contentSize;    // 内容大小
    };

    static const quint32 MAGIC_NUMBER = 0x4C4E4150; // "LANP" 的ASCII码
    static const quint32 PROTOCOL_VERSION = 1;
};

} // namespace LocalNetworkApp

#endif // MESSAGE_PROTOCOL_H
