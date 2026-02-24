#include "contact_manager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include <QDateTime>
#include "../utils/constants.h"

namespace LocalNetworkApp {

QJsonObject ContactInfo::toJson() const
{
    QJsonObject json;
    json["id"] = id.toString();
    json["nickname"] = nickname;
    json["remark"] = remark;
    json["state"] = static_cast<int>(state);
    json["lastSeen"] = lastSeen.toString(Qt::ISODate);
    return json;
}

ContactInfo ContactInfo::fromJson(const QJsonObject &json)
{
    ContactInfo info;
    info.id = QUuid(json["id"].toString());
    info.nickname = json["nickname"].toString();
    info.remark = json["remark"].toString();
    info.state = static_cast<UserState>(json["state"].toInt());
    info.lastSeen = QDateTime::fromString(json["lastSeen"].toString(), Qt::ISODate);
    return info;
}

ContactManager::ContactManager()
{
}

void ContactManager::addContact(const ContactInfo &contact)
{
    contacts[contact.id] = contact;
}

void ContactManager::removeContact(QUuid contactId)
{
    contacts.remove(contactId);
    blacklist.remove(contactId);
    whitelist.remove(contactId);
}

ContactInfo ContactManager::getContact(QUuid contactId) const
{
    return contacts.value(contactId);
}

QList<ContactInfo> ContactManager::getAllContacts() const
{
    return contacts.values();
}

void ContactManager::setContactRemark(QUuid contactId, const QString &remark)
{
    if (contacts.contains(contactId)) {
        contacts[contactId].remark = remark;
    }
}

void ContactManager::updateContactState(QUuid contactId, UserState state)
{
    if (contacts.contains(contactId)) {
        contacts[contactId].state = state;
        if (state != UserState::Invisible) {
            contacts[contactId].lastSeen = QDateTime::currentDateTime();
        }
    }
}

void ContactManager::addToBlacklist(QUuid contactId)
{
    blacklist.insert(contactId);
    whitelist.remove(contactId); // 从白名单中移除
}

void ContactManager::removeFromBlacklist(QUuid contactId)
{
    blacklist.remove(contactId);
}

bool ContactManager::isInBlacklist(QUuid contactId) const
{
    return blacklist.contains(contactId);
}

void ContactManager::addToWhitelist(QUuid contactId)
{
    whitelist.insert(contactId);
    blacklist.remove(contactId); // 从黑名单中移除
}

void ContactManager::removeFromWhitelist(QUuid contactId)
{
    whitelist.remove(contactId);
}

bool ContactManager::isInWhitelist(QUuid contactId) const
{
    return whitelist.contains(contactId);
}

bool ContactManager::saveToLocal() const
{
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    
    // 保存联系人列表
    QJsonArray contactsArray;
    for (const auto &contact : contacts) {
        contactsArray.append(contact.toJson());
    }
    settings.setValue("contacts/list", QString::fromUtf8(QJsonDocument(contactsArray).toJson()));
    
    // 保存黑名单
    QJsonArray blacklistArray;
    for (const auto &id : blacklist) {
        blacklistArray.append(id.toString());
    }
    settings.setValue("contacts/blacklist", QString::fromUtf8(QJsonDocument(blacklistArray).toJson()));
    
    // 保存白名单
    QJsonArray whitelistArray;
    for (const auto &id : whitelist) {
        whitelistArray.append(id.toString());
    }
    settings.setValue("contacts/whitelist", QString::fromUtf8(QJsonDocument(whitelistArray).toJson()));
    
    return settings.status() == QSettings::NoError;
}

ContactManager ContactManager::loadFromLocal()
{
    ContactManager manager;
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    
    // 加载联系人列表
    if (settings.contains("contacts/list")) {
        QByteArray jsonData = settings.value("contacts/list").toByteArray();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QJsonArray contactsArray = doc.array();
        
        for (const auto &value : contactsArray) {
            QJsonObject contactJson = value.toObject();
            ContactInfo contact = ContactInfo::fromJson(contactJson);
            manager.contacts[contact.id] = contact;
        }
    }
    
    // 加载黑名单
    if (settings.contains("contacts/blacklist")) {
        QByteArray jsonData = settings.value("contacts/blacklist").toByteArray();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QJsonArray blacklistArray = doc.array();
        
        for (const auto &value : blacklistArray) {
            manager.blacklist.insert(QUuid(value.toString()));
        }
    }
    
    // 加载白名单
    if (settings.contains("contacts/whitelist")) {
        QByteArray jsonData = settings.value("contacts/whitelist").toByteArray();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QJsonArray whitelistArray = doc.array();
        
        for (const auto &value : whitelistArray) {
            manager.whitelist.insert(QUuid(value.toString()));
        }
    }
    
    return manager;
}

QJsonObject ContactManager::toJson() const
{
    QJsonObject json;
    
    // 保存联系人列表
    QJsonArray contactsArray;
    for (const auto &contact : contacts) {
        contactsArray.append(contact.toJson());
    }
    json["contacts"] = contactsArray;
    
    // 保存黑名单
    QJsonArray blacklistArray;
    for (const auto &id : blacklist) {
        blacklistArray.append(id.toString());
    }
    json["blacklist"] = blacklistArray;
    
    // 保存白名单
    QJsonArray whitelistArray;
    for (const auto &id : whitelist) {
        whitelistArray.append(id.toString());
    }
    json["whitelist"] = whitelistArray;
    
    return json;
}

ContactManager ContactManager::fromJson(const QJsonObject &json)
{
    ContactManager manager;
    
    // 加载联系人列表
    if (json.contains("contacts")) {
        QJsonArray contactsArray = json["contacts"].toArray();
        for (const auto &value : contactsArray) {
            QJsonObject contactJson = value.toObject();
            ContactInfo contact = ContactInfo::fromJson(contactJson);
            manager.contacts[contact.id] = contact;
        }
    }
    
    // 加载黑名单
    if (json.contains("blacklist")) {
        QJsonArray blacklistArray = json["blacklist"].toArray();
        for (const auto &value : blacklistArray) {
            manager.blacklist.insert(QUuid(value.toString()));
        }
    }
    
    // 加载白名单
    if (json.contains("whitelist")) {
        QJsonArray whitelistArray = json["whitelist"].toArray();
        for (const auto &value : whitelistArray) {
            manager.whitelist.insert(QUuid(value.toString()));
        }
    }
    
    return manager;
}

} // namespace LocalNetworkApp