#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QList>
#include <algorithm>

#include <KF5/KWindowSystem/KWindowSystem>

#include "notificationsdialog.h"
#include "frame.h"
#include "panel.h"
#include "shell.h"
#include "model.h"

static QList<Q::Frame*> notificationFrames;

Q::NotificationsDialog::NotificationsDialog(QPushButton *button)
    : QWidget(), myButton(button) {
    Model *m = dynamic_cast<Model*>(button);
    if(m)
        frame = new Q::Frame(m->shell());
    else
        frame = new Q::Frame();
    frame->setCentralWidget(this);
    if(!m->shell()->wmManageDialogs()) {
        frame->setWindowFlags(frame->windowFlags() | Qt::X11BypassWindowManagerHint);
    }
    move(0, 0);
    notificationFrames.append(frame);

    connect(button, SIGNAL(clicked()), this, SLOT(toggle()));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(hideFrame(WId)));
}

void Q::NotificationsDialog::hideFrame(WId wid) {
    if(frame->winId() != wid)
        frame->hide();
}

void Q::NotificationsDialog::updateDialog() {
    resize(frame->width(), frame->height());
    const QRect geo = QGuiApplication::primaryScreen()->geometry();
    Model *m = dynamic_cast<Model*>(myButton);
    if(!m) return;
    Shell *shell = m->shell();
    const int xoff = myButton->parentWidget()->parentWidget()->x();
    const int yoff = myButton->parentWidget()->parentWidget()->y();
    frame->move(
        std::min(geo.width() - frame->width() + shell->getStrutRight(),
                 std::max(shell->getStrutLeft() + xoff, shell->getStrutLeft() + xoff + myButton->x() + myButton->width()/2 - width()/2)),
        std::min(geo.height() - frame->height() - shell->getStrutBottom() + shell->getStrutTop(),
                 std::max(shell->getStrutTop() + yoff, shell->getStrutTop() + yoff + myButton->y()))
    );

    Display *display = QX11Info::display();
    Atom atom = XInternAtom(display, "_KDE_SLIDE", false);

    QVarLengthArray<long, 1024> data(4);

    data[0] = 0;
    data[1] = 1;
    data[2] = 200;
    data[3] = 200;

    XChangeProperty(display, frame->winId(), atom, atom, 32, PropModeReplace,
            reinterpret_cast<unsigned char *>(data.data()), data.size());
}


void Q::NotificationsDialog::toggle() {
    if(!frame->isVisible()) {
        Q::NotificationsDialog::hideAll();
        frame->show();
        frame->update();
        KWindowSystem::setState(frame->winId(), NET::SkipTaskbar);
    } else {
        frame->hide();
    }
}

void Q::NotificationsDialog::hideAll() {
    foreach(Frame *frame, notificationFrames)
        frame->hide();
}
