#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QSettings>
#include "core/utils/constants.h"
#include "core/data/security_manager.h"

namespace LocalNetworkApp {

SettingsDialog::SettingsDialog(UserIdentity &userIdentity, PasswordManager &passwordManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , userIdentity(userIdentity)
    , passwordManager(passwordManager)
{
    ui->setupUi(this);
    initUI();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_btnSave_clicked()
{
    if (!validateForm()) {
        return;
    }

    // 保存基本设置
    userIdentity.setNickname(ui->txtNickname->text().trimmed());

    // 保存下载路径
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    settings.setValue("download/path", ui->txtDownloadPath->text().trimmed());

    QMessageBox::information(this, tr("保存成功"), tr("设置已保存！"));
    accept();
}

void SettingsDialog::on_btnAddPassword_clicked()
{
    QString password1 = ui->txtNewPassword->text();
    QString password2 = ui->txtConfirmPassword->text();

    if (password1 != password2) {
        QMessageBox::warning(this, tr("错误"), tr("两次输入的密码不一致！"));
        return;
    }

    if (!SecurityManager::isStrongPassword(password1)) {
        QMessageBox::warning(this, tr("密码强度不足"),
            tr("密码必须满足以下条件：\n"
               "1. 长度至少8位\n"
               "2. 包含数字\n"
               "3. 包含小写字母\n"
               "4. 包含大写字母\n"
               "5. 包含特殊字符"));
        return;
    }

    if (passwordManager.addPassword(password1)) {
        QMessageBox::information(this, tr("成功"), tr("密码已添加！"));
        ui->txtNewPassword->clear();
        ui->txtConfirmPassword->clear();
    } else {
        QMessageBox::warning(this, tr("失败"), tr("添加密码失败！"));
    }
}

void SettingsDialog::on_btnRemovePassword_clicked()
{
    QString password = ui->txtCurrentPassword->text();

    if (passwordManager.verifyPassword(password)) {
        if (passwordManager.removePassword(password)) {
            QMessageBox::information(this, tr("成功"), tr("密码已移除！"));
            ui->txtCurrentPassword->clear();
        } else {
            QMessageBox::warning(this, tr("失败"), tr("移除密码失败！"));
        }
    } else {
        QMessageBox::warning(this, tr("错误"), tr("当前密码不正确！"));
    }
}

void SettingsDialog::on_btnVerifyPassword_clicked()
{
    QString password = ui->txtCurrentPassword->text();

    if (passwordManager.verifyPassword(password)) {
        QMessageBox::information(this, tr("验证成功"), tr("密码正确！"));
    } else {
        QMessageBox::warning(this, tr("验证失败"), tr("密码不正确！"));
    }
}

void SettingsDialog::on_btnSelectDownloadPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("选择下载目录"),
        ui->txtDownloadPath->text());

    if (!path.isEmpty()) {
        ui->txtDownloadPath->setText(path);
    }
}

void SettingsDialog::initUI()
{
    setWindowTitle(tr("程序设置"));
    setModal(true);

    // 加载当前设置
    ui->txtNickname->setText(userIdentity.getNickname());
    ui->lblUserId->setText(tr("用户ID：%1").arg(userIdentity.getUuid().toString()));

    // 加载下载路径
    QSettings settings(Constants::SETTINGS_FILE, QSettings::IniFormat);
    QString downloadPath = settings.value("download/path", Constants::DEFAULT_DOWNLOAD_PATH).toString();
    ui->txtDownloadPath->setText(downloadPath);

    // 设置密码强度检查
    connect(ui->txtNewPassword, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString strength = SecurityManager::getPasswordStrength(text);
        ui->lblPasswordStrength->setText(tr("密码强度：%1").arg(strength));

        if (strength == "强") {
            ui->lblPasswordStrength->setStyleSheet("color: green;");
        } else if (strength == "中") {
            ui->lblPasswordStrength->setStyleSheet("color: orange;");
        } else {
            ui->lblPasswordStrength->setStyleSheet("color: red;");
        }
    });
}

bool SettingsDialog::validateForm()
{
    // 验证昵称
    QString nickname = ui->txtNickname->text().trimmed();
    if (nickname.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("昵称不能为空！"));
        ui->txtNickname->setFocus();
        return false;
    }

    // 验证下载路径
    QString downloadPath = ui->txtDownloadPath->text().trimmed();
    if (downloadPath.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("下载路径不能为空！"));
        ui->txtDownloadPath->setFocus();
        return false;
    }

    // 检查路径是否存在，不存在则创建
    QDir dir(downloadPath);
    if (!dir.exists()) {
        if (QMessageBox::question(this, tr("路径不存在"),
            tr("指定的下载路径不存在，是否创建？"),
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            if (!dir.mkpath(".")) {
                QMessageBox::warning(this, tr("错误"), tr("无法创建下载路径！"));
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

} // namespace LocalNetworkApp
