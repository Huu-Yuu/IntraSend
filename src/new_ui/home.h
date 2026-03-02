#ifndef HOME_H
#define HOME_H

#include <QSystemTrayIcon>
#include <QWidget>
#include "IntroductionCard.h"
#include "core/data/password_manager.h"
#include "core/filetransfer/file_transfer_manager.h"
#include "core/message/message_manager.h"
#include "core/network/client.h"
#include "core/network/server.h"
#include "core/network/user_discovery.h"
#include "core/user/contact_manager.h"
using namespace LocalNetworkApp;
namespace Ui {
class main_widget;
}

class Home : public QWidget
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = nullptr);
    ~Home();

private slots:
    void on_btn_littleshow_clicked();
    void on_btn_logout_clicked();

    // 界面切换
    void on_btn_contact_clicked();
    void on_btn_radar_clicked();
    void on_btn_file_trans_clicked();
    void on_btn_program_lock_clicked();
    void on_btn_sys_config_clicked();
    void on_btn_home_clicked();
    void on_content_widget_currentChanged(int arg1);

    // 更新用户列表
    void updateUserList();

    // 消息相关
    void onMessageReceived(const LocalNetworkApp::Message &message);
    void onUnreadCountChanged(int count);

    // 文件传输相关
    void onFileTransferRequestReceived(const LocalNetworkApp::FileTransferRequest &request);
    void onFileTransferProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes);
    void onFileTransferCompleted(QUuid sessionId, bool success);

    // 用户雷达相关
    void onUserCardClicked(const QString &userId, LocalNetworkApp::UserState state);
    void showUserContextMenu(fancy::IntroductionCard *card, const QString &userId, const QPoint &pos);


protected:
    bool eventFilter(QObject *obj, QEvent *evt) override;

private:
    Ui::main_widget *ui;
    //窗口管理动作
    QAction *returnNormal;
    QAction *quitAction;
    //最小化到托盘
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    UserIdentity userIdentity;               // 当前用户身份
    ContactManager contactManager;           // 联系人管理器
    MessageManager messageManager;           // 消息管理器
    FileTransferManager fileTransferManager; // 文件传输管理器
    Server *server;                          // TCP服务器
    Client *client;                          // TCP客户端
    UserDiscovery *userDiscovery;            // 用户发现服务
    PasswordManager passwordManager;         // 密码管理器
    QUuid currentContactId;                  // 当前选中的联系人ID
    bool isAppLocked;                        // 程序是否锁定
    bool incognitoMode;                      // 无痕模式
    void InitUi();          //Ui界面初始化函数
    void InitMember();      //成员变量初始化函数
    void LittleShow();      //最小化显示函数
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason); // 托盘激活事件
    void updateContactList();   // 更新联系人列表
    void showFileTransferDialog(const FileTransferRequest &request);    // 显示文件传输对话框
    void clearUserCards();  // 清理用户卡片
    void updateUserCardInfo(const QUuid &userId); // 更新用户卡片
};

#endif // HOME_H
