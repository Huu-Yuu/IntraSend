#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QUuid>
#include "core/user/userIdentity.h"
#include "core/user/contact_manager.h"
#include "core/user/user_status.h"
#include "core/message/message_manager.h"
#include "core/filetransfer/file_transfer_manager.h"
#include "core/network/server.h"
#include "core/network/client.h"
#include "core/network/user_discovery.h"
#include "core/data/password_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace LocalNetworkApp {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 初始化程序
    void initialize();

    // 锁定/解锁程序
    void lockApp();
    bool unlockApp(const QString &password);

protected:
    // 重写拖拽事件
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    // 重写关闭事件（最小化到托盘）
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 菜单操作
    void on_actionSettings_triggered();
    void on_actionUserRadar_triggered();
    void on_actionClearHistory_triggered();
    void on_actionIncognitoMode_triggered(bool checked);
    void on_actionLockApp_triggered();
    void on_actionExit_triggered();

    // 状态切换
    void on_actionOnline_triggered();
    void on_actionDoNotDisturb_triggered();
    void on_actionInvisible_triggered();

    // 联系人列表操作
    void onContactSelected(QUuid contactId);
    void onContactContextMenu(const QPoint &pos);

    // 消息相关
    void onSendMessage();
    void onMessageReceived(const Message &message);
    void onUnreadCountChanged(int count);

    // 文件传输相关
    void onFileTransferRequestReceived(const FileTransferRequest &request);
    void onFileTransferProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes);
    void onFileTransferCompleted(QUuid sessionId, bool success);

    // 用户发现相关
    void onUserDiscovered(const DiscoveredUser &user);
    void onUserLost(QUuid userId);
    void onUserStateChanged(QUuid userId, UserState state);

    // 托盘图标操作
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    Ui::MainWindow *ui;
    UserIdentity userIdentity;               // 当前用户身份
    ContactManager contactManager;           // 联系人管理器
    MessageManager messageManager;           // 消息管理器
    FileTransferManager fileTransferManager; // 文件传输管理器
    Server *server;                          // TCP服务器
    Client *client;                          // TCP客户端
    UserDiscovery *userDiscovery;            // 用户发现服务
    PasswordManager passwordManager;         // 密码管理器
    QSystemTrayIcon *trayIcon;               // 系统托盘图标
    QUuid currentContactId;                  // 当前选中的联系人ID
    bool isAppLocked;                        // 程序是否锁定
    bool incognitoMode;                      // 无痕模式

    // 初始化UI
    void initUI();

    // 初始化网络服务
    void initNetwork();

    // 初始化托盘图标
    void initTrayIcon();

    // 更新联系人列表
    void updateContactList();

    // 更新状态栏
    void updateStatusBar(UserState state);

    // 显示文件传输对话框
    void showFileTransferDialog(const FileTransferRequest &request);

    // 加载样式表
    void loadStylesheet();
};

} // namespace LocalNetworkApp

#endif // MAINWINDOW_H
