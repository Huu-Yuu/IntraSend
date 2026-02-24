#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QUuid>
#include "core/user/userIdentity.h"
#include "core/data/password_manager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsDialog; }
QT_END_NAMESPACE

namespace LocalNetworkApp {

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(UserIdentity &userIdentity, PasswordManager &passwordManager, QWidget *parent = nullptr);
    ~SettingsDialog() override;

private slots:
    // 保存设置
    void on_btnSave_clicked();

    // 密码管理
    void on_btnAddPassword_clicked();
    void on_btnRemovePassword_clicked();
    void on_btnVerifyPassword_clicked();

    // 路径选择
    void on_btnSelectDownloadPath_clicked();

private:
    Ui::SettingsDialog *ui;
    UserIdentity &userIdentity;       // 用户身份（引用）
    PasswordManager &passwordManager; // 密码管理器（引用）

    // 初始化界面
    void initUI();

    // 验证表单
    bool validateForm();
};

} // namespace LocalNetworkApp

#endif // SETTINGS_DIALOG_H
