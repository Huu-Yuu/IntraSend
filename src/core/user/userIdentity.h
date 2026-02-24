#ifndef USERIDENTITY_H
#define USERIDENTITY_H

#include <QUuid>
#include <QString>

namespace LocalNetworkApp {

class UserIdentity {
public:
    UserIdentity();
    ~UserIdentity() = default;

    // 获取用户唯一标识符
    QUuid getUuid() const;

    // 获取用户昵称
    QString getNickname() const;

    // 设置用户昵称
    void setNickname(const QString &nickname);

    // 获取设备信息
    QString getDeviceInfo() const;

    // 从设备信息生成UUID
    static QUuid generateUuidFromDevice();

    // 保存用户信息到本地
    bool saveToLocal() const;

    // 从本地加载用户信息
    static UserIdentity loadFromLocal();

private:
    QUuid uuid;         // 用户唯一标识符
    QString nickname;   // 用户昵称
    QString deviceInfo; // 设备信息

    // 获取设备信息
    static QString getDeviceInfoString();
};

} // namespace LocalNetworkApp

#endif // USERIDENTITY_H
