#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QUuid>
#include "message.h"
#include "../user/user_status.h"

namespace LocalNetworkApp {

class MessageManager : public QObject {
    Q_OBJECT

public:
    MessageManager(QObject *parent = nullptr);
    ~MessageManager() = default;

    // 设置是否启用无痕模式
    void setIncognitoMode(bool enabled);

    // 获取无痕模式状态
    bool isIncognitoMode() const;

    // 发送消息
    bool sendMessage(const Message &message, const UserStatus &receiverStatus);

    // 接收消息
    void receiveMessage(const Message &message);

    // 获取与特定联系人的消息历史
    QList<Message> getMessageHistory(QUuid contactId) const;

    // 清除与特定联系人的消息历史
    void clearMessageHistory(QUuid contactId);

    // 清除所有消息历史
    void clearAllMessageHistory();

    // 保存消息历史到本地
    bool saveMessageHistory() const;

    // 从本地加载消息历史
    bool loadMessageHistory();

    // 设置消息为已读
    void markAsRead(QUuid messageId);

    // 获取未读消息数量
    int getUnreadMessageCount() const;

    // 获取与特定联系人的未读消息数量
    int getUnreadMessageCount(QUuid contactId) const;

signals:
    // 当收到新消息时发出
    void messageReceived(const Message &message);

    // 当消息发送成功时发出
    void messageSent(const Message &message);

    // 当消息发送失败时发出
    void messageSendFailed(const Message &message, const QString &error);

    // 当未读消息数量变化时发出
    void unreadMessageCountChanged(int count);

private:
    QMap<QUuid, QList<Message>> messageHistory; // 消息历史，按联系人ID组织
    bool incognitoMode; // 无痕模式标志
    int unreadCount; // 未读消息总数
    QMap<QUuid, int> contactUnreadCount; // 每个联系人的未读消息数
};

} // namespace LocalNetworkApp

#endif // MESSAGE_MANAGER_H
