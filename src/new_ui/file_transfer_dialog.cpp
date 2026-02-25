#include "file_transfer_dialog.h"
#include "ui_file_transfer_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include "core/utils/constants.h"

namespace LocalNetworkApp {

FileTransferDialog::FileTransferDialog(const FileTransferRequest &request, FileTransferManager &manager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileTransferDialog)
    , fileTransferManager(manager)
    , transferRequest(request)
    , isSender(false)
{
    ui->setupUi(this);
    initReceiverUI();
}

FileTransferDialog::FileTransferDialog(QUuid sessionId, FileTransferManager &manager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileTransferDialog)
    , fileTransferManager(manager)
    , sessionId(sessionId)
    , isSender(true)
{
    ui->setupUi(this);
    initSenderUI();
}

FileTransferDialog::~FileTransferDialog()
{
    delete ui;
}

void FileTransferDialog::on_btnAccept_clicked()
{
    if (isSender) return;

    // 选择保存路径
    QString defaultPath = FileTransferManager::getDefaultDownloadDirectory();
    QString savePath = QFileDialog::getSaveFileName(this, tr("保存文件"),
        defaultPath + "/" + transferRequest.getFileName());

    if (savePath.isEmpty()) return;

    // 接受文件传输
    fileTransferManager.acceptFileTransfer(transferRequest, savePath);

    // 切换到进度显示模式
    ui->widgetRequest->setVisible(false);
    ui->widgetProgress->setVisible(true);

    // 更新UI
    ui->lblFileName->setText(transferRequest.getFileName());
    ui->progressBar->setMaximum(static_cast<int>(transferRequest.getFileSize()));

    // 禁用按钮
    ui->btnAccept->setEnabled(false);
    ui->btnReject->setEnabled(false);
}

void FileTransferDialog::on_btnReject_clicked()
{
    if (isSender) return;

    // 拒绝文件传输
    fileTransferManager.rejectFileTransfer(transferRequest);
    QMessageBox::information(this, tr("已拒绝"), tr("已拒绝接收该文件！"));
    reject();
}

void FileTransferDialog::on_btnPause_clicked()
{
    if (!isSender) return;

    FileTransferSession *session = fileTransferManager.getTransferSession(sessionId);
    if (!session) return;

    if (session->getStatus() == FileTransferStatus::Transferring) {
        // 暂停传输
        fileTransferManager.pauseTransfer(sessionId);
        ui->btnPause->setText(tr("恢复"));
    } else if (session->getStatus() == FileTransferStatus::Paused) {
        // 恢复传输
        fileTransferManager.resumeTransfer(sessionId);
        ui->btnPause->setText(tr("暂停"));
    }
}

void FileTransferDialog::on_btnCancel_clicked()
{
    int ret = QMessageBox::question(this, tr("取消传输"), tr("确定要取消文件传输吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (isSender) {
            fileTransferManager.cancelTransfer(sessionId);
        } else {
            // 对于接收方，直接关闭对话框即可
            reject();
        }

        close();
    }
}

void FileTransferDialog::updateProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes)
{
    if (this->sessionId != sessionId) return;

    // 更新进度条
    ui->progressBar->setMaximum(static_cast<int>(totalBytes));
    ui->progressBar->setValue(static_cast<int>(bytesTransferred));

    // 更新进度文本
    double progress = (totalBytes > 0) ? (static_cast<double>(bytesTransferred) / totalBytes) * 100 : 0;
    ui->lblProgress->setText(tr("进度：%1% (%2/%3)").arg(progress, 0, 'f', 1)
        .arg(formatFileSize(bytesTransferred)).arg(formatFileSize(totalBytes)));
}

void FileTransferDialog::onTransferStatusChanged(QUuid sessionId, FileTransferStatus status)
{
    if (this->sessionId != sessionId) return;

    updateUIState(status);
}

void FileTransferDialog::onTransferCompleted(QUuid sessionId, bool success)
{
    if (this->sessionId != sessionId) return;

    if (success) {
        QMessageBox::information(this, tr("传输完成"), tr("文件传输成功完成！"));
        accept();
    } else {
        QMessageBox::warning(this, tr("传输失败"), tr("文件传输失败！"));
        reject();
    }
}

void FileTransferDialog::initReceiverUI()
{
    setWindowTitle(tr("接收文件"));

    // 隐藏发送方控件
    ui->btnPause->setVisible(false);

    // 显示请求信息
    ui->lblRequestInfo->setText(tr("用户 %1 向您发送文件：").arg(transferRequest.getSenderId().toString().left(8)));
    ui->lblFileName->setText(tr("文件名：%1").arg(transferRequest.getFileName()));
    ui->lblFileSize->setText(tr("文件大小：%1").arg(formatFileSize(transferRequest.getFileSize())));

    // 连接信号
    connect(&fileTransferManager, &FileTransferManager::transferProgress, this, &FileTransferDialog::updateProgress);
    connect(&fileTransferManager, &FileTransferManager::transferStatusChanged, this, &FileTransferDialog::onTransferStatusChanged);
    connect(&fileTransferManager, &FileTransferManager::transferCompleted, this, &FileTransferDialog::onTransferCompleted);
}

void FileTransferDialog::initSenderUI()
{
    setWindowTitle(tr("发送文件"));

    // 隐藏接收方控件
    ui->widgetRequest->setVisible(false);
    ui->widgetProgress->setVisible(true);

    // 获取会话信息
    FileTransferSession *session = fileTransferManager.getTransferSession(sessionId);
    if (session) {
        ui->lblFileName->setText(session->getFileName());
        ui->progressBar->setMaximum(static_cast<int>(session->getFileSize()));
        updateUIState(session->getStatus());
    }

    // 连接信号
    connect(&fileTransferManager, &FileTransferManager::transferProgress, this, &FileTransferDialog::updateProgress);
    connect(&fileTransferManager, &FileTransferManager::transferStatusChanged, this, &FileTransferDialog::onTransferStatusChanged);
    connect(&fileTransferManager, &FileTransferManager::transferCompleted, this, &FileTransferDialog::onTransferCompleted);
}

void FileTransferDialog::updateUIState(FileTransferStatus status)
{
    QString statusText;
    QColor statusColor;

    switch (status) {
        case FileTransferStatus::Pending:
            statusText = tr("等待接收方确认...");
            statusColor = Qt::blue;
            break;
        case FileTransferStatus::Accepted:
            statusText = tr("接收方已接受，开始传输...");
            statusColor = Qt::darkGreen;
            break;
        case FileTransferStatus::Rejected:
            statusText = tr("接收方已拒绝");
            statusColor = Qt::red;
            break;
        case FileTransferStatus::Transferring:
            statusText = tr("传输中...");
            statusColor = Qt::darkGreen;
            ui->btnPause->setText(tr("暂停"));
            break;
        case FileTransferStatus::Paused:
            statusText = tr("已暂停");
            statusColor = Qt::darkYellow;
            ui->btnPause->setText(tr("恢复"));
            break;
        case FileTransferStatus::Completed:
            statusText = tr("传输完成");
            statusColor = Qt::darkGreen;
            break;
        case FileTransferStatus::Failed:
            statusText = tr("传输失败");
            statusColor = Qt::red;
            break;
        case FileTransferStatus::Cancelled:
            statusText = tr("已取消");
            statusColor = Qt::red;
            break;
    }

    ui->lblStatus->setText(statusText);
    ui->lblStatus->setStyleSheet(QString("color: %1;").arg(statusColor.name()));

    // 更新按钮状态
    ui->btnPause->setEnabled(status == FileTransferStatus::Transferring || status == FileTransferStatus::Paused);
    ui->btnCancel->setEnabled(status != FileTransferStatus::Completed && status != FileTransferStatus::Failed);
}

QString FileTransferDialog::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes < KB) {
        return tr("%1 B").arg(bytes);
    } else if (bytes < MB) {
        return tr("%1 KB").arg(static_cast<double>(bytes) / KB, 0, 'f', 1);
    } else if (bytes < GB) {
        return tr("%1 MB").arg(static_cast<double>(bytes) / MB, 0, 'f', 1);
    } else {
        return tr("%1 GB").arg(static_cast<double>(bytes) / GB, 0, 'f', 1);
    }
}

} // namespace LocalNetworkApp
