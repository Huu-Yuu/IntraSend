#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QMap>
#include <QUuid>
#include <QtNetwork/QHostAddress>
#include "message_protocol.h"
#include "../user/user_status.h"
#include "../user/contact_manager.h"

namespace LocalNetworkApp {

class ClientConnection : public QObject {
    Q_OBJECT

public:
    ClientConnection(QTcpSocket *socket, QUuid clientId, QObject *parent = nullptr);
    ~ClientConnection();

    // 获取客户端ID
    QUuid getClientId() const;

    // 获取客户端地址
    QHostAddress getClientAddress() const;

    // 获取客户端端口
    quint16 getClientPort() const;

    // 发送消息
    void sendMessage(const MessageProtocol::NetworkMessage &message);

    // 关闭连接
    void close();

signals:
    // 收到消息
    void messageReceived(const MessageProtocol::NetworkMessage &message);

    // 连接断开
    void disconnected(QUuid clientId);

private slots:
    // 读取数据
    void onReadyRead();

    // 连接断开
    void onDisconnected();

private:
    QTcpSocket *socket;       // 客户端Socket
    QUuid clientId;           // 客户端ID
    QByteArray buffer;        // 数据缓冲区
};

class Server : public QObject {
    Q_OBJECT

public:
    Server(ContactManager *contactManager, QObject *parent = nullptr);
    ~Server();

    // 启动服务器
    bool start(quint16 port = Constants::DEFAULT_TCP_PORT);

    // 停止服务器
    void stop();

    // 广播用户状态
    void broadcastUserStatus(const UserStatus &status);

    // 发送消息给特定客户端
    void sendMessageToClient(QUuid clientId, const MessageProtocol::NetworkMessage &message);

    // 广播消息给所有客户端
    void broadcastMessage(const MessageProtocol::NetworkMessage &message);

    // 获取在线客户端列表
    QList<QUuid> getOnlineClients() const;

    // 检查客户端是否在线
    bool isClientOnline(QUuid clientId) const;

signals:
    // 收到消息
    void messageReceived(const MessageProtocol::NetworkMessage &message, QUuid senderId);

    // 客户端连接
    void clientConnected(QUuid clientId);

    // 客户端断开连接
    void clientDisconnected(QUuid clientId);

    // 服务器启动
    void serverStarted(quint16 port);

    // 服务器停止
    void serverStopped();

private slots:
    // 新客户端连接
    void onNewConnection();

    // 客户端断开连接
    void onClientDisconnected(QUuid clientId);

    // 处理客户端消息
    void onClientMessageReceived(const MessageProtocol::NetworkMessage &message);

private:
    QTcpServer *tcpServer;                             // TCP服务器
    QMap<QUuid, ClientConnection*> clients;            // 客户端连接映射
    ContactManager *contactManager;                    // 联系人管理器
};

} // namespace LocalNetworkApp

#endif // SERVER_H
