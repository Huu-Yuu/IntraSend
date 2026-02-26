#include "appinit.h"
#include "qmutex.h"
#include "qapplication.h"
#include "qevent.h"
#include "qwidget.h"

AppInit *AppInit::self = 0;
AppInit *AppInit::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new AppInit;
        }
    }

    return self;
}

AppInit::AppInit(QObject *parent) : QObject(parent)
{
}

bool AppInit::eventFilter(QObject *obj, QEvent *evt)
{
    QWidget *w = (QWidget *)obj;
    if (!w->property("canMove").toBool()) {
        return QObject::eventFilter(obj, evt);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;

    QMouseEvent *event = static_cast<QMouseEvent *>(evt);
    if (event->type() == QEvent::MouseButtonPress) {
        qDebug() << "按压";
        if (event->button() == Qt::LeftButton) {
            qDebug() << "鼠标左键";
            mousePressed = true;
            mousePoint =  event->globalPosition().toPoint() - w->pos();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        qDebug() << "鼠标释放";
        mousePressed = false;
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        qDebug() << "鼠标移动";
        if (mousePressed && (event->buttons() & Qt::LeftButton)) {
            w->move( event->globalPosition().toPoint() - mousePoint);
            return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

void AppInit::start()
{
    qApp->installEventFilter(this);
}
