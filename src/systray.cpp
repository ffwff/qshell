#include <QAbstractEventDispatcher>
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

Q::SystrayThread::SystrayThread(WId wid) : QThread(), wid(wid) {

}

void Q::SystrayThread::run() {
    // HACK: there is something wrong with this
    // why are you using a qthread to observe xlib events?
    // XSelectInput+nativeEventFilter would've done the job, had it even worked!
    Display *d = XOpenDisplay(nullptr);
    XSelectInput(d, wid, SubstructureNotifyMask);
    XEvent ev;
    while(true) {
        XNextEvent(d, &ev);
        if(ev.type == DestroyNotify) {
            emit windowRemoved();
            return;
        } else {
            KWindowInfo info(wid, NET::WMGeometry);
            const QRect &rect = info.geometry();
            emit resize(rect);
        }
    }
}

// systray
Q::Systray::Systray(const QString &name, Q::Shell *shell)
: QWidget(), Model(name, shell) {
    auto layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
}

Q::Systray::~Systray() {
    stalonetray.kill();
}

void Q::Systray::load(KConfigGroup *grp) {
    const QString args = grp->readEntry("StalonetrayArguments", "-t --window-layer top --dockapp-mode simple");
    stalonetray.start("stalonetray " + args);
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)));
}

void Q::Systray::windowAdded(WId wid) {
    if(myWid) return;
    NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMIcon|NET::WMState, NET::WM2WindowClass);
    if(strcmp(info.windowClassClass(), "stalonetray") == 0) {
        KWindowInfo info(wid, NET::WMGeometry);
        systrayResized(info.geometry());
        myWidget = QWidget::createWindowContainer(QWindow::fromWinId(wid), this);
        static_cast<QHBoxLayout*>(layout())->addWidget(myWidget);
        myWid = wid;
        thread = new SystrayThread(wid);
        connect(thread, &SystrayThread::finished, thread, &QObject::deleteLater);
        connect(thread, &SystrayThread::resize, this, &Q::Systray::systrayResized);
        connect(thread, &SystrayThread::windowRemoved, this, &Q::Systray::windowRemoved);
        thread->start();
    }
}

void Q::Systray::systrayResized(const QRect &rect) {
    setFixedSize(rect.width(), rect.height());
}

void Q::Systray::windowRemoved() {
    myWid = 0;
    myWidget->deleteLater();
    myWidget = nullptr;
    thread->deleteLater();
    thread = nullptr;
}
