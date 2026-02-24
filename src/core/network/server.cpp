#include "server.h"
#include <QDataStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "../utils/constants.h"

namespace LocalNetworkApp {

ClientConnection::ClientConnection(QTcpSocket *socket, QUuid clientId, QObject *parent) :
    QObject(parent),
    socket(socket),
    clientId(clientId)
{
    connect(socket, &QTcpSocket::readyRead, this, &ClientConnection::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ClientConnection::onDisconnected);
}

ClientConnection::~ClientConnection()
{
    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
}

QUuid ClientConnection::getClientId() const
{
    return clientId;
}

QHostAddress ClientConnection::getClientAddress() const
{
    return socket->peerAddress();
}

quint16 ClientConnection::getClientPort() const
{
    return socket->peerPort();
}

void ClientConnection::sendMessage(const MessageProtocol::NetworkMessage &message)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QByteArray data = MessageProtocol::serializeMessage(message);
    socket->write(data);
    socket->flush();
}

void ClientConnection::close()
{
    if (socket) {
        socket->disconnectFromHost();
    }
}

void ClientConnection::onReadyRead()
{
    if (!socket) {
        return;
    }

    // 读取所有可用数据
    buffer.append(socket->readAll());

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

        // 发送信号
        emit messageReceived(message);
    }
}

void ClientConnection::onDisconnected()
{
    emit disconnected(clientId);
    deleteLater();
}

Server::Server(ContactManager *contactManager, QObject *parent) :
    QObject(parent),
    contactManager(contactManager)
{
    tcpServer = new QTcpServer(this);
    connect(tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

Server::~Server()
{
    stop();
    delete tcpServer;
}

bool Server::start(quint16 port)
{
    if (tcpServer->isListening()) {
        return true;
    }

    if (!tcpServer->listen(QHostAddress::Any, port)) {
        qWarning() << "服务器启动失败:" << tcpServer->errorString();
        return false;
    }

    qInfo() << "服务器已启动，监听端口:" << port;
    emit serverStarted(port);
    return true;
}

void Server::stop()
{
    if (!tcpServer->isListening()) {
        return;
    }

    // 关闭所有客户端连接
    for (auto connection : clients) {
        connection->close();
    }
    clients.clear();

    // 停止服务器
    tcpServer->close();
    qInfo() << "服务器已停止";
    emit serverStopped();
}

void Server::broadcastUserStatus(const UserStatus &status)
{
    QJsonObject statusObj = status.toJson();
    MessageProtocol::NetworkMessage message = MessageProtocol::createUserStatusMessage(status.getUserId(), statusObj);
    broadcastMessage(message);
}

void Server::sendMessageToClient(QUuid clientId, const MessageProtocol::NetworkMessage &message)
{
    if (clients.contains(clientId)) {
        clients[clientId]->sendMessage(message);
    }
}

void Server::broadcastMessage(const MessageProtocol::NetworkMessage &message)
{
    for (auto connection : clients) {
        connection->sendMessage(message);
    }
}

QList<QUuid> Server::getOnlineClients() const
{
    return clients.keys();
}

bool Server::isClientOnline(QUuid clientId) const
{
    return clients.contains(clientId);
}

void Server::onNewConnection()
{
    QTcpSocket *socket = tcpServer->nextPendingConnection();
    if (!socket) {
        return;
    }

    // 注意：这里我们假设客户端会在连接后立即发送其ID
    // 在实际应用中，可能需要一个握手过程来验证客户端身份

    // 暂时使用临时ID，等待客户端发送其真实ID
    QUuid tempId = QUuid::createUuid();
    ClientConnection *connection = new ClientConnection(socket, tempId, this);

    connect(connection, &ClientConnection::messageReceived, this, &Server::onClientMessageReceived);
    connect(connection, &ClientConnection::disconnected, this, &Server::onClientDisconnected);

    clients[tempId] = connection;

    qInfo() << "新客户端连接:" << socket->peerAddress().toString() << ":" << socket->peerPort();
}

void Server::onClientDisconnected(QUuid clientId)
{
    if (clients.contains(clientId)) {
        ClientConnection *connection = clients.take(clientId);
        qInfo() << "客户端断开连接:" << clientId.toString();
        emit clientDisconnected(clientId);
        connection->deleteLater();
    }
}

void Server::onClientMessageReceived(const MessageProtocol::NetworkMessage &message)
{
    QUuid senderId = message.senderId;
    ClientConnection *connection = qobject_cast<ClientConnection*>(sender());

    if (!connection) {
        return;
    }

    // 如果是第一个消息，更新客户端ID
    if (connection->getClientId() != senderId) {
        QUuid oldId = connection->getClientId();
        clients.remove(oldId);
        clients[senderId] = connection;

        // 发送信号通知客户端连接
        emit clientConnected(senderId);
    }

    // 发送信号通知消息接收
    emit messageReceived(message, senderId);
}

} // namespace LocalNetworkApp