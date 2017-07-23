#include <QWidget>

#include <X11/Xatom.h>
#include <QX11Info>
#include <X11/Xlib.h>
#include <fixx11h.h>

#include "shell.h"
#include "frame.h"

Q::Frame::Frame(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setAttribute(Qt::WA_TranslucentBackground);
    
    Atom atom;
    atom = XInternAtom(QX11Info::display(), "_KDE_NET_WM_BLUR_BEHIND_REGION", False);
    XChangeProperty(QX11Info::display(), winId(), atom, XA_CARDINAL,
                    32, PropModeReplace, 0, 0);
    repaint();
};

void Q::Frame::setCentralWidget(QWidget *w)
{
    widget = w;
    widget->setParent(this);
    widget->move(0, 0);
};

void Q::Frame::resizeEvent(QResizeEvent *) {
    if(widget)
        widget->resize(size());
};
