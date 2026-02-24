#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#include <QString>
#include <QByteArray>
#include <QCryptographicHash>

namespace LocalNetworkApp {

class SecurityManager {
public:
    SecurityManager() = delete;
    ~SecurityManager() = delete;

    // 密码哈希算法
    static QByteArray hashPassword(const QString &password);

    // 验证密码
    static bool verifyPassword(const QString &password, const QByteArray &hash);

    // 生成随机密钥
    static QByteArray generateRandomKey(int length = 32);

    // 加密数据
    static QByteArray encryptData(const QByteArray &data, const QString &key);

    // 解密数据
    static QByteArray decryptData(const QByteArray &data, const QString &key);

    // 计算文件哈希
    static QByteArray calculateFileHash(const QString &filePath);

    // 验证密码强度
    static bool isStrongPassword(const QString &password);

    // 获取密码强度描述
    static QString getPasswordStrength(const QString &password);

private:
    // 加密算法
    static QByteArray encryptAES(const QByteArray &data, const QByteArray &key, const QByteArray &iv);

    // 解密算法
    static QByteArray decryptAES(const QByteArray &data, const QByteArray &key, const QByteArray &iv);

    // 填充数据
    static QByteArray padData(const QByteArray &data, int blockSize);

    // 移除填充
    static QByteArray unpadData(const QByteArray &data);

    // 辅助函数：XOR两个等长字节数组
    static QByteArray xorBytes(const QByteArray &a, const QByteArray &b);

    // 辅助函数：AES单块加密（简化实现）
    static QByteArray aesBlockEncrypt(const QByteArray &block, const QByteArray &key);

    // 辅助函数：AES单块解密（简化实现）
    static QByteArray aesBlockDecrypt(const QByteArray &block, const QByteArray &key);

    // 辅助函数：将密钥标准化为256位（32字节）
    static QByteArray normalizeKey(const QByteArray &key);
};

} // namespace LocalNetworkApp

#endif // SECURITY_MANAGER_H
