#include "file_transfer_session.h"
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
#include <QDir>

namespace LocalNetworkApp {

FileTransferSession::FileTransferSession(QUuid sessionId, QUuid senderId, QUuid receiverId, 
                                         const QString &filePath, bool isSender, QObject *parent) :
    QObject(parent),
    sessionId(sessionId),
    senderId(senderId),
    receiverId(receiverId),
    filePath(filePath),
    isSender(isSender),
    status(FileTransferStatus::Pending),
    bytesTransferred(0),
    currentBlockIndex(0),
    file(nullptr)
{
    if (isSender) {
        // 发送方：获取文件信息
        QFileInfo fileInfo(filePath);
        fileName = fileInfo.fileName();
        fileSize = fileInfo.size();
    } else {
        // 接收方：文件名和大小将在接收到数据后设置
        fileName = "";
        fileSize = 0;
    }
}

FileTransferSession::~FileTransferSession()
{
    closeFile();
}

QUuid FileTransferSession::getSessionId() const
{
    return sessionId;
}

QUuid FileTransferSession::getSenderId() const
{
    return senderId;
}

QUuid FileTransferSession::getReceiverId() const
{
    return receiverId;
}

QString FileTransferSession::getFilePath() const
{
    return filePath;
}

QString FileTransferSession::getFileName() const
{
    return fileName;
}

qint64 FileTransferSession::getFileSize() const
{
    return fileSize;
}

qint64 FileTransferSession::getBytesTransferred() const
{
    return bytesTransferred;
}

qreal FileTransferSession::getProgress() const
{
    if (fileSize <= 0) {
        return 0.0;
    }
    return static_cast<qreal>(bytesTransferred) / static_cast<qreal>(fileSize);
}

FileTransferStatus FileTransferSession::getStatus() const
{
    return status;
}

void FileTransferSession::start()
{
    if (status == FileTransferStatus::Completed || 
        status == FileTransferStatus::Failed || 
        status == FileTransferStatus::Cancelled) {
        return; // 这些状态不能重新开始
    }

    if (!initFile()) {
        updateStatus(FileTransferStatus::Failed);
        emit error("无法初始化文件");
        return;
    }

    updateStatus(FileTransferStatus::Transferring);

    if (isSender) {
        // 发送方开始发送数据块
        currentBlockIndex = 0;
        sendNextBlock();
    } else {
        // 接收方等待接收数据块
        if (pendingBlocks.contains(currentBlockIndex)) {
            processDataBlock(currentBlockIndex, pendingBlocks.take(currentBlockIndex));
        }
    }
}

void FileTransferSession::pause()
{
    if (status == FileTransferStatus::Transferring) {
        updateStatus(FileTransferStatus::Paused);
    }
}

void FileTransferSession::resume()
{
    if (status == FileTransferStatus::Paused) {
        updateStatus(FileTransferStatus::Transferring);
        
        if (isSender) {
            sendNextBlock();
        } else if (pendingBlocks.contains(currentBlockIndex)) {
            processDataBlock(currentBlockIndex, pendingBlocks.take(currentBlockIndex));
        }
    }
}

void FileTransferSession::cancel()
{
    if (status != FileTransferStatus::Completed && 
        status != FileTransferStatus::Failed && 
        status != FileTransferStatus::Cancelled) {
        updateStatus(FileTransferStatus::Cancelled);
        closeFile();
        
        // 如果是接收方且文件已部分写入，删除文件
        if (!isSender && !savePath.isEmpty()) {
            QFile::remove(savePath);
        }
        
        emit completed(false);
    }
}

void FileTransferSession::processDataBlock(qint64 blockIndex, const QByteArray &data)
{
    if (status != FileTransferStatus::Transferring && 
        status != FileTransferStatus::Pending) {
        // 如果不是传输中状态，将数据块缓存
        if (blockIndex == currentBlockIndex) {
            // 如果是期望的下一个块，立即处理
            if (status == FileTransferStatus::Pending) {
                start();
            } else {
                pendingBlocks[blockIndex] = data;
            }
        } else {
            // 否则缓存
            pendingBlocks[blockIndex] = data;
        }
        return;
    }

    if (blockIndex != currentBlockIndex) {
        // 不是期望的下一个块，缓存
        pendingBlocks[blockIndex] = data;
        return;
    }

    // 处理数据块
    if (!file) {
        if (!initFile()) {
            updateStatus(FileTransferStatus::Failed);
            emit error("无法初始化文件");
            return;
        }
    }

    // 写入数据
    qint64 bytesWritten = file->write(data);
    if (bytesWritten != data.size()) {
        updateStatus(FileTransferStatus::Failed);
        emit error("写入文件失败");
        return;
    }

    // 更新进度
    bytesTransferred += data.size();
    emit progressChanged(bytesTransferred, fileSize);

    // 检查是否完成
    if (bytesTransferred >= fileSize) {
        updateStatus(FileTransferStatus::Completed);
        closeFile();
        emit completed(true);
        return;
    }

    // 准备接收下一个块
    currentBlockIndex++;

    // 检查是否有缓存的下一个块
    if (pendingBlocks.contains(currentBlockIndex)) {
        QByteArray nextData = pendingBlocks.take(currentBlockIndex);
        QTimer::singleShot(0, this, [this, nextData]() {
            processDataBlock(currentBlockIndex, nextData);
        });
    }
}

void FileTransferSession::setSavePath(const QString &savePath)
{
    this->savePath = savePath;
    
    // 如果是接收方且文件名未设置，从保存路径中提取
    if (!isSender && fileName.isEmpty()) {
        QFileInfo fileInfo(savePath);
        fileName = fileInfo.fileName();
    }
}

void FileTransferSession::sendNextBlock()
{
    if (status != FileTransferStatus::Transferring) {
        return;
    }

    if (!file) {
        updateStatus(FileTransferStatus::Failed);
        emit error("文件未打开");
        return;
    }

    // 计算块大小和偏移量
    qint64 blockSize = Constants::FILE_BLOCK_SIZE;
    qint64 offset = currentBlockIndex * blockSize;

    // 检查是否超出文件大小
    if (offset >= fileSize) {
        updateStatus(FileTransferStatus::Completed);
        closeFile();
        emit completed(true);
        return;
    }

    // 调整最后一个块的大小
    if (offset + blockSize > fileSize) {
        blockSize = fileSize - offset;
    }

    // 读取数据块
    file->seek(offset);
    QByteArray data = file->read(blockSize);

    if (data.size() != blockSize) {
        updateStatus(FileTransferStatus::Failed);
        emit error("读取文件失败");
        return;
    }

    // 发送数据块
    emit sendDataBlock(sessionId, currentBlockIndex, data);

    // 更新进度
    bytesTransferred += data.size();
    emit progressChanged(bytesTransferred, fileSize);

    // 准备发送下一个块
    currentBlockIndex++;

    // 延迟发送下一个块，避免阻塞UI
    QTimer::singleShot(10, this, &FileTransferSession::sendNextBlock);
}

bool FileTransferSession::initFile()
{
    if (file) {
        return true; // 文件已经打开
    }

    if (isSender) {
        // 发送方：打开文件进行读取
        file = new QFile(filePath);
        if (!file->open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开文件进行读取:" << filePath << file->errorString();
            delete file;
            file = nullptr;
            return false;
        }
    } else {
        // 接收方：打开文件进行写入
        if (savePath.isEmpty()) {
            qWarning() << "保存路径未设置";
            return false;
        }

        // 确保目录存在
        QFileInfo fileInfo(savePath);
        QDir dir = fileInfo.dir();
        if (!dir.exists()) {
            if (!dir.mkpath(dir.absolutePath())) {
                qWarning() << "无法创建目录:" << dir.absolutePath();
                return false;
            }
        }

        file = new QFile(savePath);
        if (!file->open(QIODevice::WriteOnly)) {
            qWarning() << "无法打开文件进行写入:" << savePath << file->errorString();
            delete file;
            file = nullptr;
            return false;
        }
    }

    return true;
}

void FileTransferSession::closeFile()
{
    if (file) {
        file->close();
        delete file;
        file = nullptr;
    }
}

void FileTransferSession::updateStatus(FileTransferStatus newStatus)
{
    if (status != newStatus) {
        status = newStatus;
        emit statusChanged(status);
    }
}

} // namespace LocalNetworkApp
