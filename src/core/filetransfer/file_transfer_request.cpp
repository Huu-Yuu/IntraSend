#include "file_transfer_request.h"
#include <QFileInfo>

namespace LocalNetworkApp {

FileTransferRequest::FileTransferRequest(QUuid senderId, QUuid receiverId, const QString &filePath) :
    requestId(QUuid::createUuid()),
    senderId(senderId),
    receiverId(receiverId),
    filePath(filePath),
    timestamp(QDateTime::currentDateTime())
{
    // 获取文件名和大小
    QFileInfo fileInfo(filePath);
    fileName = fileInfo.fileName();
    fileSize = fileInfo.size();
}

FileTransferRequest::FileTransferRequest(const QJsonObject &json) :
    requestId(QUuid(json["requestId"].toString())),
    senderId(QUuid(json["senderId"].toString())),
    receiverId(QUuid(json["receiverId"].toString())),
    filePath(json["filePath"].toString()),
    fileName(json["fileName"].toString()),
    fileSize(json["fileSize"].toVariant().toLongLong()),
    timestamp(QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate))
{
}

QUuid FileTransferRequest::getRequestId() const
{
    return requestId;
}

QUuid FileTransferRequest::getSenderId() const
{
    return senderId;
}

QUuid FileTransferRequest::getReceiverId() const
{
    return receiverId;
}

QString FileTransferRequest::getFilePath() const
{
    return filePath;
}

QString FileTransferRequest::getFileName() const
{
    return fileName;
}

qint64 FileTransferRequest::getFileSize() const
{
    return fileSize;
}

QDateTime FileTransferRequest::getTimestamp() const
{
    return timestamp;
}

QJsonObject FileTransferRequest::toJson() const
{
    QJsonObject json;
    json["requestId"] = requestId.toString();
    json["senderId"] = senderId.toString();
    json["receiverId"] = receiverId.toString();
    json["filePath"] = filePath;
    json["fileName"] = fileName;
    json["fileSize"] = static_cast<qint64>(fileSize);
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    return json;
}

} // namespace LocalNetworkApp