#ifndef CONTACT_MANAGER_H
#define CONTACT_MANAGER_H

#include <QUuid>
#include <QMap>
#include <QSet>
#include <QString>
#include <QJsonObject>
#include "core/utils/enums.h"
namespace LocalNetworkApp {

struct ContactInfo {
    QUuid id;          // 联系人ID
    QString nickname;  // 联系人昵称
    QString remark;    // 备注名称
    UserState state;   // 联系人状态
    QDateTime lastSeen; // 最后在线时间

    // 转换为JSON格式
    QJsonObject toJson() const;

    // 从JSON格式创建
    static ContactInfo fromJson(const QJsonObject &json);
};

class ContactManager {
public:
    ContactManager();
    ~ContactManager() = default;

    // 添加联系人
    void addContact(const ContactInfo &contact);

    // 删除联系人
    void removeContact(QUuid contactId);

    // 获取联系人信息
    ContactInfo getContact(QUuid contactId) const;

    // 获取所有联系人
    QList<ContactInfo> getAllContacts() const;

    // 设置联系人备注
    void setContactRemark(QUuid contactId, const QString &remark);

    // 更新联系人状态
    void updateContactState(QUuid contactId, UserState state);

    // 添加到黑名单
    void addToBlacklist(QUuid contactId);

    // 从黑名单移除
    void removeFromBlacklist(QUuid contactId);

    // 检查是否在黑名单中
    bool isInBlacklist(QUuid contactId) const;

    // 添加到白名单
    void addToWhitelist(QUuid contactId);

    // 从白名单移除
    void removeFromWhitelist(QUuid contactId);

    // 检查是否在白名单中
    bool isInWhitelist(QUuid contactId) const;

    // 保存到本地
    bool saveToLocal() const;

    // 从本地加载
    static ContactManager loadFromLocal();

    // 转换为JSON格式
    QJsonObject toJson() const;

    // 从JSON格式创建
    static ContactManager fromJson(const QJsonObject &json);

private:
    QMap<QUuid, ContactInfo> contacts;  // 联系人列表
    QSet<QUuid> blacklist;              // 黑名单
    QSet<QUuid> whitelist;              // 白名单
};

} // namespace LocalNetworkApp

#endif // CONTACT_MANAGER_H
