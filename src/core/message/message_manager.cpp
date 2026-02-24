#include "message_manager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QDateTime>
#include "../utils/constants.h"

namespace LocalNetworkApp {

MessageManager::MessageManager(QObject *parent) :
    QObject(parent),
    incognitoMode(false),
    unreadCount(0)
{
    loadMessageHistory();
}

void MessageManager::setIncognitoMode(bool enabled)
{
    incognitoMode = enabled;
    if (enabled) {
        // 启用无痕模式时清除所有历史记录
        clearAllMessageHistory();
    }
}

bool MessageManager::isIncognitoMode() const
{
    return incognitoMode;
}

bool MessageManager::sendMessage(const Message &message, const UserStatus &receiverStatus)
{
    // 检查接收者状态，如果是勿扰模式，则不发送聊天消息
    if (receiverStatus.getState() == UserState::DoNotDisturb && 
        message.getType() == MessageType::Text) {
        emit messageSendFailed(message, "接收者处于勿扰模式，无法发送消息");
        return false;
    }

    // 检查是否在无痕模式下
    if (!incognitoMode) {
        // 保存消息到历史记录
        messageHistory[message.getReceiverId()].append(message);
        saveMessageHistory();
    }

    emit messageSent(message);
    return true;
}

void MessageManager::receiveMessage(const Message &message)
{
    // 检查是否在无痕模式下
    if (!incognitoMode) {
        // 保存消息到历史记录
        messageHistory[message.getSenderId()].append(message);
        
        // 如果消息未读，增加未读计数
        if (!message.isRead()) {
            unreadCount++;
            contactUnreadCount[message.getSenderId()]++;
            emit unreadMessageCountChanged(unreadCount);
        }
        
        saveMessageHistory();
    }

    emit messageReceived(message);
}

QList<Message> MessageManager::getMessageHistory(QUuid contactId) const
{
    return messageHistory.value(contactId);
}

void MessageManager::clearMessageHistory(QUuid contactId)
{
    if (messageHistory.contains(contactId)) {
        // 减去该联系人的未读消息数
        unreadCount -= contactUnreadCount.value(contactId, 0);
        contactUnreadCount.remove(contactId);
        emit unreadMessageCountChanged(unreadCount);
        
        messageHistory.remove(contactId);
        saveMessageHistory();
    }
}

void MessageManager::clearAllMessageHistory()
{
    messageHistory.clear();
    unreadCount = 0;
    contactUnreadCount.clear();
    emit unreadMessageCountChanged(unreadCount);
    saveMessageHistory();
}

bool MessageManager::saveMessageHistory() const
{
    if (incognitoMode) {
        return true; // 无痕模式下不保存
    }

    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    QJsonObject root;
    
    // 保存每个联系人的消息历史
    for (auto it = messageHistory.constBegin(); it != messageHistory.constEnd(); ++it) {
        QJsonArray messagesArray;
        for (const auto &message : it.value()) {
            messagesArray.append(message.toJson());
        }
        root[it.key().toString()] = messagesArray;
    }
    
    // 保存未读消息计数
    QJsonObject unreadObj;
    for (auto it = contactUnreadCount.constBegin(); it != contactUnreadCount.constEnd(); ++it) {
        unreadObj[it.key().toString()] = it.value();
    }
    root["unreadCount"] = unreadObj;
    root["totalUnread"] = unreadCount;
    
    settings.setValue("messages/history", QString::fromUtf8(QJsonDocument(root).toJson()));
    return settings.status() == QSettings::NoError;
}

bool MessageManager::loadMessageHistory()
{
    if (incognitoMode) {
        return true; // 无痕模式下不加载
    }

    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    
    if (!settings.contains("messages/history")) {
        return false;
    }
    
    QByteArray jsonData = settings.value("messages/history").toByteArray();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject root = doc.object();
    
    messageHistory.clear();
    contactUnreadCount.clear();
    unreadCount = 0;
    
    // 加载每个联系人的消息历史
    for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
        if (it.key() == "unreadCount" || it.key() == "totalUnread") {
            continue; // 跳过特殊字段
        }
        
        QUuid contactId(it.key());
        QJsonArray messagesArray = it.value().toArray();
        QList<Message> messages;
        
        for (const auto &value : messagesArray) {
            Message message(value.toObject());
            messages.append(message);
        }
        
        messageHistory[contactId] = messages;
    }
    
    // 加载未读消息计数
    if (root.contains("unreadCount")) {
        QJsonObject unreadObj = root["unreadCount"].toObject();
        for (auto it = unreadObj.constBegin(); it != unreadObj.constEnd(); ++it) {
            contactUnreadCount[QUuid(it.key())] = it.value().toInt();
        }
    }
    
    if (root.contains("totalUnread")) {
        unreadCount = root["totalUnread"].toInt();
    } else {
        // 如果没有保存总未读数，重新计算
        unreadCount = 0;
        for (int count : contactUnreadCount) {
            unreadCount += count;
        }
    }
    
    emit unreadMessageCountChanged(unreadCount);
    return true;
}

void MessageManager::markAsRead(QUuid messageId)
{
    for (auto it = messageHistory.begin(); it != messageHistory.end(); ++it) {
        QList<Message> &messages = it.value();
        for (int i = 0; i < messages.size(); ++i) {
            if (messages[i].getMessageId() == messageId && !messages[i].isRead()) {
                messages[i].setRead(true);
                
                // 更新未读计数
                unreadCount--;
                contactUnreadCount[it.key()]--;
                if (contactUnreadCount[it.key()] <= 0) {
                    contactUnreadCount.remove(it.key());
                }
                
                emit unreadMessageCountChanged(unreadCount);
                saveMessageHistory();
                return;
            }
        }
    }
}

int MessageManager::getUnreadMessageCount() const
{
    return unreadCount;
}

int MessageManager::getUnreadMessageCount(QUuid contactId) const
{
    return contactUnreadCount.value(contactId, 0);
}

} // namespace LocalNetworkApp