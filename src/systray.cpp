#include <QAbstractEventDispatcher>
#include <QWidget>
#include <QByteArray>
#include <QDebug>
#include <cstring>

#include <QX11Info>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <fixx11h.h>

#include <KF5/KWindowSystem/KWindowSystem>

#include "systray.h"
#include "shell.h"
#include "model.h"

Q::Systray::Systray(const QString &name, Q::Shell *shell) : QLabel(), Model(name, shell) {
    stalonetray.start("stalonetray -t -p --window-layer top --sticky");
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, NET::Properties, NET::Properties2)),
            this, SLOT(windowChanged(WId, NET::Properties, NET::Properties2)));
}

void Q::Systray::windowAdded(WId wid) {
    if(this->wid) return;
    NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMIcon|NET::WMState, NET::WM2WindowClass);
    if(strcmp(info.windowClassClass(), "stalonetray") == 0) {
        KWindowInfo info(wid, NET::WMGeometry);
        XReparentWindow(QX11Info::display(), wid, winId(), 0, 0);
        const QRect &rect = info.geometry();
        setMinimumSize(rect.width(), rect.height());
        this->wid = wid;
    }
}

void Q::Systray::windowRemoved(WId wid) {
    if(this->wid == wid) this->wid = 0;
}

void Q::Systray::windowChanged(WId wid, NET::Properties properties, NET::Properties2 properties2) {
    if(this->wid != wid) return;
    if(properties.testFlag(NET::WMGeometry)) {
        KWindowInfo info(wid, NET::WMGeometry);
        QRect rect = info.geometry();
        if(rect.width() != width() && rect.height() != height()) {
            setMinimumSize(rect.width(), rect.height());
        }
    }
}
