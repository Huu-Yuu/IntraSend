#include "file_transfer_response.h"

namespace LocalNetworkApp {

FileTransferResponse::FileTransferResponse(QUuid requestId, QUuid receiverId, bool accepted, const QString &savePath) :
    requestId(requestId),
    receiverId(receiverId),
    accepted(accepted),
    savePath(savePath)
{
}

FileTransferResponse::FileTransferResponse(const QJsonObject &json) :
    requestId(QUuid(json["requestId"].toString())),
    receiverId(QUuid(json["receiverId"].toString())),
    accepted(json["accepted"].toBool()),
    savePath(json["savePath"].toString())
{
}

QUuid FileTransferResponse::getRequestId() const
{
    return requestId;
}

QUuid FileTransferResponse::getReceiverId() const
{
    return receiverId;
}

bool FileTransferResponse::isAccepted() const
{
    return accepted;
}

QString FileTransferResponse::getSavePath() const
{
    return savePath;
}

QJsonObject FileTransferResponse::toJson() const
{
    QJsonObject json;
    json["requestId"] = requestId.toString();
    json["receiverId"] = receiverId.toString();
    json["accepted"] = accepted;
    json["savePath"] = savePath;
    return json;
}

} // namespace LocalNetworkApp