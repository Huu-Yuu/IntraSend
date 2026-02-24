#include "user_radar_dialog.h"
#include "ui_user_radar_dialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QListWidgetItem>

namespace LocalNetworkApp {

UserRadarDialog::UserRadarDialog(UserDiscovery &userDiscovery, ContactManager &contactManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserRadarDialog)
    , userDiscovery(userDiscovery)
    , contactManager(contactManager)
{
    ui->setupUi(this);
    setWindowTitle(tr("用户雷达"));
    setModal(true);

    // 初始刷新
    refreshUI();

    // 定时刷新
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &UserRadarDialog::updateUserList);
    timer->start(2000);

    // 连接信号
    connect(ui->btnRefresh, &QPushButton::clicked, this, &UserRadarDialog::refreshUI);
}

UserRadarDialog::~UserRadarDialog()
{
    delete ui;
}

void UserRadarDialog::updateUserList()
{
    ui->listUsers->clear();

    // 获取已发现的用户
    QList<DiscoveredUser> users = userDiscovery.getDiscoveredUsers();

    for (const DiscoveredUser &user : users) {
        // 跳过隐身用户
        if (user.state == UserState::Invisible) {
            continue;
        }

        // 创建列表项
        QListWidgetItem *item = new QListWidgetItem();

        // 设置显示文本
        QString statusText;
        switch (user.state) {
            case UserState::Online:
                statusText = tr("[在线]");
                break;
            case UserState::DoNotDisturb:
                statusText = tr("[勿扰]");
                break;
            default:
                statusText = tr("[未知]");
        }

        QString displayText = QString("%1 %2 - %3").arg(user.nickname).arg(statusText).arg(user.address.toString());
        item->setText(displayText);

        // 存储用户ID
        item->setData(Qt::UserRole, user.userId.toString());

        // 检查是否已添加为联系人
        ContactInfo contact = contactManager.getContact(user.userId);
        if (!contact.id.isNull()) {
            item->setForeground(Qt::darkGreen);

            // 显示备注
            if (!contact.remark.isEmpty()) {
                item->setToolTip(tr("备注：%1").arg(contact.remark));
            }

            // 检查黑名单/白名单状态
            if (contactManager.isInBlacklist(user.userId)) {
                item->setBackground(Qt::lightGray);
                item->setToolTip(item->toolTip() + tr(" - 黑名单"));
            } else if (contactManager.isInWhitelist(user.userId)) {
                item->setToolTip(item->toolTip() + tr(" - 白名单"));
            }
        }

        ui->listUsers->addItem(item);
    }

    // 更新状态信息
    ui->lblStatus->setText(tr("已发现 %1 个在线用户").arg(ui->listUsers->count()));
}

void UserRadarDialog::on_btnAddContact_clicked()
{
    QUuid userId = getSelectedUserId();
    if (userId.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个用户！"));
        return;
    }

    // 检查是否已添加
    ContactInfo existingContact = contactManager.getContact(userId);
    if (!existingContact.id.isNull()) {
        QMessageBox::information(this, tr("提示"), tr("该用户已添加为联系人！"));
        return;
    }

    // 获取用户信息
    DiscoveredUser user = userDiscovery.getDiscoveredUser(userId);

    // 创建联系人信息
    ContactInfo contact;
    contact.id = userId;
    contact.nickname = user.nickname;
    contact.state = user.state;
    contact.lastSeen = user.lastSeen;

    // 添加联系人
    contactManager.addContact(contact);

    QMessageBox::information(this, tr("成功"), tr("已添加 %1 为联系人！").arg(user.nickname));
    refreshUI();
}

void UserRadarDialog::on_btnSetRemark_clicked()
{
    QUuid userId = getSelectedUserId();
    if (userId.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个用户！"));
        return;
    }

    // 检查是否已添加
    ContactInfo existingContact = contactManager.getContact(userId);
    if (existingContact.id.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先将该用户添加为联系人！"));
        return;
    }

    // 获取当前备注
    QString currentRemark = existingContact.remark;

    // 输入新备注
    bool ok;
    QString remark = QInputDialog::getText(this, tr("设置备注"),
        tr("请输入备注名称："), QLineEdit::Normal, currentRemark, &ok);

    if (ok && !remark.isEmpty()) {
        contactManager.setContactRemark(userId, remark);
        QMessageBox::information(this, tr("成功"), tr("备注已更新！"));
        refreshUI();
    }
}

void UserRadarDialog::on_btnBlacklist_clicked()
{
    QUuid userId = getSelectedUserId();
    if (userId.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个用户！"));
        return;
    }

    // 检查是否已添加
    ContactInfo existingContact = contactManager.getContact(userId);
    if (existingContact.id.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先将该用户添加为联系人！"));
        return;
    }

    if (contactManager.isInBlacklist(userId)) {
        // 移出黑名单
        contactManager.removeFromBlacklist(userId);
        QMessageBox::information(this, tr("成功"), tr("已将用户移出黑名单！"));
    } else {
        // 加入黑名单
        int ret = QMessageBox::question(this, tr("确认"), tr("确定要将该用户加入黑名单吗？"),
            QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            contactManager.addToBlacklist(userId);
            QMessageBox::information(this, tr("成功"), tr("已将用户加入黑名单！"));
        }
    }

    refreshUI();
}

void UserRadarDialog::on_btnWhitelist_clicked()
{
    QUuid userId = getSelectedUserId();
    if (userId.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择一个用户！"));
        return;
    }

    // 检查是否已添加
    ContactInfo existingContact = contactManager.getContact(userId);
    if (existingContact.id.isNull()) {
        QMessageBox::warning(this, tr("提示"), tr("请先将该用户添加为联系人！"));
        return;
    }

    if (contactManager.isInWhitelist(userId)) {
        // 移出白名单
        contactManager.removeFromWhitelist(userId);
        QMessageBox::information(this, tr("成功"), tr("已将用户移出白名单！"));
    } else {
        // 加入白名单
        contactManager.addToWhitelist(userId);
        QMessageBox::information(this, tr("成功"), tr("已将用户加入白名单！"));
    }

    refreshUI();
}

void UserRadarDialog::on_listUsers_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    on_btnAddContact_clicked();
}

QUuid UserRadarDialog::getSelectedUserId() const
{
    QListWidgetItem *item = ui->listUsers->currentItem();
    if (!item) {
        return QUuid();
    }

    QString userIdStr = item->data(Qt::UserRole).toString();
    return QUuid(userIdStr);
}

void UserRadarDialog::refreshUI()
{
    updateUserList();

    // 更新按钮状态
    bool hasSelection = (ui->listUsers->currentItem() != nullptr);
    ui->btnAddContact->setEnabled(hasSelection);
    ui->btnSetRemark->setEnabled(hasSelection);
    ui->btnBlacklist->setEnabled(hasSelection);
    ui->btnWhitelist->setEnabled(hasSelection);
}

} // namespace LocalNetworkApp
