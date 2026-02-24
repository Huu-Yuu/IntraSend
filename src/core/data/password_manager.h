#ifndef PASSWORD_MANAGER_H
#define PASSWORD_MANAGER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include "security_manager.h"
#include "../utils/constants.h"

namespace LocalNetworkApp {

class PasswordManager : public QObject {
    Q_OBJECT

public:
    PasswordManager(QObject *parent = nullptr);
    ~PasswordManager() = default;

    // 添加密码
    bool addPassword(const QString &password);

    // 移除密码
    bool removePassword(const QString &password);

    // 验证密码
    bool verifyPassword(const QString &password) const;

    // 检查是否有密码
    bool hasPasswords() const;

    // 获取密码列表（仅返回哈希值）
    QList<QByteArray> getPasswordHashes() const;

    // 保存密码到本地
    bool saveToLocal() const;

    // 从本地加载密码
    bool loadFromLocal();

    // 清除所有密码
    void clearAllPasswords();

signals:
    // 密码添加成功
    void passwordAdded();

    // 密码移除成功
    void passwordRemoved();

    // 密码列表变更
    void passwordsChanged();

private:
    QList<QByteArray> passwordHashes; // 密码哈希列表

    // 获取密码文件路径
    QString getPasswordFilePath() const;
};

} // namespace LocalNetworkApp

#endif // PASSWORD_MANAGER_H
