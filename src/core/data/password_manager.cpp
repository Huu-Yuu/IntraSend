#include "password_manager.h"
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>

namespace LocalNetworkApp {

PasswordManager::PasswordManager(QObject *parent) :
    QObject(parent)
{
    loadFromLocal();
}

bool PasswordManager::addPassword(const QString &password)
{
    // 检查密码强度
    if (!SecurityManager::isStrongPassword(password)) {
        qWarning() << "密码强度不足";
        return false;
    }

    // 检查密码是否已存在
    QByteArray passwordHash = SecurityManager::hashPassword(password);
    if (passwordHashes.contains(passwordHash)) {
        qWarning() << "密码已存在";
        return false;
    }

    // 添加密码哈希
    passwordHashes.append(passwordHash);

    // 保存到本地
    if (saveToLocal()) {
        emit passwordAdded();
        emit passwordsChanged();
        return true;
    }

    return false;
}

bool PasswordManager::removePassword(const QString &password)
{
    // 计算密码哈希
    QByteArray passwordHash = SecurityManager::hashPassword(password);

    // 查找并移除密码
    int index = passwordHashes.indexOf(passwordHash);
    if (index == -1) {
        qWarning() << "密码不存在";
        return false;
    }

    passwordHashes.removeAt(index);

    // 保存到本地
    if (saveToLocal()) {
        emit passwordRemoved();
        emit passwordsChanged();
        return true;
    }

    return false;
}

bool PasswordManager::verifyPassword(const QString &password) const
{
    QByteArray passwordHash = SecurityManager::hashPassword(password);
    return passwordHashes.contains(passwordHash);
}

bool PasswordManager::hasPasswords() const
{
    return !passwordHashes.isEmpty();
}

QList<QByteArray> PasswordManager::getPasswordHashes() const
{
    return passwordHashes;
}

bool PasswordManager::saveToLocal() const
{
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);

    // 将密码哈希转换为JSON数组
    QJsonArray passwordArray;
    for (const auto &hash : passwordHashes) {
        passwordArray.append(QString::fromUtf8(hash));
    }

    // 保存到设置文件
    settings.setValue("security/passwords", QString::fromUtf8(QJsonDocument(passwordArray).toJson()));
    return settings.status() == QSettings::NoError;
}

bool PasswordManager::loadFromLocal()
{
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);

    if (!settings.contains("security/passwords")) {
        return false;
    }

    // 从设置文件加载
    QByteArray jsonData = settings.value("security/passwords").toByteArray();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonArray passwordArray = doc.array();

    // 转换为密码哈希列表
    passwordHashes.clear();
    for (const auto &value : passwordArray) {
        passwordHashes.append(value.toString().toUtf8());
    }

    return true;
}

void PasswordManager::clearAllPasswords()
{
    passwordHashes.clear();
    saveToLocal();
    emit passwordsChanged();
}

QString PasswordManager::getPasswordFilePath() const
{
    return QSettings().fileName();
}

} // namespace LocalNetworkApp