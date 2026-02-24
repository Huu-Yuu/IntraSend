#ifndef FILE_TRANSFER_MANAGER_H
#define FILE_TRANSFER_MANAGER_H

#include <QObject>
#include <QMap>
#include <QUuid>
#include <QString>
#include "file_transfer_request.h"
#include "file_transfer_response.h"
#include "file_transfer_session.h"
#include "../user/contact_manager.h"
#include "../message/message_manager.h"

namespace LocalNetworkApp {

class FileTransferManager : public QObject {
    Q_OBJECT

public:
    FileTransferManager(ContactManager *contactManager, MessageManager *messageManager, QObject *parent = nullptr);
    ~FileTransferManager();

    // 初始化下载目录
    static bool initDownloadDirectory();

    // 获取默认下载目录
    static QString getDefaultDownloadDirectory();

    // 发起文件传输请求
    bool initiateFileTransfer(QUuid senderId, QUuid receiverId, const QString &filePath);

    // 处理文件传输请求
    void handleFileTransferRequest(const FileTransferRequest &request);

    // 处理文件传输响应
    void handleFileTransferResponse(const FileTransferResponse &response);

    // 处理文件数据块
    void handleFileData(QUuid sessionId, qint64 blockIndex, const QByteArray &data);

    // 取消文件传输
    void cancelTransfer(QUuid sessionId);

    // 暂停文件传输
    void pauseTransfer(QUuid sessionId);

    // 恢复文件传输
    void resumeTransfer(QUuid sessionId);

    // 获取所有活动的传输会话
    QList<FileTransferSession*> getActiveTransfers() const;

    // 获取特定的传输会话
    FileTransferSession* getTransferSession(QUuid sessionId) const;

    // 设置是否启用无痕模式
    void setIncognitoMode(bool enabled);

    // 获取无痕模式状态
    bool isIncognitoMode() const;

    // 清除所有文件传输历史
    void clearAllTransferHistory();

signals:
    // 发送文件传输请求
    void fileTransferRequestSent(const FileTransferRequest &request);

    // 收到文件传输请求
    void fileTransferRequestReceived(const FileTransferRequest &request);

    // 发送文件传输响应
    void fileTransferResponseSent(const FileTransferResponse &response);

    // 收到文件传输响应
    void fileTransferResponseReceived(const FileTransferResponse &response);

    // 发送文件数据块
    void fileDataBlockSent(QUuid sessionId, qint64 blockIndex, const QByteArray &data);

    // 传输进度更新
    void transferProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes);

    // 传输完成
    void transferCompleted(QUuid sessionId, bool success);

    // 传输状态变更
    void transferStatusChanged(QUuid sessionId, FileTransferStatus status);

public slots:
    // 接受文件传输请求
    void acceptFileTransfer(const FileTransferRequest &request, const QString &savePath);

    // 拒绝文件传输请求
    void rejectFileTransfer(const FileTransferRequest &request);

private:
    QMap<QUuid, FileTransferSession*> activeTransfers; // 活动的传输会话
    QMap<QUuid, FileTransferRequest> pendingRequests; // 待处理的请求
    ContactManager *contactManager; // 联系人管理器
    MessageManager *messageManager; // 消息管理器
    bool incognitoMode; // 无痕模式标志

    // 保存传输历史
    void saveTransferHistory(const FileTransferSession *session, bool success);

    // 加载传输历史
    void loadTransferHistory();
};

} // namespace LocalNetworkApp

#endif // FILE_TRANSFER_MANAGER_H
