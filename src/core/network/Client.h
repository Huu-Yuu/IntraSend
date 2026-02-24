#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QUuid>
#include <QTimer>
#include <QtNetwork/QHostAddress>
#include "message_protocol.h"
#include "../user/user_status.h"
#include "../user/userIdentity.h"

namespace LocalNetworkApp {

class Client : public QObject {
    Q_OBJECT

public:
    Client(const UserIdentity &userIdentity, QObject *parent = nullptr);
    ~Client();

    // 连接到服务器
    bool connectToServer(const QHostAddress &serverAddress, quint16 serverPort = Constants::DEFAULT_TCP_PORT);

    // 断开连接
    void disconnectFromServer();

    // 发送消息
    void sendMessage(const MessageProtocol::NetworkMessage &message);

    // 发送用户状态
    void sendUserStatus(const UserStatus &status);

    // 获取连接状态
    bool isConnected() const;

    // 获取服务器地址
    QHostAddress getServerAddress() const;

    // 获取服务器端口
    quint16 getServerPort() const;

    // 获取用户身份
    UserIdentity getUserIdentity() const;

signals:
    // 连接成功
    void connected();

    // 连接断开
    void disconnected();

    // 连接错误
    void connectionError(const QString &errorString);

    // 收到消息
    void messageReceived(const MessageProtocol::NetworkMessage &message);

    // 重连成功
    void reconnected();

private slots:
    // 连接成功
    void onConnected();

    // 连接断开
    void onDisconnected();

    // 连接错误
    void onError(QAbstractSocket::SocketError socketError);

    // 读取数据
    void onReadyRead();

    // 发送心跳
    void sendHeartbeat();

    // 尝试重连
    void attemptReconnect();

private:
    QTcpSocket *tcpSocket;               // TCP Socket
    UserIdentity userIdentity;           // 用户身份
    QHostAddress serverAddress;          // 服务器地址
    quint16 serverPort;                  // 服务器端口
    QByteArray buffer;                   // 数据缓冲区
    QTimer *heartbeatTimer;              // 心跳定时器
    QTimer *reconnectTimer;              // 重连定时器
    bool reconnecting;                   // 是否正在重连

    // 初始化Socket
    void initSocket();

    // 停止定时器
    void stopTimers();

    // 启动定时器
    void startTimers();
};

} // namespace LocalNetworkApp

#endif // CLIENT_H
