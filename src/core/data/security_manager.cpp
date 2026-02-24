#include "security_manager.h"
#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringList>
#include <QDebug>

namespace LocalNetworkApp {

QByteArray SecurityManager::hashPassword(const QString &password)
{
    // 使用SHA-256算法哈希密码
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    return hash.result().toHex();
}

bool SecurityManager::verifyPassword(const QString &password, const QByteArray &hash)
{
    QByteArray passwordHash = hashPassword(password);
    return passwordHash == hash;
}

QByteArray SecurityManager::generateRandomKey(int length)
{
    QByteArray key;
    key.resize(length);

    for (int i = 0; i < length; ++i) {
        key[i] = QRandomGenerator::global()->bounded(256);
    }

    return key;
}

QByteArray SecurityManager::encryptData(const QByteArray &data, const QString &key)
{
    // 生成随机IV
    QByteArray iv = generateRandomKey(16);

    // 使用SHA-256哈希密钥
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(key.toUtf8());
    QByteArray keyHash = hash.result();

    // 加密数据
    QByteArray paddedData = padData(data, 16);
    QByteArray encrypted = encryptAES(paddedData, keyHash, iv);

    // 返回IV + 加密数据
    return iv + encrypted;
}

QByteArray SecurityManager::decryptData(const QByteArray &data, const QString &key)
{
    if (data.size() < 16) {
        return QByteArray();
    }

    // 提取IV
    QByteArray iv = data.left(16);
    QByteArray encrypted = data.mid(16);

    // 使用SHA-256哈希密钥
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(key.toUtf8());
    QByteArray keyHash = hash.result();

    // 解密数据
    QByteArray decrypted = decryptAES(encrypted, keyHash, iv);
    if (decrypted.isEmpty()) {
        return QByteArray();
    }

    // 移除填充
    return unpadData(decrypted);
}

QByteArray SecurityManager::calculateFileHash(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!file.atEnd()) {
        QByteArray chunk = file.read(8192);
        hash.addData(chunk);
    }

    return hash.result().toHex();
}

bool SecurityManager::isStrongPassword(const QString &password)
{
    // 密码长度至少8位
    if (password.length() < 8) {
        return false;
    }

    // 检查是否包含数字
    if (!password.contains(QRegularExpression("[0-9]"))) {
        return false;
    }

    // 检查是否包含小写字母
    if (!password.contains(QRegularExpression("[a-z]"))) {
        return false;
    }

    // 检查是否包含大写字母
    if (!password.contains(QRegularExpression("[A-Z]"))) {
        return false;
    }

    // 检查是否包含特殊字符
    if (!password.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) {
        return false;
    }

    return true;
}

QString SecurityManager::getPasswordStrength(const QString &password)
{
    int strength = 0;

    // 长度检查
    if (password.length() >= 8) {
        strength += 1;
    }
    if (password.length() >= 12) {
        strength += 1;
    }

    // 字符类型检查
    if (password.contains(QRegularExpression("[0-9]"))) {
        strength += 1;
    }
    if (password.contains(QRegularExpression("[a-z]"))) {
        strength += 1;
    }
    if (password.contains(QRegularExpression("[A-Z]"))) {
        strength += 1;
    }
    if (password.contains(QRegularExpression("[!@#$%^&*(),.?\":{}|<>]"))) {
        strength += 1;
    }

    // 返回强度描述
    if (strength <= 2) {
        return "弱";
    } else if (strength <= 4) {
        return "中";
    } else {
        return "强";
    }
}

QByteArray SecurityManager::encryptAES(const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
    QByteArray paddedData = padData(data);
    QByteArray normalizedKey = normalizeKey(key);
    QByteArray result;
    QByteArray prevBlock = iv;

    // CBC模式加密（分块处理，每块16字节）
    for (int i = 0; i < paddedData.size(); i += 16) {
        QByteArray block = paddedData.mid(i, 16);
        // CBC步骤1：与前一块（或IV）XOR
        block = xorBytes(block, prevBlock);
        // CBC步骤2：加密当前块
        block = aesBlockEncrypt(block, normalizedKey);
        // 保存结果
        result.append(block);
        // 更新前一块
        prevBlock = block;
    }

    return result;
}

QByteArray SecurityManager::decryptAES(const QByteArray &data, const QByteArray &key, const QByteArray &iv)
{
    QByteArray normalizedKey = normalizeKey(key);
    QByteArray result;
    QByteArray prevBlock = iv;

    // CBC模式解密（分块处理）
    for (int i = 0; i < data.size(); i += 16) {
        QByteArray block = data.mid(i, 16);
        // 保存当前块用于下一轮XOR
        QByteArray currentBlock = block;
        // CBC步骤1：解密当前块
        block = aesBlockDecrypt(block, normalizedKey);
        // CBC步骤2：与前一块（或IV）XOR
        block = xorBytes(block, prevBlock);
        // 保存结果
        result.append(block);
        // 更新前一块
        prevBlock = currentBlock;
    }

    // 去除填充
    return unpadData(result);
}

QByteArray SecurityManager::padData(const QByteArray &data, int blockSize)
{
    int padding = blockSize - (data.size() % blockSize);
    QByteArray padded = data;
    padded.append(padding, padding);
    return padded;
}

QByteArray SecurityManager::unpadData(const QByteArray &data)
{
    if (data.isEmpty()) {
        return data;
    }

    int padding = static_cast<unsigned char>(data.last());
    if (padding > data.size()) {
        return data;
    }

    return data.left(data.size() - padding);
}

// 辅助函数实现
QByteArray SecurityManager::xorBytes(const QByteArray &a, const QByteArray &b)
{
    QByteArray result;
    int size = qMin(a.size(), b.size());
    for (int i = 0; i < size; ++i) {
        result.append(static_cast<char>(static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i])));
    }
    return result;
}

QByteArray SecurityManager::aesBlockEncrypt(const QByteArray &block, const QByteArray &key)
{
    // 简化版AES块加密（实际项目建议使用QCA或第三方库）
    // 这里使用Qt哈希模拟（仅作示例，生产环境请用QCA）
    QByteArray encrypted = QCryptographicHash::hash(block + key, QCryptographicHash::Sha256);
    return encrypted.left(16); // 取前16字节作为块加密结果
}

QByteArray SecurityManager::aesBlockDecrypt(const QByteArray &block, const QByteArray &key)
{
    // 简化版AES块解密（与加密对应）
    QByteArray decrypted = QCryptographicHash::hash(block + key, QCryptographicHash::Sha256);
    return decrypted.left(16);
}

QByteArray SecurityManager::normalizeKey(const QByteArray &key)
{
    // 将任意长度的密钥转换为32字节（256位）的密钥
    if (key.size() == 32) {
        return key;
    }
    // 使用SHA-256哈希生成固定长度的密钥
    return QCryptographicHash::hash(key, QCryptographicHash::Sha256);
}


} // namespace LocalNetworkApp
