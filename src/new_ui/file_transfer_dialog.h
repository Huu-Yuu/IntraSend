#ifndef FILE_TRANSFER_DIALOG_H
#define FILE_TRANSFER_DIALOG_H

#include <QDialog>
#include <QUuid>
#include "core/filetransfer/file_transfer_request.h"
#include "core/filetransfer/file_transfer_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileTransferDialog; }
QT_END_NAMESPACE

namespace LocalNetworkApp {

class FileTransferDialog : public QDialog
{
    Q_OBJECT

public:
    // 接收方对话框
    explicit FileTransferDialog(const FileTransferRequest &request, FileTransferManager &manager, QWidget *parent = nullptr);

    // 发送方对话框
    explicit FileTransferDialog(QUuid sessionId, FileTransferManager &manager, QWidget *parent = nullptr);

    ~FileTransferDialog() override;

private slots:
    // 接收方操作
    void on_btnAccept_clicked();
    void on_btnReject_clicked();

    // 发送方操作
    void on_btnPause_clicked();
    void on_btnCancel_clicked();

    // 更新传输进度
    void updateProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes);

    // 传输状态变更
    void onTransferStatusChanged(QUuid sessionId, FileTransferStatus status);

    // 传输完成
    void onTransferCompleted(QUuid sessionId, bool success);

private:
    Ui::FileTransferDialog *ui;
    FileTransferManager &fileTransferManager; // 文件传输管理器（引用）
    FileTransferRequest transferRequest;      // 传输请求（接收方）
    QUuid sessionId;                          // 会话ID（发送方）
    bool isSender;                            // 是否为发送方

    // 初始化接收方界面
    void initReceiverUI();

    // 初始化发送方界面
    void initSenderUI();

    // 更新界面状态
    void updateUIState(FileTransferStatus status);
};

} // namespace LocalNetworkApp

#endif // FILE_TRANSFER_DIALOG_H
