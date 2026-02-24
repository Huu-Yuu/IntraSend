#ifndef USER_STATUS_H
#define USER_STATUS_H

#include <QUuid>
#include "core/utils/enums.h"
#include <QJsonObject>
namespace LocalNetworkApp {

class UserStatus {
public:
    UserStatus(QUuid userId, UserState state);
    ~UserStatus() = default;

    // 获取用户ID
    QUuid getUserId() const;

    // 获取用户状态
    UserState getState() const;

    // 设置用户状态
    void setState(UserState state);

    // 转换为JSON格式
    QJsonObject toJson() const;

    // 从JSON格式创建
    static UserStatus fromJson(const QJsonObject &json);

private:
    QUuid userId;    // 用户ID
    UserState state; // 用户状态
};

} // namespace LocalNetworkApp

#endif // USER_STATUS_H
