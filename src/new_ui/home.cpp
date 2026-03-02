#include "file_transfer_dialog.h"
#include "home.h"
#include "ui_home.h"
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QMessageBox>
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
    // setLayout(ui->out_lay);
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

void Home::onUserCardClicked(const QString &userId, UserState state)
{
    qDebug() << "用户卡片被点击：" << userId;

    // 根据用户状态决定操作
    switch (state) {
    case UserState::Online:
        // TODO: 打开聊天窗口
        // openChatWindow(userId);
        break;
    case UserState::DoNotDisturb:
        QMessageBox::information(this, tr("提示"), tr("该用户正在勿扰模式，无法发送消息"));
        // 但仍可以发送文件（根据需求）
        break;
    default:
        break;
    }
}

void Home::showUserContextMenu(fancy::IntroductionCard *card, const QString &userId, const QPoint &pos)
{
    QMenu menu;

    // 根据需求添加菜单项
    menu.addAction(tr("发送消息"), this, [this, userId]() {
        //TODO: 打开聊天窗口
        // openChatWindow(userId);
    });

    menu.addAction(tr("发送文件"), this, [this, userId]() {
        // TODO: 发送文件
        // sendFileToUser(userId);
    });

    menu.addSeparator();

    // 检查是否是联系人
    ContactInfo contact = contactManager.getContact(QUuid(userId));
    if (!contact.id.isNull()) {
        menu.addAction(tr("设置备注"), this, [this, userId]() {
            // TODO: 设置备注
            // setUserRemark(userId);
        });

        if (contactManager.isInBlacklist(QUuid(userId))) {
            menu.addAction(tr("移出黑名单"), this, [this, userId]() {
                // TODO: 移出黑名单
                // removeFromBlacklist(userId);
            });
        } else {
            menu.addAction(tr("加入黑名单"), this, [this, userId]() {
                // TODO: 加入黑名单
                // addToBlacklist(userId);
            });
        }

        if (contactManager.isInWhitelist(QUuid(userId))) {
            menu.addAction(tr("移出白名单"), this, [this, userId]() {
                // TODO: 移出白名单
                // removeFromWhitelist(userId);
            });
        } else {
            menu.addAction(tr("加入白名单"), this, [this, userId]() {
                // TODO: 加入白名单
                // addToWhitelist(userId);
            });
        }
    } else {
        menu.addAction(tr("添加为联系人"), this, [this, userId]() {
            // TODO: 添加为联系人
            // addAsContact(userId);
        });
    }

    menu.exec(card->mapToGlobal(pos));
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

void Home::clearUserCards()
{
    QList<fancy::IntroductionCard*> cards = ui->user_list_widget->findChildren<fancy::IntroductionCard*>();
    for (fancy::IntroductionCard* card : cards) {
        card->deleteLater();
    }

    QList<QLabel*> labels = ui->user_list_widget->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        label->deleteLater();
    }
}

void Home::updateUserCardInfo(const QUuid &userId)
{
    // 查找对应的卡片
    QList<fancy::IntroductionCard*> cards = ui->user_list_widget->findChildren<fancy::IntroductionCard*>();
    for (fancy::IntroductionCard* card : cards) {
        if (card->property("userId").toString() == userId.toString()) {
            // 更新卡片信息
            ContactInfo contact = contactManager.getContact(userId);
            if (!contact.id.isNull() && !contact.remark.isEmpty()) {
                card->setMainText(contact.remark);
            }
            break;
        }
    }
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

// 界面切换
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

void Home::updateUserList()
{
    // 清除现有的卡片
    QLayout* layout = ui->user_list_widget->layout();
    if (!layout) {
        // 如果是第一次调用，创建布局
        QGridLayout* gridLayout = new QGridLayout(ui->user_list_widget);
        layout = gridLayout;
        ui->user_list_widget->setLayout(layout);
    }
    // 清除现有卡片
    clearUserCards();
    // 获取已发现的用户
    QList<DiscoveredUser> users = userDiscovery->getDiscoveredUsers();

    int row = 0;
    int col = 0;
    const int MAX_COLS = 3; // 每行最多显示3个卡片
    int visibleUserCount = 0;

    for (const DiscoveredUser &user : users) {
        // 跳过隐身用户
        if (user.state == UserState::Invisible) {
            continue;
        }

        visibleUserCount++;

        // 创建卡片控件
        fancy::IntroductionCard* card = new fancy::IntroductionCard(ui->user_list_widget, ui->user_list_widget);
        card->setBlurRadius(30);

        // 根据状态设置不同的图标
        fancy::BootstrapIcons iconType = fancy::BootstrapIcons::Person; // 默认图标
        QString statusText;
        QColor statusColor = Qt::gray;

        switch (user.state) {
        case UserState::Online:
            iconType = fancy::BootstrapIcons::PersonCheck;
            statusText = tr("在线");
            statusColor = QColor(76, 175, 80); // 绿色
            break;
        case UserState::DoNotDisturb:
            iconType = fancy::BootstrapIcons::PersonX;
            statusText = tr("勿扰");
            statusColor = QColor(244, 67, 54); // 红色
            break;
        default:
            iconType = fancy::BootstrapIcons::Person;
            statusText = tr("未知");
            statusColor = Qt::gray;
        }

        card->setIcon(fancy::iconId(iconType));

        // 设置卡片主文本（使用昵称）
        QString displayText = user.nickname;
        card->setMainText(displayText);

        // 设置副文本（状态和IP）
        QString subText = QString("%1 - %2").arg(statusText).arg(user.address.toString());

        // 检查是否已添加为联系人
        ContactInfo contact = contactManager.getContact(user.userId);
        if (!contact.id.isNull()) {
            // 如果有备注，优先显示备注
            if (!contact.remark.isEmpty()) {
                card->setMainText(contact.remark);
                card->setToolTip(tr("昵称：%1\n备注：%2").arg(user.nickname).arg(contact.remark));
            } else {
                card->setToolTip(tr("昵称：%1").arg(user.nickname));
            }

            // 检查黑名单/白名单状态
            if (contactManager.isInBlacklist(user.userId)) {
                subText += " - " + tr("黑名单");
                // card->setBackgroundColor(QColor(240, 240, 240)); // 浅灰色背景
                card->setIcon(fancy::iconId(fancy::BootstrapIcons::PersonSlash));
            } else if (contactManager.isInWhitelist(user.userId)) {
                subText += " - " + tr("白名单");
                // card->setBorderColor(Qt::blue);
            }
        } else {
            card->setToolTip(tr("昵称：%1").arg(user.nickname));
        }

        card->setSubText(subText);

        // 设置状态颜色指示
        QPixmap statusPixmap(12, 12);
        statusPixmap.fill(statusColor);
        // 如果 IntroductionCard 支持设置额外的图标或状态指示，可以在这里设置

        // 存储用户ID到卡片的自定义属性中
        card->setProperty("userId", user.userId.toString());
        card->setProperty("userState", static_cast<int>(user.state));
        card->setProperty("userAddress", user.address.toString());
        card->setProperty("userNickname", user.nickname);

        // 连接卡片的点击信号
        connect(card, &fancy::IntroductionCard::clicked, this, [this, card]() {
            QString userId = card->property("userId").toString();
            UserState state = static_cast<UserState>(card->property("userState").toInt());
            onUserCardClicked(userId, state);
        });

        // 如果需要右键菜单
        card->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(card, &fancy::IntroductionCard::customContextMenuRequested, this, [this, card](const QPoint &pos) {
            QString userId = card->property("userId").toString();
            showUserContextMenu(card, userId, pos);
        });

        // 添加到布局
        QGridLayout* gridLayout = qobject_cast<QGridLayout*>(layout);
        if (gridLayout) {
            gridLayout->addWidget(card, row, col);

            // 更新行列位置
            col++;
            if (col >= MAX_COLS) {
                col = 0;
                row++;
            }
        } else {
            // 如果是其他布局，直接添加
            layout->addWidget(card);
        }
    }

    // 如果没有用户，显示提示
    if (visibleUserCount == 0) {
        QLabel* emptyLabel = new QLabel(tr("没有发现其他在线用户"), ui->user_list_widget);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("QLabel { color: #999; font-size: 14pt; padding: 50px; }");
        layout->addWidget(emptyLabel);
    }

    // 添加弹簧填充剩余空间（如果是网格布局）
    if (QGridLayout* gridLayout = qobject_cast<QGridLayout*>(layout)) {
        // 在最后一行添加弹簧
        gridLayout->setRowStretch(row + 1, 1);
        // 设置列拉伸
        for (int i = 0; i < MAX_COLS; i++) {
            gridLayout->setColumnStretch(i, 1);
        }
    } else {
        // 添加垂直弹簧
        qobject_cast<QVBoxLayout*>(layout)->addStretch();
    }

    // 更新状态信息
    qDebug() << QString("已发现 %1 个在线用户").arg(visibleUserCount);
}


void Home::on_content_widget_currentChanged(int arg1)
{
    qDebug() << "当前页面索引： " << arg1 ;
}

