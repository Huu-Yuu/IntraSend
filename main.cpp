#include "mainwindow.h"

#include <QApplication>
#include "QtWidgets/QApplication"
int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
