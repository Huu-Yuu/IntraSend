#include "userIdentity.h"
#include <QSettings>
#include <QtNetwork/QNetworkInterface>
#include <QStringList>
#include <QDir>
#include <QHostInfo>
#include "../utils/constants.h"

namespace LocalNetworkApp {

UserIdentity::UserIdentity() :
    uuid(generateUuidFromDevice()),
    nickname(Constants::DEFAULT_NICKNAME),
    deviceInfo(getDeviceInfoString())
{
}

QUuid UserIdentity::getUuid() const
{
    return uuid;
}

QString UserIdentity::getNickname() const
{
    return nickname;
}

void UserIdentity::setNickname(const QString &nickname)
{
    this->nickname = nickname;
}

QString UserIdentity::getDeviceInfo() const
{
    return deviceInfo;
}

QUuid UserIdentity::generateUuidFromDevice()
{
    QString deviceInfo = getDeviceInfoString();
    return QUuid::createUuidV5(QUuid(), deviceInfo);
}

bool UserIdentity::saveToLocal() const
{
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    settings.setValue("user/uuid", uuid.toString());
    settings.setValue("user/nickname", nickname);
    settings.setValue("user/deviceInfo", deviceInfo);
    return settings.status() == QSettings::NoError;
}

UserIdentity UserIdentity::loadFromLocal()
{
    UserIdentity identity;
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    
    if (settings.contains("user/uuid")) {
        identity.uuid = QUuid(settings.value("user/uuid").toString());
    }
    
    if (settings.contains("user/nickname")) {
        identity.nickname = settings.value("user/nickname").toString();
    }
    
    if (settings.contains("user/deviceInfo")) {
        identity.deviceInfo = settings.value("user/deviceInfo").toString();
    }
    
    return identity;
}

QString UserIdentity::getDeviceInfoString()
{
    QStringList macAddresses;
    
    // 获取所有网络接口的MAC地址
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack)) {
            QString mac = interface.hardwareAddress();
            if (!mac.isEmpty()) {
                macAddresses.append(mac);
            }
        }
    }
    
    // 排序MAC地址以确保一致性
    macAddresses.sort();
    
    // 组合成一个字符串
    QString deviceInfo = macAddresses.join(",");
    
    // 如果没有找到MAC地址，使用主机名作为备选
    if (deviceInfo.isEmpty()) {
        deviceInfo = QHostInfo::localHostName();
    }
    
    return deviceInfo;
}

} // namespace LocalNetworkApp
