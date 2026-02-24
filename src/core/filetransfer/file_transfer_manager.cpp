#include "file_transfer_manager.h"
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include "../utils/constants.h"
#include "../message/message.h"

namespace LocalNetworkApp {

FileTransferManager::FileTransferManager(ContactManager *contactManager, MessageManager *messageManager, QObject *parent) :
    QObject(parent),
    contactManager(contactManager),
    messageManager(messageManager),
    incognitoMode(false)
{
    initDownloadDirectory();
    loadTransferHistory();
}

FileTransferManager::~FileTransferManager()
{
    // 清理所有活动的传输会话
    qDeleteAll(activeTransfers);
    activeTransfers.clear();
}

bool FileTransferManager::initDownloadDirectory()
{
    QDir dir(getDefaultDownloadDirectory());
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

QString FileTransferManager::getDefaultDownloadDirectory()
{
    return QDir::homePath() + "/Downloads/LocalNetworkApp";
}

bool FileTransferManager::initiateFileTransfer(QUuid senderId, QUuid receiverId, const QString &filePath)
{
    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    // 检查接收者是否在黑名单中
    if (contactManager->isInBlacklist(receiverId)) {
        // 发送消息通知用户
        Message message(senderId, receiverId, "您在对方的黑名单中，无法发送文件", MessageType::System);
        messageManager->receiveMessage(message);
        return false;
    }

    // 创建文件传输请求
    FileTransferRequest request(senderId, receiverId, filePath);
    pendingRequests[request.getRequestId()] = request;

    // 发送请求
    emit fileTransferRequestSent(request);
    return true;
}

void FileTransferManager::handleFileTransferRequest(const FileTransferRequest &request)
{
    QUuid senderId = request.getSenderId();
    QUuid receiverId = request.getReceiverId();

    // 检查发送者是否在黑名单中
    if (contactManager->isInBlacklist(senderId)) {
        // 直接拒绝请求
        FileTransferResponse response(request.getRequestId(), receiverId, false);
        emit fileTransferResponseSent(response);
        return;
    }

    // 检查发送者是否在白名单中
    if (contactManager->isInWhitelist(senderId)) {
        // 自动接受请求
        QString savePath = getDefaultDownloadDirectory() + "/" + request.getFileName();
        acceptFileTransfer(request, savePath);
        return;
    }

    // 否则，通知UI显示请求
    emit fileTransferRequestReceived(request);
}

void FileTransferManager::handleFileTransferResponse(const FileTransferResponse &response)
{
    QUuid requestId = response.getRequestId();

    // 查找对应的请求
    if (!pendingRequests.contains(requestId)) {
        return; // 请求不存在
    }

    FileTransferRequest request = pendingRequests.take(requestId);

    if (response.isAccepted()) {
        // 创建传输会话
        QUuid sessionId = QUuid::createUuid();
        FileTransferSession *session = new FileTransferSession(
            sessionId, request.getSenderId(), request.getReceiverId(),
            request.getFilePath(), true, this);

        // 连接信号
        connect(session, &FileTransferSession::sendDataBlock, this, &FileTransferManager::fileDataBlockSent);
        connect(session, &FileTransferSession::progressChanged, this, [this, sessionId](qint64 bytesTransferred, qint64 totalBytes) {
            emit transferProgress(sessionId, bytesTransferred, totalBytes);
        });
        connect(session, &FileTransferSession::completed, this, [this, sessionId](bool success) {
            saveTransferHistory(getTransferSession(sessionId), success);
            emit transferCompleted(sessionId, success);
        });
        connect(session, &FileTransferSession::statusChanged, this, [this, sessionId](FileTransferStatus status) {
            emit transferStatusChanged(sessionId, status);
        });

        // 添加到活动会话
        activeTransfers[sessionId] = session;

        // 开始传输
        session->start();
    }

    // 通知UI
    emit fileTransferResponseReceived(response);
}

void FileTransferManager::handleFileData(QUuid sessionId, qint64 blockIndex, const QByteArray &data)
{
    if (!activeTransfers.contains(sessionId)) {
        return; // 会话不存在
    }

    FileTransferSession *session = activeTransfers[sessionId];
    session->processDataBlock(blockIndex, data);
}

void FileTransferManager::cancelTransfer(QUuid sessionId)
{
    if (!activeTransfers.contains(sessionId)) {
        return;
    }

    FileTransferSession *session = activeTransfers.take(sessionId);
    session->cancel();
    delete session;
}

void FileTransferManager::pauseTransfer(QUuid sessionId)
{
    if (!activeTransfers.contains(sessionId)) {
        return;
    }

    FileTransferSession *session = activeTransfers[sessionId];
    session->pause();
}

void FileTransferManager::resumeTransfer(QUuid sessionId)
{
    if (!activeTransfers.contains(sessionId)) {
        return;
    }

    FileTransferSession *session = activeTransfers[sessionId];
    session->resume();
}

QList<FileTransferSession*> FileTransferManager::getActiveTransfers() const
{
    return activeTransfers.values();
}

FileTransferSession* FileTransferManager::getTransferSession(QUuid sessionId) const
{
    return activeTransfers.value(sessionId, nullptr);
}

void FileTransferManager::setIncognitoMode(bool enabled)
{
    incognitoMode = enabled;
    if (enabled) {
        clearAllTransferHistory();
    }
}

bool FileTransferManager::isIncognitoMode() const
{
    return incognitoMode;
}

void FileTransferManager::clearAllTransferHistory()
{
    // 清除历史记录
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    settings.remove("fileTransfers/history");
}

void FileTransferManager::acceptFileTransfer(const FileTransferRequest &request, const QString &savePath)
{
    QUuid receiverId = request.getReceiverId();

    // 创建响应
    FileTransferResponse response(request.getRequestId(), receiverId, true, savePath);
    emit fileTransferResponseSent(response);

    // 创建传输会话
    QUuid sessionId = QUuid::createUuid();
    FileTransferSession *session = new FileTransferSession(
        sessionId, request.getSenderId(), request.getReceiverId(),
        savePath, false, this);

    // 设置保存路径
    session->setSavePath(savePath);

    // 连接信号
    connect(session, &FileTransferSession::sendDataBlock, this, &FileTransferManager::fileDataBlockSent);
    connect(session, &FileTransferSession::progressChanged, this, [this, sessionId](qint64 bytesTransferred, qint64 totalBytes) {
        emit transferProgress(sessionId, bytesTransferred, totalBytes);
    });
    connect(session, &FileTransferSession::completed, this, [this, sessionId](bool success) {
        saveTransferHistory(getTransferSession(sessionId), success);
        emit transferCompleted(sessionId, success);
    });
    connect(session, &FileTransferSession::statusChanged, this, [this, sessionId](FileTransferStatus status) {
        emit transferStatusChanged(sessionId, status);
    });

    // 添加到活动会话
    activeTransfers[sessionId] = session;

    // 开始传输
    session->start();
}

void FileTransferManager::rejectFileTransfer(const FileTransferRequest &request)
{
    QUuid receiverId = request.getReceiverId();

    // 创建响应
    FileTransferResponse response(request.getRequestId(), receiverId, false);
    emit fileTransferResponseSent(response);
}

void FileTransferManager::saveTransferHistory(const FileTransferSession *session, bool success)
{
    if (incognitoMode || !session) {
        return; // 无痕模式下不保存
    }

    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);

    // 获取现有历史记录
    QJsonArray historyArray;
    if (settings.contains("fileTransfers/history")) {
        QByteArray jsonData = settings.value("fileTransfers/history").toByteArray();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        historyArray = doc.array();
    }

    // 添加新记录
    QJsonObject record;
    record["sessionId"] = session->getSessionId().toString();
    record["senderId"] = session->getSenderId().toString();
    record["receiverId"] = session->getReceiverId().toString();
    record["fileName"] = session->getFileName();
    record["fileSize"] = static_cast<qint64>(session->getFileSize());
    record["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    record["success"] = success;
    record["isSender"] = (session->getSenderId() == contactManager->getContact(session->getSenderId()).id);

    historyArray.append(record);

    // 保存历史记录
    settings.setValue("fileTransfers/history", QString::fromUtf8(QJsonDocument(historyArray).toJson()));
}

void FileTransferManager::loadTransferHistory()
{
    if (incognitoMode) {
        return; // 无痕模式下不加载
    }

    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);

    // 这里可以实现加载历史记录的逻辑
    // 但由于历史记录主要用于显示，可能不需要在管理器中保留
}

} // namespace LocalNetworkApp