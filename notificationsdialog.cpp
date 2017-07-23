#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <algorithm>

#include <KF5/KWindowSystem/KWindowSystem>

#include "notificationsdialog.h"
#include "frame.h"
#include "panel.h"
#include "shell.h"
#include "model.h"

Q::NotificationsDialog::NotificationsDialog(QWidget *button) : QWidget(), myButton(button)
{
    Model *m = dynamic_cast<Model*>(button);
    if(m)
        frame = new Q::Frame(m->shell());
    else
        frame = new Q::Frame();
    frame->setCentralWidget(this);
    move(0, 0);
    
    connect(button, SIGNAL(clicked()), this, SLOT(toggle()));
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, [this](WId wid) {
        if(frame->isVisible() && frame->winId() != wid)
            frame->hide();
    });
};

void Q::NotificationsDialog::updateDialog()
{
    resize(frame->width(), frame->height());
    QRect geo = QGuiApplication::primaryScreen()->geometry();
    Model *m = dynamic_cast<Model*>(myButton);
    if(!m) return;
    Shell *shell = m->shell();
    frame->move(
        std::min(geo.width() - frame->width(), shell->getStrutLeft() + myButton->x() + frame->width() / 2),
        std::min(geo.height() - frame->height(), shell->getStrutTop() + myButton->y())
    );
};


void Q::NotificationsDialog::toggle() {
    frame->setVisible(!frame->isVisible());
    KWindowSystem::setState(frame->winId(), NET::SkipTaskbar);
};
