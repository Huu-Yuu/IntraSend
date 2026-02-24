#include "user_discovery.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>
#include <QDebug>
#include <QtNetwork/QNetworkInterface>

namespace LocalNetworkApp {

UserDiscovery::UserDiscovery(const UserIdentity &userIdentity, QObject *parent) :
    QObject(parent),
    userIdentity(userIdentity),
    currentState(UserState::Online)
{
    initSocket();
}

UserDiscovery::~UserDiscovery()
{
    stopDiscovery();
    delete udpSocket;
}

bool UserDiscovery::startDiscovery(quint16 port)
{
    if (udpSocket->isOpen()) {
        return true;
    }

    // 绑定到指定端口
    if (!udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "无法绑定到端口:" << port << udpSocket->errorString();
        return false;
    }

    // 启动定时器
    startTimers();

    // 发送初始广播
    sendBroadcast();

    qInfo() << "用户发现服务已启动，监听端口:" << port;
    return true;
}

void UserDiscovery::stopDiscovery()
{
    stopTimers();

    if (udpSocket->isOpen()) {
        udpSocket->close();
    }

    discoveredUsers.clear();
}

QList<DiscoveredUser> UserDiscovery::getDiscoveredUsers() const
{
    QList<DiscoveredUser> users;

    // 只返回在线状态的用户
    for (const auto &user : discoveredUsers) {
        if (user.state != UserState::Invisible) {
            users.append(user);
        }
    }

    return users;
}

DiscoveredUser UserDiscovery::getDiscoveredUser(QUuid userId) const
{
    return discoveredUsers.value(userId);
}

bool UserDiscovery::isUserOnline(QUuid userId) const
{
    if (!discoveredUsers.contains(userId)) {
        return false;
    }

    const DiscoveredUser &user = discoveredUsers[userId];
    return user.state != UserState::Invisible && 
           user.lastSeen.secsTo(QDateTime::currentDateTime()) < Constants::USER_TIMEOUT_MS / 1000;
}

void UserDiscovery::setUserState(UserState state)
{
    if (currentState != state) {
        currentState = state;
        sendBroadcast();
    }
}

UserState UserDiscovery::getUserState() const
{
    return currentState;
}

void UserDiscovery::sendBroadcast()
{
    // 创建用户发现消息
    QJsonObject discoveryObj;
    discoveryObj["userId"] = userIdentity.getUuid().toString();
    discoveryObj["nickname"] = userIdentity.getNickname();
    discoveryObj["state"] = static_cast<int>(currentState);
    discoveryObj["tcpPort"] = Constants::DEFAULT_TCP_PORT;

    QJsonDocument doc(discoveryObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // 发送到广播地址
    udpSocket->writeDatagram(data, QHostAddress::Broadcast, Constants::DEFAULT_UDP_PORT);

    // 发送到所有本地网络接口的广播地址
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp && 
            interface.flags() & QNetworkInterface::IsRunning && 
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {
            
            const QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : entries) {
                QHostAddress broadcastAddress = entry.broadcast();
                if (!broadcastAddress.isNull()) {
                    udpSocket->writeDatagram(data, broadcastAddress, Constants::DEFAULT_UDP_PORT);
                }
            }
        }
    }
}

void UserDiscovery::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());

        QHostAddress senderAddress;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        // 解析消息
        QJsonDocument doc = QJsonDocument::fromJson(datagram);
        if (!doc.isObject()) {
            continue;
        }

        QJsonObject discoveryObj = doc.object();

        // 检查必要字段
        if (!discoveryObj.contains("userId") || 
            !discoveryObj.contains("nickname") || 
            !discoveryObj.contains("state") || 
            !discoveryObj.contains("tcpPort")) {
            continue;
        }

        QUuid userId(discoveryObj["userId"].toString());

        // 忽略自己
        if (userId == userIdentity.getUuid()) {
            continue;
        }

        // 创建或更新用户信息
        DiscoveredUser user;
        user.userId = userId;
        user.nickname = discoveryObj["nickname"].toString();
        user.address = senderAddress;
        user.port = discoveryObj["tcpPort"].toInt();
        user.state = static_cast<UserState>(discoveryObj["state"].toInt());
        user.lastSeen = QDateTime::currentDateTime();

        bool isNewUser = !discoveredUsers.contains(userId);
        bool stateChanged = false;

        if (!isNewUser) {
            const DiscoveredUser &oldUser = discoveredUsers[userId];
            stateChanged = (oldUser.state != user.state);
        }

        // 更新用户列表
        discoveredUsers[userId] = user;

        // 发送信号
        if (isNewUser) {
            emit userDiscovered(user);
        } else if (stateChanged) {
            emit userStateChanged(userId, user.state);
        }

        // 回复自己的信息
        sendUserDiscoveryMessage(senderAddress, senderPort);
    }
}

void UserDiscovery::cleanupTimeoutUsers()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<QUuid> toRemove;

    for (auto it = discoveredUsers.constBegin(); it != discoveredUsers.constEnd(); ++it) {
        const DiscoveredUser &user = it.value();
        if (user.lastSeen.secsTo(now) > Constants::USER_TIMEOUT_MS / 1000) {
            toRemove.append(it.key());
        }
    }

    for (const QUuid &userId : toRemove) {
        discoveredUsers.remove(userId);
        emit userLost(userId);
    }
}

void UserDiscovery::initSocket()
{
    udpSocket = new QUdpSocket(this);

    connect(udpSocket, &QUdpSocket::readyRead, this, &UserDiscovery::processPendingDatagrams);

    // 创建定时器
    broadcastTimer = new QTimer(this);
    connect(broadcastTimer, &QTimer::timeout, this, &UserDiscovery::sendBroadcast);

    cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, &UserDiscovery::cleanupTimeoutUsers);
}

void UserDiscovery::stopTimers()
{
    if (broadcastTimer) {
        broadcastTimer->stop();
    }

    if (cleanupTimer) {
        cleanupTimer->stop();
    }
}

void UserDiscovery::startTimers()
{
    if (broadcastTimer) {
        broadcastTimer->start(Constants::HEARTBEAT_INTERVAL_MS);
    }

    if (cleanupTimer) {
        cleanupTimer->start(Constants::USER_TIMEOUT_MS / 2);
    }
}

void UserDiscovery::sendUserDiscoveryMessage(const QHostAddress &address, quint16 port)
{
    // 创建用户发现消息
    QJsonObject discoveryObj;
    discoveryObj["userId"] = userIdentity.getUuid().toString();
    discoveryObj["nickname"] = userIdentity.getNickname();
    discoveryObj["state"] = static_cast<int>(currentState);
    discoveryObj["tcpPort"] = Constants::DEFAULT_TCP_PORT;

    QJsonDocument doc(discoveryObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    // 发送到指定地址
    udpSocket->writeDatagram(data, address, port);
}

} // namespace LocalNetworkApp
