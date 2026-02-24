#include "user_status.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace LocalNetworkApp {

UserStatus::UserStatus(QUuid userId, UserState state) :
    userId(userId),
    state(state)
{
}

QUuid UserStatus::getUserId() const
{
    return userId;
}

UserState UserStatus::getState() const
{
    return state;
}

void UserStatus::setState(UserState state)
{
    this->state = state;
}

QJsonObject UserStatus::toJson() const
{
    QJsonObject json;
    json["userId"] = userId.toString();
    json["state"] = static_cast<int>(state);
    return json;
}

UserStatus UserStatus::fromJson(const QJsonObject &json)
{
    QUuid userId = QUuid(json["userId"].toString());
    UserState state = static_cast<UserState>(json["state"].toInt());
    return UserStatus(userId, state);
}

} // namespace LocalNetworkApp