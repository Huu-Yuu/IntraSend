#ifndef FILE_TRANSFER_RESPONSE_H
#define FILE_TRANSFER_RESPONSE_H

#include <QUuid>
#include <QString>
#include <QJsonObject>

namespace LocalNetworkApp {

class FileTransferResponse {
public:
    FileTransferResponse(QUuid requestId, QUuid receiverId, bool accepted, const QString &savePath = "");
    FileTransferResponse(const QJsonObject &json);
    ~FileTransferResponse() = default;

    // 获取请求ID
    QUuid getRequestId() const;

    // 获取接收者ID
    QUuid getReceiverId() const;

    // 获取是否接受传输
    bool isAccepted() const;

    // 获取保存路径
    QString getSavePath() const;

    // 转换为JSON格式
    QJsonObject toJson() const;

private:
    QUuid requestId;    // 请求ID
    QUuid receiverId;   // 接收者ID
    bool accepted;      // 是否接受传输
    QString savePath;   // 保存路径
};

} // namespace LocalNetworkApp

#endif // FILE_TRANSFER_RESPONSE_H
