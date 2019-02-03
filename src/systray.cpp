#include <QAbstractEventDispatcher>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QWidget>
#include <QWindow>
#include <QDebug>
#include <cstring>

#include <QX11Info>
#include <X11/Xlib.h>
#include <fixx11h.h>

#include <KF5/KWindowSystem/KWindowSystem>

#include "systray.h"
#include "shell.h"
#include "model.h"

struct IconPixmap {
    int width;
    int height;
    QByteArray data;
};
Q_DECLARE_METATYPE(IconPixmap)

QDBusArgument &operator<<(QDBusArgument &argument, const IconPixmap &){
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IconPixmap &icon){
    qint32 width;
    qint32 height;
    QByteArray data;

    argument.beginStructure();
    argument >> width;
    argument >> height;
    argument >> data;
    argument.endStructure();

    icon.width = width;
    icon.height = height;
    icon.data = data;

    return argument;
}

// systray item
Q::SystrayItem::SystrayItem(const QString &name, Q::Systray *systray)
    : QPushButton(systray), systray(systray) {
    const QStringList list = name.split("/");
    const QString service = list[0];
    const QString path = QString("/") + list[1];
    interface = new QDBusInterface(service, path, "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus(), this);
    itemInterface = new QDBusInterface(service, path, "org.kde.StatusNotifierItem", QDBusConnection::sessionBus(), this);
    connect(itemInterface, SIGNAL(NewIcon()), this, SLOT(update()));
    update();
}

void Q::SystrayItem::update() {
    QDBusMessage response = interface->call("Get", "org.kde.StatusNotifierItem", "IconPixmap");
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> args = response.arguments();
        if (args.length() == 1) {
            QVariant arg = args.at(0);
            if (!arg.isNull() && arg.isValid()) {
                QVariant v = qvariant_cast<QDBusVariant>(arg).variant();
                const QDBusArgument &arg = qvariant_cast<QDBusArgument>(v);
                QVector<IconPixmap> ipv; arg >> ipv;
                if(ipv.size() == 1) {
                    IconPixmap ip = ipv.at(0);
                    QImage image((const uchar*)ip.data.constData(), ip.width, ip.height, QImage::Format_ARGB32);
                    setIcon(QIcon(QPixmap::fromImage(image)));
                    setIconSize(QSize(systray->iconSize(), systray->iconSize()));
                    setMinimumSize(QSize(ip.width, ip.height));
                } else {
                    hide();
                }
            }
        }
    }
}

void Q::SystrayItem::mouseReleaseEvent(QMouseEvent *) {
    itemInterface->call("ContextMenu", 0, 10);
}

// systray
Q::Systray::Systray(const QString &name, Q::Shell *shell)
    : QWidget(), Model(name, shell) {
    qDBusRegisterMetaType<IconPixmap>();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    statusNotifierGetter = new QDBusInterface("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus(), this);
    statusNotifierWatcher = new QDBusInterface("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher", QDBusConnection::sessionBus(), this);
    connect(statusNotifierWatcher, SIGNAL(StatusNotifierItemRegistered(QString)),
            this, SLOT(itemRegistered(QString)));
    connect(statusNotifierWatcher, SIGNAL(StatusNotifierItemUnregistered(QString)),
            this, SLOT(itemUnregistered(QString)));
}

void Q::Systray::load(KConfigGroup *grp) {
    myIconSize = grp->readEntry("Size", 24);
    update();
}

// events
void Q::Systray::itemRegistered(QString str) {
    SystrayItem *item = new SystrayItem(str, this);
    static_cast<QBoxLayout*>(layout())->addWidget(item);
    items[str] = item;
}

void Q::Systray::itemUnregistered(QString str) {
    items[str]->deleteLater();
    items.remove(str);
}

void Q::Systray::update() {
    QDBusMessage response = statusNotifierGetter->call("Get", "org.kde.StatusNotifierWatcher", "RegisteredStatusNotifierItems");
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> args = response.arguments();
        if (args.length() == 1) {
            QVariant arg = args.at(0);
            if (!arg.isNull() && arg.isValid()) {
                QVariant v = qvariant_cast<QDBusVariant>(arg).variant();
                QStringList list = v.toStringList();
                foreach(const QString str, list) {
                    itemRegistered(str);
                }
            }
        }
    }
}
