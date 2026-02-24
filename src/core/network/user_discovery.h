#ifndef USER_DISCOVERY_H
#define USER_DISCOVERY_H

#include <QObject>
#include <QtNetwork/QUdpSocket>
#include <QTimer>
#include <QtNetwork/QHostAddress>
#include <QMap>
#include <QDateTime>
#include "../user/userIdentity.h"
#include "../user/user_status.h"
#include "../utils/constants.h"

namespace LocalNetworkApp {

struct DiscoveredUser {
    QUuid userId;              // 用户ID
    QString nickname;          // 用户昵称
    QHostAddress address;      // 用户地址
    quint16 port;              // 用户端口
    UserState state;           // 用户状态
    QDateTime lastSeen;        // 最后在线时间
};

class UserDiscovery : public QObject {
    Q_OBJECT

public:
    UserDiscovery(const UserIdentity &userIdentity, QObject *parent = nullptr);
    ~UserDiscovery();

    // 开始用户发现
    bool startDiscovery(quint16 port = Constants::DEFAULT_UDP_PORT);

    // 停止用户发现
    void stopDiscovery();

    // 获取已发现的用户列表
    QList<DiscoveredUser> getDiscoveredUsers() const;

    // 获取特定用户信息
    DiscoveredUser getDiscoveredUser(QUuid userId) const;

    // 检查用户是否在线
    bool isUserOnline(QUuid userId) const;

    // 设置用户状态
    void setUserState(UserState state);

    // 获取用户状态
    UserState getUserState() const;

signals:
    // 发现新用户
    void userDiscovered(const DiscoveredUser &user);

    // 用户离线
    void userLost(QUuid userId);

    // 用户状态变更
    void userStateChanged(QUuid userId, UserState state);

private slots:
    // 发送广播
    void sendBroadcast();

    // 处理接收到的数据
    void processPendingDatagrams();

    // 清理超时用户
    void cleanupTimeoutUsers();

private:
    QUdpSocket *udpSocket;                     // UDP Socket
    UserIdentity userIdentity;                 // 用户身份
    QTimer *broadcastTimer;                    // 广播定时器
    QTimer *cleanupTimer;                      // 清理定时器
    QMap<QUuid, DiscoveredUser> discoveredUsers; // 已发现的用户
    UserState currentState;                    // 当前用户状态

    // 初始化Socket
    void initSocket();

    // 停止定时器
    void stopTimers();

    // 启动定时器
    void startTimers();

    // 发送用户发现消息
    void sendUserDiscoveryMessage(const QHostAddress &address, quint16 port);
};

} // namespace LocalNetworkApp

#endif // USER_DISCOVERY_H
