#ifndef FILE_TRANSFER_SESSION_H
#define FILE_TRANSFER_SESSION_H

#include <QObject>
#include <QUuid>
#include <QFile>
#include <QMap>
#include <QByteArray>
#include "../utils/enums.h"
#include "../utils/constants.h"

namespace LocalNetworkApp {

class FileTransferSession : public QObject {
    Q_OBJECT

public:
    FileTransferSession(QUuid sessionId, QUuid senderId, QUuid receiverId, 
                        const QString &filePath, bool isSender, QObject *parent = nullptr);
    ~FileTransferSession();

    // 获取会话ID
    QUuid getSessionId() const;

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

    // 获取已传输字节数
    qint64 getBytesTransferred() const;

    // 获取传输进度（0.0-1.0）
    qreal getProgress() const;

    // 获取传输状态
    FileTransferStatus getStatus() const;

    // 开始传输
    void start();

    // 暂停传输
    void pause();

    // 恢复传输
    void resume();

    // 取消传输
    void cancel();

    // 处理接收到的数据块
    void processDataBlock(qint64 blockIndex, const QByteArray &data);

    // 设置保存路径（接收方使用）
    void setSavePath(const QString &savePath);

signals:
    // 传输进度更新
    void progressChanged(qint64 bytesTransferred, qint64 totalBytes);

    // 传输完成
    void completed(bool success);

    // 传输错误
    void error(const QString &errorMessage);

    // 状态变更
    void statusChanged(FileTransferStatus status);

    // 发送数据块（内部信号）
    void sendDataBlock(QUuid sessionId, qint64 blockIndex, const QByteArray &data);

private slots:
    // 发送下一个数据块
    void sendNextBlock();

private:
    QUuid sessionId;              // 会话ID
    QUuid senderId;               // 发送者ID
    QUuid receiverId;             // 接收者ID
    QString filePath;             // 文件路径
    QString fileName;             // 文件名
    QString savePath;             // 保存路径（接收方使用）
    qint64 fileSize;              // 文件大小
    qint64 bytesTransferred;      // 已传输字节数
    QFile *file;                  // 文件对象
    bool isSender;                // 是否为发送方
    FileTransferStatus status;    // 传输状态
    qint64 currentBlockIndex;     // 当前块索引
    QMap<qint64, QByteArray> pendingBlocks; // 待处理的数据块

    // 初始化文件
    bool initFile();

    // 关闭文件
    void closeFile();

    // 更新状态
    void updateStatus(FileTransferStatus newStatus);
};

} // namespace LocalNetworkApp

#endif // FILE_TRANSFER_SESSION_H
