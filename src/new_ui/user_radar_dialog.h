#ifndef USER_RADAR_DIALOG_H
#define USER_RADAR_DIALOG_H

#include <QDialog>
#include <QUuid>
#include <QListWidgetItem>
#include "core/network/user_discovery.h"
#include "core/user/contact_manager.h"


QT_BEGIN_NAMESPACE
namespace Ui { class UserRadarDialog; }
QT_END_NAMESPACE

namespace LocalNetworkApp {

class UserRadarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserRadarDialog(UserDiscovery &userDiscovery, ContactManager &contactManager, QWidget *parent = nullptr);
    ~UserRadarDialog() override;

private slots:
    // 更新用户列表
    void updateUserList();

    // 添加联系人
    void on_btnAddContact_clicked();

    // 设置备注
    void on_btnSetRemark_clicked();

    // 拉黑/白名单操作
    void on_btnBlacklist_clicked();
    void on_btnWhitelist_clicked();

    // 双击用户项（快速添加）
    void on_listUsers_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::UserRadarDialog *ui;
    UserDiscovery &userDiscovery;   // 用户发现服务（引用）
    ContactManager &contactManager; // 联系人管理器（引用）

    // 获取选中的用户ID
    QUuid getSelectedUserId() const;

    // 刷新界面显示
    void refreshUI();
};

} // namespace LocalNetworkApp

#endif // USER_RADAR_DIALOG_H
