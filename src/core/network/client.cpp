#include "client.h"
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "../utils/constants.h"

namespace LocalNetworkApp {

Client::Client(const UserIdentity &userIdentity, QObject *parent) :
    QObject(parent),
    userIdentity(userIdentity),
    serverPort(Constants::DEFAULT_TCP_PORT),
    reconnecting(false)
{
    initSocket();
}

Client::~Client()
{
    disconnectFromServer();
    delete tcpSocket;
}

bool Client::connectToServer(const QHostAddress &serverAddress, quint16 serverPort)
{
    if (isConnected()) {
        return true;
    }

    this->serverAddress = serverAddress;
    this->serverPort = serverPort;

    // 停止重连定时器
    if (reconnectTimer) {
        reconnectTimer->stop();
    }

    reconnecting = false;

    // 连接到服务器
    tcpSocket->connectToHost(serverAddress, serverPort);

    return true;
}

void Client::disconnectFromServer()
{
    if (!tcpSocket) {
        return;
    }

    stopTimers();

    if (tcpSocket->state() == QAbstractSocket::ConnectedState ||
        tcpSocket->state() == QAbstractSocket::ConnectingState) {
        tcpSocket->disconnectFromHost();
    }

    reconnecting = false;
}

void Client::sendMessage(const MessageProtocol::NetworkMessage &message)
{
    if (!isConnected()) {
        return;
    }

    QByteArray data = MessageProtocol::serializeMessage(message);
    tcpSocket->write(data);
    tcpSocket->flush();
}

void Client::sendUserStatus(const UserStatus &status)
{
    QJsonObject statusObj = status.toJson();
    MessageProtocol::NetworkMessage message = MessageProtocol::createUserStatusMessage(userIdentity.getUuid(), statusObj);
    sendMessage(message);
}

bool Client::isConnected() const
{
    return tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState;
}

QHostAddress Client::getServerAddress() const
{
    return serverAddress;
}

quint16 Client::getServerPort() const
{
    return serverPort;
}

UserIdentity Client::getUserIdentity() const
{
    return userIdentity;
}

void Client::onConnected()
{
    qInfo() << "已连接到服务器:" << serverAddress.toString() << ":" << serverPort;

    // 发送用户身份信息
    QJsonObject identityObj;
    identityObj["uuid"] = userIdentity.getUuid().toString();
    identityObj["nickname"] = userIdentity.getNickname();
    identityObj["deviceInfo"] = userIdentity.getDeviceInfo();

    MessageProtocol::NetworkMessage message;
    message.type = NetworkMessageType::UserDiscovery;
    message.messageId = QUuid::createUuid();
    message.senderId = userIdentity.getUuid();
    message.timestamp = QDateTime::currentDateTime();
    message.content = identityObj;

    sendMessage(message);

    // 启动定时器
    startTimers();

    // 如果是重连，发送重连信号
    if (reconnecting) {
        reconnecting = false;
        emit reconnected();
    } else {
        emit connected();
    }
}

void Client::onDisconnected()
{
    qInfo() << "与服务器断开连接:" << serverAddress.toString() << ":" << serverPort;

    stopTimers();
    emit disconnected();

    // 尝试重连
    reconnecting = true;
    reconnectTimer->start(5000); // 5秒后尝试重连
}

void Client::onError(QAbstractSocket::SocketError socketError)
{
    QString errorString = tcpSocket->errorString();
    qWarning() << "连接错误:" << errorString;

    emit connectionError(errorString);

    // 如果是连接错误且正在重连，忽略
    if (socketError == QAbstractSocket::ConnectionRefusedError && reconnecting) {
        return;
    }

    // 如果是连接错误且未在重连，开始重连
    if (socketError == QAbstractSocket::ConnectionRefusedError && !reconnecting) {
        reconnecting = true;
        reconnectTimer->start(5000); // 5秒后尝试重连
    }
}

void Client::onReadyRead()
{
    if (!tcpSocket) {
        return;
    }

    // 读取所有可用数据
    buffer.append(tcpSocket->readAll());

    // 尝试解析消息
    while (buffer.size() >= sizeof(MessageProtocol::MessageHeader)) {
        // 解析消息头
        MessageProtocol::MessageHeader header;
        QDataStream headerStream(buffer.left(sizeof(MessageProtocol::MessageHeader)));
        headerStream.setByteOrder(QDataStream::BigEndian);
        headerStream >> header.magic >> header.version >> header.contentSize;

        // 检查魔术数字和版本
        if (header.magic != MessageProtocol::MAGIC_NUMBER || header.version != MessageProtocol::PROTOCOL_VERSION) {
            // 无效消息，清空缓冲区
            buffer.clear();
            return;
        }

        // 检查数据是否完整
        if (buffer.size() < sizeof(MessageProtocol::MessageHeader) + header.contentSize) {
            // 数据不完整，等待更多数据
            return;
        }

        // 提取完整消息
        QByteArray messageData = buffer.left(sizeof(MessageProtocol::MessageHeader) + header.contentSize);
        buffer.remove(0, sizeof(MessageProtocol::MessageHeader) + header.contentSize);

        // 解析消息
        MessageProtocol::NetworkMessage message = MessageProtocol::deserializeMessage(messageData);

        // 处理心跳消息
        if (message.type == NetworkMessageType::Heartbeat) {
            continue;
        }

        // 发送信号
        emit messageReceived(message);
    }
}

void Client::sendHeartbeat()
{
    if (!isConnected()) {
        return;
    }

    MessageProtocol::NetworkMessage message = MessageProtocol::createHeartbeatMessage(userIdentity.getUuid());
    sendMessage(message);
}

void Client::attemptReconnect()
{
    if (isConnected()) {
        return;
    }

    qInfo() << "尝试重新连接到服务器:" << serverAddress.toString() << ":" << serverPort;
    tcpSocket->connectToHost(serverAddress, serverPort);
}

void Client::initSocket()
{
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &Client::onError);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &Client::onReadyRead);

    // 创建定时器
    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &Client::sendHeartbeat);

    reconnectTimer = new QTimer(this);
    connect(reconnectTimer, &QTimer::timeout, this, &Client::attemptReconnect);
}

void Client::stopTimers()
{
    if (heartbeatTimer) {
        heartbeatTimer->stop();
    }

    if (reconnectTimer) {
        reconnectTimer->stop();
    }
}

void Client::startTimers()
{
    if (heartbeatTimer) {
        heartbeatTimer->start(Constants::HEARTBEAT_INTERVAL_MS);
    }
}

} // namespace LocalNetworkApp