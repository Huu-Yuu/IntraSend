#include <QApplication>
#include <QFile>
#include <QFont>
#include "new_ui/home.h"
// #include "new_ui/appinit.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //加载样式表
    // QFile file(":/pic/css/index.css");
    // if (file.open(QFile::ReadOnly)) {
    //     QString qss = QLatin1String(file.readAll());
    //     qApp->setStyleSheet(qss);
    //     file.close();
    // }

    //全局字体
    QFont font("Arial", 10);
    a.setFont(font);

    //屏幕拖动
    // AppInit::Instance()->start();

    Home w;
    w.show();

    return a.exec();
}
