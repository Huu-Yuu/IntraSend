#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "user_radar_dialog.h"
#include "file_transfer_dialog.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QStyle>
#include "core/utils/constants.h"

namespace LocalNetworkApp {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileTransferManager(&contactManager, &messageManager, this)
    , server(nullptr)
    , client(nullptr)
    , userDiscovery(nullptr)
    , trayIcon(nullptr)
    , isAppLocked(false)
    , incognitoMode(false)
{
    ui->setupUi(this);
    initUI();
}

MainWindow::~MainWindow()
{
    // 停止网络服务
    if (userDiscovery) {
        userDiscovery->stopDiscovery();
        delete userDiscovery;
    }

    if (server) {
        server->stop();
        delete server;
    }

    if (client) {
        client->disconnectFromServer();
        delete client;
    }

    // 销毁托盘图标
    if (trayIcon) {
        delete trayIcon;
    }

    delete ui;
}

void MainWindow::initialize()
{
    // 加载用户身份
    userIdentity = UserIdentity::loadFromLocal();
    if (userIdentity.getNickname() == Constants::DEFAULT_NICKNAME) {
        // 首次使用，设置昵称
        bool ok;
        QString nickname = QInputDialog::getText(this, tr("设置昵称"),
            tr("请输入您的昵称："), QLineEdit::Normal,
            Constants::DEFAULT_NICKNAME, &ok);

        if (ok && !nickname.isEmpty()) {
            userIdentity.setNickname(nickname);
            userIdentity.saveToLocal();
        }
    }

    // 加载联系人管理器
    contactManager = ContactManager::loadFromLocal();

    // 初始化网络服务
    initNetwork();

    // 更新UI
    updateContactList();
    updateStatusBar(UserState::Online);

    // 检查密码保护
    if (passwordManager.hasPasswords()) {
        lockApp();
    }
}

void MainWindow::lockApp()
{
    isAppLocked = true;

    // 禁用所有界面元素
    ui->centralWidget->setEnabled(false);
    ui->menuBar->setEnabled(false);

    // 显示解锁对话框
    bool ok;
    QString password = QInputDialog::getText(this, tr("程序已锁定"),
        tr("请输入解锁密码："), QLineEdit::Password, "", &ok);

    if (ok) {
        unlockApp(password);
    } else {
        // 最小化到托盘
        hide();
    }
}

bool MainWindow::unlockApp(const QString &password)
{
    if (passwordManager.verifyPassword(password)) {
        isAppLocked = false;
        ui->centralWidget->setEnabled(true);
        ui->menuBar->setEnabled(true);
        return true;
    } else {
        QMessageBox::warning(this, tr("密码错误"), tr("输入的密码不正确，请重试！"));
        lockApp();
        return false;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (isAppLocked) return;

    // 只接受文件拖拽
    if (event->mimeData()->hasUrls() && !currentContactId.isNull()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (isAppLocked) return;

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            if (!filePath.isEmpty()) {
                // 发起文件传输
                fileTransferManager.initiateFileTransfer(
                    userIdentity.getUuid(),
                    currentContactId,
                    filePath);
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 最小化到托盘，不退出程序
    if (trayIcon && trayIcon->isVisible()) {
        hide();
        event->ignore();

        // 显示托盘提示
        trayIcon->showMessage(tr("程序后台运行"),
            tr("程序已最小化到系统托盘，点击托盘图标可恢复窗口"),
            QSystemTrayIcon::Information, 2000);
    } else {
        event->accept();
    }
}

void MainWindow::on_actionSettings_triggered()
{
    if (isAppLocked) return;

    SettingsDialog dialog(userIdentity, passwordManager, this);
    dialog.exec();

    // 保存用户信息
    userIdentity.saveToLocal();
}

void MainWindow::on_actionUserRadar_triggered()
{
    if (isAppLocked) return;

    UserRadarDialog dialog(*userDiscovery, contactManager, this);
    dialog.exec();

    // 保存联系人信息
    contactManager.saveToLocal();
    updateContactList();
}

void MainWindow::on_actionClearHistory_triggered()
{
    if (isAppLocked) return;

    int ret = QMessageBox::question(this, tr("清除历史记录"),
        tr("确定要清除所有聊天和文件传输记录吗？此操作不可恢复！"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        messageManager.clearAllMessageHistory();
        fileTransferManager.clearAllTransferHistory();
        QMessageBox::information(this, tr("完成"), tr("历史记录已清除！"));
    }
}

void MainWindow::on_actionIncognitoMode_triggered(bool checked)
{
    if (isAppLocked) return;

    incognitoMode = checked;
    messageManager.setIncognitoMode(checked);
    fileTransferManager.setIncognitoMode(checked);

    // 更新UI提示
    ui->statusbar->showMessage(checked ? tr("无痕模式已开启") : tr("无痕模式已关闭"), 3000);
}

void MainWindow::on_actionLockApp_triggered()
{
    lockApp();
}

void MainWindow::on_actionExit_triggered()
{
    int ret = QMessageBox::question(this, tr("退出程序"),
        tr("确定要退出程序吗？"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        // 保存数据
        userIdentity.saveToLocal();
        contactManager.saveToLocal();
        messageManager.saveMessageHistory();

        // 退出程序
        qApp->quit();
    }
}

void MainWindow::on_actionOnline_triggered()
{
    if (isAppLocked) return;

    userDiscovery->setUserState(UserState::Online);
    updateStatusBar(UserState::Online);
}

void MainWindow::on_actionDoNotDisturb_triggered()
{
    if (isAppLocked) return;

    userDiscovery->setUserState(UserState::DoNotDisturb);
    updateStatusBar(UserState::DoNotDisturb);
}

void MainWindow::on_actionInvisible_triggered()
{
    if (isAppLocked) return;

    userDiscovery->setUserState(UserState::Invisible);
    updateStatusBar(UserState::Invisible);
}

void MainWindow::onContactSelected(QUuid contactId)
{
    if (isAppLocked) return;

    currentContactId = contactId;

    // 加载聊天历史
    QList<Message> history = messageManager.getMessageHistory(contactId);
    ui->messageDisplayWidget->setMessages(history);

    // 清除未读计数
    messageManager.markAsRead(contactId);

    // 更新UI
    ContactInfo contact = contactManager.getContact(contactId);
    ui->lblCurrentContact->setText(contact.remark.isEmpty() ? contact.nickname : contact.remark);
}

void MainWindow::onContactContextMenu(const QPoint &pos)
{
    if (isAppLocked) return;

    QMenu menu(this);

    // 添加菜单项
    menu.addAction(tr("设置备注"), this, [this]() {
        ContactInfo contact = contactManager.getContact(currentContactId);
        bool ok;
        QString remark = QInputDialog::getText(this, tr("设置备注"),
            tr("请输入备注名称："), QLineEdit::Normal,
            contact.remark, &ok);

        if (ok && !remark.isEmpty()) {
            contactManager.setContactRemark(currentContactId, remark);
            contactManager.saveToLocal();
            updateContactList();
        }
    });

    menu.addSeparator();

    if (contactManager.isInBlacklist(currentContactId)) {
        menu.addAction(tr("移出黑名单"), this, [this]() {
            contactManager.removeFromBlacklist(currentContactId);
            contactManager.saveToLocal();
            QMessageBox::information(this, tr("完成"), tr("已将联系人移出黑名单！"));
        });
    } else {
        menu.addAction(tr("加入黑名单"), this, [this]() {
            contactManager.addToBlacklist(currentContactId);
            contactManager.saveToLocal();
            QMessageBox::information(this, tr("完成"), tr("已将联系人加入黑名单！"));
        });
    }

    if (contactManager.isInWhitelist(currentContactId)) {
        menu.addAction(tr("移出白名单"), this, [this]() {
            contactManager.removeFromWhitelist(currentContactId);
            contactManager.saveToLocal();
        });
    } else {
        menu.addAction(tr("加入白名单"), this, [this]() {
            contactManager.addToWhitelist(currentContactId);
            contactManager.saveToLocal();
        });
    }

    menu.addSeparator();
    menu.addAction(tr("清除聊天记录"), this, [this]() {
        messageManager.clearMessageHistory(currentContactId);
        ui->messageDisplayWidget->clear();
    });

    // 显示菜单
    menu.exec(ui->contactListWidget->mapToGlobal(pos));
}

void MainWindow::onSendMessage()
{
    if (isAppLocked || currentContactId.isNull()) return;

    QString content = ui->messageInputWidget->getMessageText();
    if (content.isEmpty()) return;

    // 获取接收者状态
    ContactInfo contact = contactManager.getContact(currentContactId);
    UserStatus receiverStatus(currentContactId, contact.state);

    // 创建并发送消息
    Message message(userIdentity.getUuid(), currentContactId, content);
    bool success = messageManager.sendMessage(message, receiverStatus);

    if (success) {
        // 清空输入框
        ui->messageInputWidget->clear();

        // 显示发送的消息
        ui->messageDisplayWidget->addMessage(message, true);
    } else {
        QMessageBox::warning(this, tr("发送失败"), tr("对方处于勿扰模式，无法发送消息！"));
    }
}

void MainWindow::onMessageReceived(const Message &message)
{
    // 如果程序锁定，仅接收白名单消息
    if (isAppLocked) {
        if (!contactManager.isInWhitelist(message.getSenderId())) {
            return;
        }
    }

    // 添加到消息显示
    if (message.getSenderId() == currentContactId) {
        ui->messageDisplayWidget->addMessage(message, false);
    }

    // 更新联系人列表（显示未读）
    updateContactList();

    // 显示托盘提示
    if (isHidden() && trayIcon) {
        ContactInfo contact = contactManager.getContact(message.getSenderId());
        QString senderName = contact.remark.isEmpty() ? contact.nickname : contact.remark;

        trayIcon->showMessage(tr("新消息 - %1").arg(senderName),
            message.getContent(),
            QSystemTrayIcon::Information, 5000);
    }
}

void MainWindow::onUnreadCountChanged(int count)
{
    // 更新窗口标题
    setWindowTitle(tr("局域网通讯工具 - %1条未读消息").arg(count));

    // 更新联系人列表
    updateContactList();
}

void MainWindow::onFileTransferRequestReceived(const FileTransferRequest &request)
{
    // 如果程序锁定，仅接收白名单文件
    if (isAppLocked) {
        if (!contactManager.isInWhitelist(request.getSenderId())) {
            return;
        }
    }

    // 显示文件传输对话框
    showFileTransferDialog(request);
}

void MainWindow::onFileTransferProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes)
{
    // 更新文件传输进度（如果对应的对话框已打开）
    Q_UNUSED(sessionId);
    Q_UNUSED(bytesTransferred);
    Q_UNUSED(totalBytes);
}

void MainWindow::onFileTransferCompleted(QUuid sessionId, bool success)
{
    Q_UNUSED(sessionId);

    if (success) {
        ui->statusbar->showMessage(tr("文件传输完成！"), 3000);
    } else {
        ui->statusbar->showMessage(tr("文件传输失败！"), 3000);
    }
}

void MainWindow::onUserDiscovered(const DiscoveredUser &user)
{
    // 更新联系人状态
    contactManager.updateContactState(user.userId, user.state);
    updateContactList();
}

void MainWindow::onUserLost(QUuid userId)
{
    // 更新联系人状态
    contactManager.updateContactState(userId, UserState::Invisible);
    updateContactList();
}

void MainWindow::onUserStateChanged(QUuid userId, UserState state)
{
    // 更新联系人状态
    contactManager.updateContactState(userId, state);
    updateContactList();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        // 单击托盘图标，显示/隐藏窗口
        if (isHidden()) {
            show();
            raise();
            activateWindow();

            // 如果程序锁定，显示解锁对话框
            if (isAppLocked) {
                lockApp();
            }
        } else {
            hide();
        }
    }
}

void MainWindow::initUI()
{
    // 设置窗口属性
    setWindowTitle(tr("局域网通讯工具"));
    setMinimumSize(Constants::MIN_WINDOW_WIDTH, Constants::MIN_WINDOW_HEIGHT);

    // 加载样式表
    loadStylesheet();

    // 初始化托盘图标
    initTrayIcon();

    // 连接信号
    connect(ui->messageInputWidget, &MessageInputWidget::sendMessage, this, &MainWindow::onSendMessage);
    connect(ui->contactListWidget, &ContactListWidget::contactSelected, this, &MainWindow::onContactSelected);
    connect(ui->contactListWidget, &ContactListWidget::contextMenuRequested, this, &MainWindow::onContactContextMenu);

    // 消息管理器信号
    connect(&messageManager, &MessageManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(&messageManager, &MessageManager::unreadMessageCountChanged, this, &MainWindow::onUnreadCountChanged);

    // 文件传输管理器信号
    connect(&fileTransferManager, &FileTransferManager::fileTransferRequestReceived, this, &MainWindow::onFileTransferRequestReceived);
    connect(&fileTransferManager, &FileTransferManager::transferProgress, this, &MainWindow::onFileTransferProgress);
    connect(&fileTransferManager, &FileTransferManager::transferCompleted, this, &MainWindow::onFileTransferCompleted);
}

void MainWindow::initNetwork()
{
    // 启动用户发现服务
    userDiscovery = new UserDiscovery(userIdentity, this);
    connect(userDiscovery, &UserDiscovery::userDiscovered, this, &MainWindow::onUserDiscovered);
    connect(userDiscovery, &UserDiscovery::userLost, this, &MainWindow::onUserLost);
    connect(userDiscovery, &UserDiscovery::userStateChanged, this, &MainWindow::onUserStateChanged);
    userDiscovery->startDiscovery();

    // 启动TCP服务器
    server = new Server(&contactManager, this);
    server->start(Constants::DEFAULT_TCP_PORT);

    // 初始化客户端（用于连接其他用户）
    client = new Client(userIdentity, this);
}

void MainWindow::initTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));

    // 创建托盘菜单
    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(tr("显示窗口"), this, [this]() {
        show();
        raise();
        activateWindow();
    });
    trayMenu->addAction(tr("锁定程序"), this, &MainWindow::lockApp);
    trayMenu->addSeparator();
    trayMenu->addAction(tr("退出"), this, &MainWindow::on_actionExit_triggered);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->setToolTip(tr("局域网通讯工具"));
    trayIcon->show();

    // 连接托盘图标信号
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
}

void MainWindow::updateContactList()
{
    // 获取所有联系人
    QList<ContactInfo> contacts = contactManager.getAllContacts();

    // 更新联系人列表控件
    ui->contactListWidget->setContacts(contacts);

    // 更新未读计数显示
    ui->contactListWidget->setUnreadCounts(messageManager.getUnreadMessageCount());
}

void MainWindow::updateStatusBar(UserState state)
{
    QString statusText;
    QIcon statusIcon;

    switch (state) {
        case UserState::Online:
            statusText = tr("在线");
            statusIcon = style()->standardIcon(QStyle::SP_ArrowUp);
            break;
        case UserState::DoNotDisturb:
            statusText = tr("勿扰");
            statusIcon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
            break;
        case UserState::Invisible:
            statusText = tr("隐身");
            statusIcon = style()->standardIcon(QStyle::SP_ArrowDown);
            break;
    }

    // 更新状态栏
    ui->statusbar->showMessage(tr("当前状态：%1 | 用户ID：%2").arg(statusText).arg(userIdentity.getUuid().toString().left(8)));

    // 更新菜单状态
    ui->actionOnline->setChecked(state == UserState::Online);
    ui->actionDoNotDisturb->setChecked(state == UserState::DoNotDisturb);
    ui->actionInvisible->setChecked(state == UserState::Invisible);

    // 更新托盘图标提示
    if (trayIcon) {
        trayIcon->setToolTip(tr("局域网通讯工具 - %1").arg(statusText));
    }
}

void MainWindow::showFileTransferDialog(const FileTransferRequest &request)
{
    FileTransferDialog dialog(request, fileTransferManager, this);
    dialog.exec();
}

void MainWindow::loadStylesheet()
{
    // 简单的样式表配置（可根据需要扩展）
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f0f0f0;
        }

        QStatusBar {
            background-color: #e0e0e0;
            font-size: 12px;
        }

        QMenuBar {
            background-color: #3498db;
            color: white;
        }

        QMenuBar::item {
            background-color: #3498db;
            color: white;
            padding: 4px 10px;
        }

        QMenuBar::item:selected {
            background-color: #2980b9;
        }

        QMenu {
            background-color: white;
            border: 1px solid #ccc;
        }

        QMenu::item:selected {
            background-color: #3498db;
            color: white;
        }

        QListWidget {
            background-color: white;
            border: 1px solid #ccc;
        }

        QListWidget::item {
            padding: 5px;
            border-bottom: 1px solid #eee;
        }

        QListWidget::item:selected {
            background-color: #3498db;
            color: white;
        }

        QTextEdit {
            background-color: white;
            border: 1px solid #ccc;
            padding: 5px;
        }

        QLineEdit {
            padding: 5px;
            border: 1px solid #ccc;
            border-radius: 3px;
        }

        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 3px;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        QPushButton:pressed {
            background-color: #1f618d;
        }
    )";

    setStyleSheet(styleSheet);
}

} // namespace LocalNetworkApp
