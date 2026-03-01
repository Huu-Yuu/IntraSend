#include "file_transfer_dialog.h"
#include "home.h"
#include "ui_home.h"
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QMouseEvent>

Home::Home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::main_widget)
    , fileTransferManager(&contactManager, &messageManager, this)
    , server(nullptr)
    , client(nullptr)
    , userDiscovery(nullptr)
    , trayIcon(nullptr)
    , isAppLocked(false)
    , incognitoMode(false)
{
    ui->setupUi(this);
    InitUi();
    InitMember();
}

Home::~Home()
{
    delete ui;
}

void Home::InitUi()
{
    resize(1000, 740);
    //初始化窗口边框
    // this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    installEventFilter(this);
    setProperty("canMove",true);
    // QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    // shadow->setOffset(0, 0);
    // shadow->setColor(QColor("#444444"));
    // shadow->setBlurRadius(16);
    // ui->w_bg->setGraphicsEffect(shadow);
    ui->out_lay->setContentsMargins(22,22,22,22);
    setLayout(ui->out_lay);
    //Logo
    QSize size(30,30);
    QPixmap logo(":/pic/icons/logo.png");
    ui->btn_logo->setIcon(logo);
    ui->btn_logo->setIconSize(QSize(30,30));
    ui->btn_home->setIcon(fancy::IconPark::Home);
    ui->btn_home->setIconSize(size);
    ui->btn_logout->setIcon(fancy::IconPark::Close);
    ui->btn_logout->setIconSize(size);
    ui->btn_littleshow->setIcon(fancy::IconPark::Minus);
    ui->btn_littleshow->setIconSize(size);
    ui->btn_mine->setIcon(fancy::IconPark::Panda);

    // 消息管理器信号
    connect(&messageManager, &MessageManager::messageReceived, this, &Home::onMessageReceived);
    connect(&messageManager, &MessageManager::unreadMessageCountChanged, this, &Home::onUnreadCountChanged);

    // 文件传输管理器信号
    connect(&fileTransferManager, &FileTransferManager::fileTransferRequestReceived, this, &Home::onFileTransferRequestReceived);
    connect(&fileTransferManager, &FileTransferManager::transferProgress, this, &Home::onFileTransferProgress);
    connect(&fileTransferManager, &FileTransferManager::transferCompleted, this, &Home::onFileTransferCompleted);
}

void Home::InitMember()
{

    //最小化到托盘
    QIcon icon = QIcon(":/icons/logo.ico");//设置最小图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QPixmap(":/pic/icons/logo.png"));
    trayIcon->setToolTip("内网传书"); //提示文字
    //添加托盘列表项(还原与退出)
    returnNormal = new QAction(" 显示界面", this);
    returnNormal->setFont(QFont("Arial", 9));
    returnNormal->setObjectName("returnNormal");
    returnNormal->setIcon(QIcon(":/pic/icons/show_win.png"));
    quitAction = new QAction(" 程序退出", this);
    quitAction->setFont(QFont("Arial", 9));
    quitAction->setObjectName("quitAction");
    quitAction->setIcon(QIcon(":/pic/icons/quit.png"));
    // connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);    //绑定槽函数退出
    connect(returnNormal, &QAction::triggered, this, &Home::showNormal);   //绑定槽函数还原界面

    //创建托盘菜单(必须先创建动作，后添加菜单项，还可以加入菜单项图标美化)
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(returnNormal);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Home::onTrayIconActivated);
}

void Home::onFileTransferCompleted(QUuid sessionId, bool success)
{
    Q_UNUSED(sessionId);

    if (success) {
        trayIcon->showMessage("内网传书",
                              "文件传输完成",
                              QSystemTrayIcon::Information, 1000);
    } else {
        trayIcon->showMessage("内网传书",
                              "文件传输失败",
                              QSystemTrayIcon::Information, 1000);
    }
}

void Home::onFileTransferRequestReceived(const FileTransferRequest &request)
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

void Home::showFileTransferDialog(const FileTransferRequest &request)
{
    FileTransferDialog dialog(request, fileTransferManager, this);
    dialog.exec();
}

void Home::onUnreadCountChanged(int count)
{
    // 更新窗口标题
    setWindowTitle(tr("内网传书 - %1条未读消息").arg(count));

    // 更新联系人列表
    updateContactList();
}


void Home::onMessageReceived(const Message &message)
{
    // 如果程序锁定，仅接收白名单消息
    if (isAppLocked) {
        if (!contactManager.isInWhitelist(message.getSenderId())) {
            return;
        }
    }

    // 添加到消息显示
    if (message.getSenderId() == currentContactId) {
        ui->msg_list->addItem(message.getContent());
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

void Home::updateContactList()
{
    // 获取所有联系人
    const QList<ContactInfo> contacts = contactManager.getAllContacts();
    for(const auto& info : contacts)
    {
        // 更新联系人列表控件
        ui->list_users->addItem(info.nickname);

    }

    // 更新未读计数显示
    // ui->contactListWidget->setUnreadCounts(messageManager.getUnreadMessageCount());
}

void Home::onFileTransferProgress(QUuid sessionId, qint64 bytesTransferred, qint64 totalBytes)
{
    // 更新文件传输进度（如果对应的对话框已打开）
    Q_UNUSED(sessionId);
    Q_UNUSED(bytesTransferred);
    Q_UNUSED(totalBytes);
}

void Home::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:  // 双击
        showNormal();
        break;
    case QSystemTrayIcon::Trigger:      // 单击（可选，根据需求添加）
        break;
    case QSystemTrayIcon::Context:      // 右键
        // 右键菜单会自动显示，不需要处理
        break;
    default:
        break;
    }
}

void Home::LittleShow()
{
    this->hide();//隐藏主窗口
    trayIcon->show();//显示托盘

    //显示到系统提示框的信息
    trayIcon->showMessage("内网传书",
                          "程序已最小化到系统托盘，双击托盘图标可重新显示窗口",
                          QSystemTrayIcon::Information, 2000);
}

void Home::on_btn_littleshow_clicked()
{
    LittleShow();
}

void Home::on_btn_logout_clicked()
{
    close();
}

bool Home::eventFilter(QObject *obj, QEvent *evt)
{
    QWidget *w = qobject_cast<QWidget*>(obj);
    if (!w || !w->property("canMove").toBool()) {
        return QObject::eventFilter(obj, evt);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;

    if (evt->type() == QEvent::MouseButtonPress) {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (event->button() == Qt::LeftButton) {
            mousePressed = true;
            mousePoint = event->globalPosition().toPoint() - w->pos(); // 修正：使用 globalPosition()
            return true;
        }
    } else if (evt->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (event->button() == Qt::LeftButton) {
            mousePressed = false;
            return true;
        }
    } else if (evt->type() == QEvent::MouseMove) {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (mousePressed && (event->buttons() & Qt::LeftButton)) { // 修正：使用位与运算符 &
            w->move(event->globalPosition().toPoint() - mousePoint); // 修正：使用 globalPosition()
            return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

void Home::on_btn_contact_clicked()
{
    ui->content_widget->setCurrentWidget(ui->contact_page);
}


void Home::on_btn_radar_clicked()
{
    ui->content_widget->setCurrentWidget(ui->radar_page);
}


void Home::on_btn_file_trans_clicked()
{
    ui->content_widget->setCurrentWidget(ui->file_trans_page);
}


void Home::on_btn_program_lock_clicked()
{
    ui->content_widget->setCurrentWidget(ui->lock_page);
}


void Home::on_btn_sys_config_clicked()
{
    ui->content_widget->setCurrentWidget(ui->sys_config_page);
}


void Home::on_btn_home_clicked()
{
    ui->content_widget->setCurrentWidget(ui->home_page);
}


void Home::on_content_widget_currentChanged(int arg1)
{
    qDebug() << "当前页面索引： " << arg1 ;
}

