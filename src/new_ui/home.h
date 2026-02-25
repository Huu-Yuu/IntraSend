#ifndef HOME_H
#define HOME_H

#include <QSystemTrayIcon>
#include <QWidget>

namespace Ui {
class Home;
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

protected:
    bool eventFilter(QObject *obj, QEvent *evt) override;

private:
    Ui::Home *ui;
    //窗口管理动作
    QAction *returnNormal;
    QAction *quitAction;
    //最小化到托盘
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    void InitUi();          //Ui界面初始化函数
    void InitMember();      //成员变量初始化函数
    void LittleShow();      //最小化显示函数
};

#endif // HOME_H
