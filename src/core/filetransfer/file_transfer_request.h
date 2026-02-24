#ifndef FILE_TRANSFER_REQUEST_H
#define FILE_TRANSFER_REQUEST_H

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QJsonObject>

namespace LocalNetworkApp {

class FileTransferRequest {
public:
    FileTransferRequest(QUuid senderId, QUuid receiverId, const QString &filePath);
    FileTransferRequest(const QJsonObject &json);
    ~FileTransferRequest() = default;

    // 获取请求ID
    QUuid getRequestId() const;

    // 获取发送者ID
    QUuid getSenderId() const;

    // 获取接收者ID
    QUuid getReceiverId() const;

    // 获取文件路径
    QString getFilePath() const;

    // 获取文件名
    QString getFileName() const;

    // 获取文件大小
    qint64 getFileSize() const;

    // 获取请求时间戳
    QDateTime getTimestamp() const;

    // 转换为JSON格式
    QJsonObject toJson() const;

private:
    QUuid requestId;    // 请求唯一标识符
    QUuid senderId;     // 发送者ID
    QUuid receiverId;   // 接收者ID
    QString filePath;   // 文件路径
    QString fileName;   // 文件名
    qint64 fileSize;    // 文件大小
    QDateTime timestamp; // 请求时间戳
};

} // namespace LocalNetworkApp

#endif // FILE_TRANSFER_REQUEST_H
