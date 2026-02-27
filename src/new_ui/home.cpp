#include "home.h"
#include "ui_home.h"
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QMouseEvent>

Home::Home(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Home)
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
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));           //绑定槽函数退出
    connect(returnNormal, SIGNAL(triggered()), this, SLOT(showNormal()));   //绑定槽函数还原界面

    //创建托盘菜单(必须先创建动作，后添加菜单项，还可以加入菜单项图标美化)
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(returnNormal);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
}

void Home::LittleShow()
{
    this->hide();//隐藏主窗口
    trayIcon->show();//显示托盘

    //显示到系统提示框的信息
    QString title="内网传书";
    QString text="自动在后台运行";
    trayIcon->showMessage(title,text,QSystemTrayIcon::Information,3000); //此参数为提示时长
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

